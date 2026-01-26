# Threads

Threads are the fundamental execution abstraction in F9.
Each thread has an associated context, address space, and scheduling state.

## Thread Control Block

The Thread Control Block (TCB) contains all kernel state for a thread:

```c
struct tcb {
    /* Hot scheduler fields - Cache Line 0 */
    struct {
        struct tcb *prev, *next;
    } sched_link;                    /* Doubly-linked ready queue */

    thread_state_t state;            /* Current execution state */
    uint8_t priority;                /* Effective priority (0-31) */
    uint8_t base_priority;           /* Natural priority level */
    uint8_t preempt_threshold;       /* Preemption threshold (PTS) */
    uint8_t user_priority;           /* Original user priority */

    l4_thread_t t_globalid;          /* Unique system-wide identifier */
    l4_thread_t t_localid;           /* Address space-local identifier */

    memptr_t stack_base;             /* Stack region base address */
    size_t stack_size;               /* Stack region size */
    /* End of Cache Line 0 (32 bytes) */

    context_t ctx;                   /* Saved CPU context */

    /* PTS (Preemption-Threshold Scheduling) fields */
    uint8_t user_preempt_threshold;  /* Original user-set threshold */
    uint8_t inherit_priority;        /* Priority Inheritance Protocol */
    uint8_t _pts_pad[2];             /* Alignment padding */

    as_t *as;                        /* Associated address space */
    struct utcb *utcb;               /* User Thread Control Block */

    l4_thread_t ipc_from;            /* Expected IPC sender */

    struct tcb *t_sibling;           /* Next sibling thread */
    struct tcb *t_parent;            /* Parent thread */
    struct tcb *t_child;             /* First child thread */

    uint32_t timeout_event;          /* Pending timeout event ID */
};
```

The TCB layout places scheduler-hot fields at the beginning for cache efficiency.
The `sched_link` enables O(1) queue operations for the priority bitmap scheduler.
The PTS (Preemption-Threshold Scheduling) fields support ThreadX-compatible threshold-based preemption control.
See [scheduler.md](scheduler.md) for complete PTS documentation.

### Stack Overflow Protection

F9 uses a canary value at the stack base for overflow detection:

```c
#define STACK_CANARY  0xDEADBEEF

static inline void thread_init_canary(tcb_t *thr)
{
    if (thr->stack_base)
        *((uint32_t *) thr->stack_base) = STACK_CANARY;
}

static inline int thread_check_canary(tcb_t *thr)
{
    if (!thr->stack_base)
        return 1;  /* No stack tracking */
    return *((uint32_t *) thr->stack_base) == STACK_CANARY;
}
```

The canary is checked on every context switch. Corruption indicates stack overflow.

### Thread Identifiers

F9 uses 18-bit thread IDs encoded in 32-bit global identifiers:
- Bits 14-31: Thread number (18 bits)
- Bits 0-13: Reserved (version field, currently unused)

Conversion macros:

```c
#define GLOBALID_TO_TID(id)  (id >> 14)   /* Extract thread number */
#define TID_TO_GLOBALID(id)  (id << 14)   /* Create global ID */
```

Reserved thread numbers:

| ID | Thread |
|----|--------|
| 0 | Idle thread |
| 1 | Kernel thread |
| 2 | Root thread |
| 3+ | IRQ threads and user threads |

### Thread States

```c
typedef enum {
    T_INACTIVE,      /* Created but not started */
    T_RUNNABLE,      /* Ready to execute */
    T_SVC_BLOCKED,   /* Blocked in system call */
    T_SEND_BLOCKED,  /* Blocked sending IPC */
    T_RECV_BLOCKED,  /* Blocked receiving IPC */
} thread_state_t;
```

## CPU Context

The context structure holds saved register state for ARM Cortex-M:

```c
typedef struct {
    uint32_t sp;         /* Stack pointer (PSP) */
    uint32_t ret;        /* Return address (EXC_RETURN or PC) */
    uint32_t ctl;        /* CONTROL register */
    uint32_t regs[8];    /* R0-R7 (also used as MR0-MR7 for IPC) */
#ifdef CONFIG_FPU
    uint64_t fp_regs[8]; /* D0-D7 floating point registers */
    uint32_t fp_flag;    /* FPU status flags */
#endif
} context_t;
```

The `regs[8]` array serves dual purpose: general registers during execution and message registers (MR0-MR7) during IPC operations.
This avoids copying between register sets.

On exception entry, the Cortex-M hardware automatically saves registers to the stack.
The exception frame layout:

```
+--------+ <- Original PSP
|  xPSR  |
|   PC   |
|   LR   |
|  R12   |
|   R3   |
|   R2   |
|   R1   |
|   R0   |
+--------+ <- PSP after exception entry
```

Additional registers (R4-R11) are saved by software when needed for context switching.

## User Thread Control Block

The UTCB provides a shared region between kernel and user space:
- Message registers MR8-MR15 (MR0-MR7 are in `ctx.regs[]`, see [ipc.md](ipc.md))
- Thread local storage
- Sender identification (`utcb->sender`)
- Intended receiver for blocked sends (`utcb->intended_receiver`)
- Pager thread reference (`utcb->t_pager`)

The UTCB is always accessible to its owning thread, mapped into the address space with appropriate permissions.
See [memory.md](memory.md) for address space configuration.

## System Threads

F9 defines three system threads that exist throughout system lifetime:

### Kernel Thread

The kernel thread runs in privileged (Handler) mode and handles software interrupts including:
- System call processing
- Kernel timer events (see [ktimer.md](ktimer.md))
- Deferred interrupt handling

The kernel thread has the highest scheduling priority, ensuring prompt handling of kernel operations.

### Root Thread

The root thread is the entry point for user-space code, responsible for:
- Creating and configuring user threads
- Mapping memory regions to child threads (see [memory.md](memory.md))
- Starting threads via IPC (see [ipc.md](ipc.md))

At startup, the root thread receives all user-space memory pools in its address space,
allowing it to distribute memory to children.

### Idle Thread

The idle thread runs when no other thread is runnable. It executes a `wfi` (wait for interrupt) instruction in a loop,
allowing the CPU to enter low-power state until the next interrupt.

```c
void idle_thread(void)
{
    while (1) {
        __WFI();
    }
}
```

## Thread Creation

Threads are created with the ThreadControl system call:

```c
L4_ThreadControl(new_tid, space_tid, scheduler_tid, pager_tid, utcb_location);
```

Parameters:
- `new_tid`: ID for the new thread
- `space_tid`: Thread whose address space to share (or same ID for new space)
- `scheduler_tid`: Scheduling authority (not fully implemented)
- `pager_tid`: Pager thread (not fully implemented)
- `utcb_location`: Memory location for the UTCB

The new thread starts in `T_INACTIVE` state.
Additional setup is typically required before starting.

## Thread Startup

After creation, the root thread typically:
1. Maps required memory regions (code, data, stack)
2. Sends a startup IPC message containing:
   - Entry point address
   - Initial stack pointer
   - Stack size

The kernel detects this special IPC and initializes the thread's context accordingly.

Example:

```c
/* Create thread */
L4_ThreadControl(new_id, new_id, L4_nilthread, myself, utcb_addr);

/* Map memory regions */
L4_map(text_base, text_size, new_id);
L4_map(stack_base, stack_size, new_id);

/* Start thread */
L4_Word_t startup[5] = {
    (L4_Word_t)entry_function,
    (L4_Word_t)stack_top,
    stack_size,
    0, 0
};
L4_MsgPut(&msg, 0, 5, startup, 0, NULL);
L4_MsgLoad(&msg);
L4_Ipc(new_id, myself, 0, NULL);
```

## Thread Hierarchy

Threads form a tree structure rooted at the root thread:
- Each thread has one parent (except root)
- Each thread can have multiple children
- Children are linked as siblings

This hierarchy is used by the scheduler to traverse runnable threads and by the kernel to manage thread lifetimes.

## Scheduling

F9 uses a fixed-priority preemptive scheduler with Preemption-Threshold Scheduling (PTS) extensions.
The scheduler provides:
- O(1) thread selection using ARM Cortex-M CLZ instruction
- 32 priority levels (0 = highest, 31 = lowest)
- Round-robin within priority levels
- Preemption thresholds for reduced context switching
- Priority inheritance for IPC and resource sharing

The TCB priority fields support both basic priority scheduling and PTS:

```c
struct tcb {
    /* ... other fields ... */
    uint8_t priority;              /* Effective priority (may be boosted) */
    uint8_t base_priority;         /* Natural priority level */
    uint8_t preempt_threshold;     /* Preemption threshold (PTS) */
    uint8_t user_priority;         /* Original user-set priority */
    uint8_t user_preempt_threshold; /* Original user threshold */
    uint8_t inherit_priority;      /* Priority inherited via PIP */
};
```

For complete scheduler documentation including PTS semantics, priority inheritance protocol, API reference, and performance characteristics, see [scheduler.md](scheduler.md).

## Context Switching

When the scheduler selects a new thread:

1. Save current thread's context:
   - Push R4-R11 to current stack
   - Store PSP (Process Stack Pointer) in TCB
   - Store return address in TCB

2. Configure memory protection for new thread:
   - Call `as_setup_mpu()` to configure MPU regions (see [memory.md](memory.md))
   - Prioritize stack and code regions

3. Restore new thread's context:
   - Load PSP from new thread's TCB
   - Pop R4-R11 from new stack
   - Execute exception return (EXC_RETURN) to resume

### Exception Return

ARM Cortex-M uses special EXC_RETURN values to control exception exit behavior:

| Value | Return Stack | Return Mode |
|-------|--------------|-------------|
| 0xFFFFFFF1 | MSP | Handler |
| 0xFFFFFFF9 | MSP | Thread |
| 0xFFFFFFFD | PSP | Thread |

F9 uses 0xFFFFFFFD for user threads (return to Thread mode using PSP).

## PendSV for Context Switch

F9 uses the PendSV exception for context switching:

- PendSV has the lowest priority, ensuring it runs after all other interrupts
- Triggered by setting PENDSVSET in ICSR (Interrupt Control and State Register)
- Allows deferred context switch from interrupt handlers

```c
void trigger_context_switch(void)
{
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}
```

## Thread Termination

Threads can be deleted via ThreadControl with a null space parameter:

```c
L4_ThreadControl(tid, L4_nilthread, L4_nilthread, L4_nilthread, (void *)-1);
```

The kernel:

1. Removes the thread from scheduling
2. Unlinks it from the thread tree
3. Releases TCB resources
4. Does not automatically clean up child threads

## Related Documentation

- [memory.md](memory.md) - Address space management and MPU configuration
- [ipc.md](ipc.md) - Thread communication and startup
- [ktimer.md](ktimer.md) - Timeslice management
- [interrupt.md](interrupt.md) - Interrupt handler scheduling
