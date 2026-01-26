# Memory Management

F9 implements memory protection without virtual memory, targeting embedded systems that lack an MMU. This document describes the memory management architecture, including address spaces, flex pages, the Memory Protection Unit (MPU), and memory mapping operations.

## Design Philosophy

Traditional L4 microkernels use flex pages to represent mappings between virtual and physical addresses, implemented through hardware page tables. F9 operates on systems without an MMU, so flex pages instead represent protected regions of physical memory.

This departure from traditional L4 semantics still preserves the key abstractions:
- Threads have isolated address spaces
- Memory regions can be mapped, granted, or unmapped between threads
- The kernel enforces access permissions through the MPU

## ARM MPU Overview

ARMv7-M processors support the Protected Memory System Architecture (PMSAv7) model with a Memory Protection Unit. The STM32F4 series (Cortex-M4) provides eight separate memory regions.

### MPU Features

The ARM MPU provides:
- Configurable protection regions (8 on Cortex-M3/M4)
- Overlapping region support with priority (higher region number wins)
- Read/write/execute access permissions
- Memory attribute control (cacheability, shareability, bufferable)
- Subregion disable for finer granularity

Permission violations trigger a MemManage fault with programmable priority.

### Region Constraints

The ARM MPU imposes constraints on memory regions:
- Minimum region size: 32 bytes
- Maximum region size: 4 GB
- Region size must be a power of two
- Base address must be aligned to the region size

These constraints require software to split non-aligned memory regions into multiple smaller regions.
For example, a 48-byte region starting at address 0x1000 cannot be represented as a single MPU region.
F9 splits it into:
- 32-byte region at 0x1000
- 16-byte region at 0x1020

This splitting is handled automatically by the flex page creation functions, but increases kernel overhead for non-aligned regions.

## MPU Register Interface

| Register | Address | Purpose |
|----------|---------|---------|
| MPU_TYPE | 0xE000ED90 | Query MPU presence and region count |
| MPU_CTRL | 0xE000ED94 | Global MPU enable/disable |
| MPU_RNR | 0xE000ED98 | Select active region number |
| MPU_RBAR | 0xE000ED9C | Region base address |
| MPU_RASR | 0xE000EDA0 | Region attributes and size |

### MPU Type Register (MPU_TYPE)

Read-only register indicating MPU capabilities:
- Bits 15-8: DREGION - Number of data regions (8 for Cortex-M3/M4)
- Bit 0: SEPARATE - Unified (0) or separate (1) instruction/data regions

### MPU Control Register (MPU_CTRL)

- Bit 2: PRIVDEFENA - Enable default memory map for privileged access
- Bit 1: HFNMIENA - Enable MPU during HardFault and NMI handlers
- Bit 0: ENABLE - Global MPU enable

### Region Base Address Register (MPU_RBAR)

- Bits 31-N: ADDR - Region base address (N depends on region size)
- Bit 4: VALID - Use REGION field to select region
- Bits 3-0: REGION - Region number when VALID=1

### Region Attribute and Size Register (MPU_RASR)

```
Bits 31-29: Reserved
Bit 28:     XN (eXecute Never)
Bit 27:     Reserved
Bits 26-24: AP (Access Permission)
Bits 23-22: Reserved
Bits 21-19: TEX (Type Extension)
Bits 18-16: S, C, B (Shareability, Cacheability, Bufferable)
Bits 15-8:  SRD (Subregion Disable)
Bits 7-6:   Reserved
Bits 5-1:   SIZE (Region size encoding)
Bit 0:      ENABLE
```

### Size Encoding

| SIZE Value | Region Size |
|------------|-------------|
| 0b00100 | 32 bytes |
| 0b00101 | 64 bytes |
| 0b00110 | 128 bytes |
| 0b00111 | 256 bytes |
| ... | ... |
| 0b11111 | 4 GB |

Formula: Region size = 2^(SIZE+1) bytes

### Access Permissions (AP)

| AP | Privileged | Unprivileged |
|----|------------|--------------|
| 000 | No access | No access |
| 001 | RW | No access |
| 010 | RW | RO |
| 011 | RW | RW |
| 101 | RO | No access |
| 110 | RO | RO |

## Address Spaces

An address space defines the memory accessible to one or more threads. Each address space contains a list of flex pages representing the permitted memory regions.

```c
typedef struct {
    uint32_t as_spaceid;
    struct fpage *first;           /* All flex pages in this address space */
    struct fpage *mpu_first;       /* Flex pages scheduled for MPU (non-stack) */
    struct fpage *mpu_stack_first; /* Stack flex pages (always mapped) */
    uint32_t shared;               /* Reference count for shared spaces */
} as_t;
```

Multiple threads can share an address space, useful for implementing shared memory regions or thread groups. See [threads.md](threads.md) for thread-address space relationships.

## Flex Pages

A flex page represents a contiguous region of protected memory:

```c
struct fpage {
    struct fpage *as_next;   /* Next flex page in address space */
    struct fpage *map_next;  /* Next in mapping chain */
    struct fpage *mpu_next;  /* Next in MPU scheduling queue */

    union {
        struct {
            uint32_t base;       /* Region base address */
            uint32_t mpid : 6;   /* Memory pool identifier */
            uint32_t flags : 6;  /* Control flags (FPAGE_ALWAYS, etc.) */
            uint32_t shift : 16; /* Size = 1 << shift */
            uint32_t rwx : 4;    /* Read/write/execute permissions */
        } fpage;
        uint32_t raw[2];         /* Raw access for efficient copying */
    };

#ifdef CONFIG_KDB
    int used;                    /* Allocation tracking (debug only) */
#endif
};
```

The `shift` field encodes region size as a power of two. Access macros provide the actual values:

```c
#define FPAGE_BASE(fp)  (fp)->fpage.base
#define FPAGE_SIZE(fp)  (1 << (fp)->fpage.shift)
#define FPAGE_END(fp)   (FPAGE_BASE(fp) + FPAGE_SIZE(fp))
```

Flex page flags control MPU mapping behavior:

| Flag | Value | Description |
|------|-------|-------------|
| FPAGE_ALWAYS | 0x1 | Always mapped in MPU |
| FPAGE_CLONE | 0x2 | Mapped from another address space |
| FPAGE_MAPPED | 0x4 | Mapped via MAP operation |

## Memory Pools

Memory pools define the static memory layout and associate regions with permissions and metadata:

```c
#define DECLARE_MEMPOOL(name, start, end, flags, tag) \
{                                                     \
    .name = name,                                     \
    .start = (memptr_t)(start),                       \
    .end = (memptr_t)(end),                           \
    .flags = flags,                                   \
    .tag = tag                                        \
}
```

### Standard Memory Pools

| Pool | Permissions | Purpose |
|------|-------------|---------|
| KTEXT | KR, KX | Kernel code |
| KDATA | KR, KW | Kernel data |
| UTEXT | UR, UX | User code |
| UDATA | UR, UW | User data |
| UBSS | UR, UW | User uninitialized data |

The `MP_MAP_ALWAYS` flag indicates regions that should always be mapped when present in an address space, such as code sections.

## Address Space Operations

L4 provides three fundamental operations on address spaces, implemented via IPC typed words (see [ipc.md](ipc.md)):

### Map

Mapping creates a shared reference from one address space to another. The source thread retains access, and the destination thread gains access to the same physical memory.

```c
int map_area(as_t *src, as_t *dst, memptr_t base, size_t size,
             map_action_t action, int is_privileged);
```

For privileged threads (like the root thread), mapping uses `assign_fpages_ext()` to directly create flex pages.
For unprivileged threads, the function:
1. Locates flex pages in the source covering the requested region
2. Splits flex pages at region boundaries if necessary
3. Creates corresponding entries in the destination address space

### Grant

Granting transfers ownership of a memory region from one address space to another. The source thread loses access, and the destination thread becomes the sole owner.

### Flush/Unmap

Unmapping removes flex pages from an address space, revoking access to the underlying memory regions.

## Flex Page Creation

The `assign_fpages_ext()` function creates flex pages for a memory region:

```c
int assign_fpages_ext(int mpid, as_t *as, memptr_t base, size_t size,
                      fpage_t **pfirst, fpage_t **plast);
```

The algorithm handles existing flex pages in the target region:
1. If `mpid` is unknown, search for an appropriate memory pool
2. Walk the address space's flex page list
3. For gaps between existing flex pages, create new ones
4. Chain the new flex pages into the address space
5. Return pointers to the first and last created flex pages

## MPU Configuration

Since the ARM MPU has limited regions (8 on Cortex-M3/M4), the kernel must decide which flex pages to map at any time.

### Prioritization Strategy

The `as_setup_mpu()` function organizes flex pages into a priority queue:
1. Stack regions: Always mapped to ensure execution can continue
2. Program counter region: The current code must be accessible
3. `FPAGE_ALWAYS` regions: Marked as required (typically code sections)
4. Other regions: Mapped in FIFO order

### Context Switch and MPU Setup

On context switch, `as_setup_mpu()` configures the MPU for the new thread (see [threads.md](threads.md)):

```c
void as_setup_mpu(as_t *as, memptr_t sp, memptr_t pc,
                  memptr_t stack_base, size_t stack_size)
{
    fpage_t *mpu[8] = { NULL };
    int i = 0;

    /* 1. Map stack flex pages first */
    for (fp = as->first; i < 8 && fp; fp = fp->as_next) {
        if (addr_in_fpage(stack_base, fp, 0)) {
            mpu[i++] = fp;
        }
    }

    /* 2. Map PC and FPAGE_ALWAYS regions */
    /* 3. Fill remaining slots from mpu_first queue */

    /* Configure hardware MPU */
    for (j = 0; j < 8; ++j) {
        mpu_setup_region(j, mpu[j]);
    }
}
```

### On-Demand Mapping

When a thread accesses memory not currently mapped in the MPU:
1. A MemManage fault exception occurs
2. The kernel's fault handler searches unmapped flex pages
3. If a flex page covers the faulting address, it replaces an existing mapping
4. If no flex page exists, the access is invalid and the kernel panics

This lazy mapping reduces context switch overhead by deferring work until actually needed.

## MemManage Fault Handling

When a thread accesses unmapped memory:
1. MemManage exception transfers control to `__memmanage_handler()`
2. Kernel reads MMFAR (MemManage Fault Address Register) to get faulting address
3. `mpu_select_lru()` searches unmapped flex pages for one covering the address
4. If found, the flex page is added to the MPU and an existing one is evicted
5. If not found, the kernel panics (invalid access)

### Fault Status Registers

- MMFSR (0xE000ED28): MemManage Fault Status Register
  - Bit 7: MMARVALID - MMFAR contains valid address
  - Bit 4: MSTKERR - Stacking error
  - Bit 3: MUNSTKERR - Unstacking error
  - Bit 1: DACCVIOL - Data access violation
  - Bit 0: IACCVIOL - Instruction access violation

- MMFAR (0xE000ED34): MemManage Fault Address Register

### LRU Selection

The `mpu_select_lru()` function implements FIFO eviction:

```c
int mpu_select_lru(as_t *as, uint32_t addr)
{
    fpage_t *fp = as->first;

    while (fp) {
        if (addr_in_fpage(addr, fp, 0)) {
            /* Remove from current position */
            remove_fpage_from_list(as, fp, mpu_first, mpu_next);

            /* Add to front of queue */
            fp->mpu_next = as->mpu_first;
            as->mpu_first = fp;

            /* Reconfigure MPU */
            /* ... */
            return 0;
        }
        fp = fp->as_next;
    }
    return 1;  /* Address not in any flex page */
}
```

## MPU Initialization

During kernel startup:

```c
void mpu_init(void)
{
    /* Disable MPU during configuration */
    MPU->CTRL = 0;

    /* Configure kernel regions (if needed) */

    /* Enable MPU with PRIVDEFENA for kernel access */
    MPU->CTRL = MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;

    __DSB();
    __ISB();
}
```

## Root Thread Initialization

At system startup, the root thread receives all user-space memory regions in its address space. This "magic" mapping allows the root thread to subsequently map regions to child threads as needed.

The root thread's responsibilities (see [threads.md](threads.md)):
1. Create child threads via the ThreadControl system call
2. Map required memory regions (code, data, stack) to each child
3. Start threads by sending initialization IPC messages

## Shared Memory

Threads can share memory regions through the mapping mechanism:
1. Thread A maps a region to Thread B
2. Both threads now have flex pages referencing the same physical memory
3. Access permissions can differ between the threads

This enables efficient inter-thread communication without copying data. For message-based communication, see [ipc.md](ipc.md).

## Kernel API

```c
/* Configure a protection region */
void mpu_setup_region(int n, fpage_t *fp);

/* Enable/disable the MPU */
void mpu_enable(mpu_state_t i);

/* Handle memory management faults */
void __memmanage_handler(void);

/* Select least recently used region for eviction */
int mpu_select_lru(as_t *as, uint32_t addr);
```

## Related Documentation

- [threads.md](threads.md) - Thread address space associations, context switching
- [ipc.md](ipc.md) - Memory mapping via IPC typed words
- [interrupt.md](interrupt.md) - MemManage fault priority

## References

- ARMv7-M Architecture Reference Manual
- Cortex-M3 Technical Reference Manual, Section 9.1
- Cortex-M4 Generic User Guide, Chapter 4.5
