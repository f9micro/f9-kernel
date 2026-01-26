# Kernel Probes (KProbes)

KProbes is an in-kernel dynamic instrumentation mechanism that allows developers to gather information about kernel operation without recompiling or rebooting. This implementation is inspired by Linux kernel KProbes but adapted for the ARM Cortex-M architecture.

## Architecture Support

KProbes is currently implemented for ARMv7-M using hardware breakpoints provided by the Flash Patch and Breakpoint Unit (FPB). The FPB is part of the ARMv7-M Debug Architecture and provides hardware comparators that can trigger debug exceptions on instruction fetch.

## How KProbes Work

### Breakpoint Mechanism

When a kprobe is registered at a specific address:
1. One of the FPB comparators is programmed with the target address
2. On every instruction fetch, the FPB compares the program counter against registered comparators
3. When a match occurs, the FPB generates a DebugMon exception
4. The Debug Fault Status Register (DFSR) bit-BKPT is set to indicate a breakpoint hit

### Handler Execution

The kprobe handler execution follows this sequence:
1. DebugMon exception triggers at the probed address
2. All registered `pre_handler` functions for that address are called
3. The kernel sets bit-MON_STEP in Debug Exception and Monitor Control Register
4. Execution returns to the probed instruction for single-step execution
5. Another DebugMon exception triggers after the instruction executes
6. The kernel checks bit-HALTED in DFSR to confirm single-step completion
7. All registered `post_handler` functions are called
8. Normal execution resumes

### Priority Considerations

If a DebugMon exception occurs without sufficient exception priority to preempt current execution, priority escalation causes a Hard Fault. This commonly happens when the `cpsid` instruction has disabled interrupts. Avoid placing kprobes in interrupt-disabled sections.

## API Reference

### Registration Functions

```c
#include <kprobes.h>

int kprobe_register(struct kprobe *kp);
int kprobe_unregister(struct kprobe *kp);
```

Both functions return 0 on success or a negative value on error.

### The kprobe Structure

```c
struct kprobe {
    void *addr;                      /* Address to probe */
    kprobe_pre_handler_t pre_handler;   /* Called before probed instruction */
    kprobe_post_handler_t post_handler; /* Called after probed instruction */
};
```

### Handler Prototypes

```c
typedef int (*kprobe_pre_handler_t)(struct kprobe *kp,
                                    uint32_t *kp_stack,
                                    uint32_t *kp_regs);

typedef int (*kprobe_post_handler_t)(struct kprobe *kp,
                                     uint32_t *kp_stack,
                                     uint32_t *kp_regs);
```

### Handler Parameters

- `kp`: Pointer to the registered kprobe structure
- `kp_stack`: Pointer to the interrupted stack (Main_SP or Process_SP), containing registers r0-r3, r12, lr, pc, psr
- `kp_regs`: Pointer to saved registers r4-r11

### Accessing Registers

Use the provided register offset macros:

```c
/* Access r0 from exception frame */
uint32_t r0_value = kp_stack[REG_R0];

/* Access r4 from saved registers */
uint32_t r4_value = kp_regs[KP_REG_R4];
```

Handlers can read and modify register values through these pointers, allowing inspection and alteration of program state at the probe point.

## Example

Register a kprobe at the `ktimer_handler` function:

```c
#include <kprobes.h>

int my_pre_handler(struct kprobe *kp, uint32_t *stack, uint32_t *regs)
{
    /* Log entry to ktimer_handler */
    dbg_printf(DL_KDB, "ktimer_handler called, r0=%x\n", stack[REG_R0]);
    return 0;
}

struct kprobe ktimer_probe = {
    .addr = ktimer_handler,
    .pre_handler = my_pre_handler,
    .post_handler = NULL,
};

void setup_probes(void)
{
    kprobe_register(&ktimer_probe);
}
```

## Limitations

- Limited number of simultaneous probes (constrained by FPB comparator count)
- Cannot probe code running with interrupts disabled
- ARM Cortex-M specific implementation
- Single-stepping overhead affects timing-sensitive code
