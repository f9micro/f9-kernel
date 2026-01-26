# Interrupt Handling

F9 implements L4-style interrupt delegation, allowing user-space threads to handle hardware interrupts while the kernel maintains control over interrupt configuration.

## Design Rationale

In traditional operating systems, interrupt handlers run in kernel mode with full privileges.
L4 microkernels take a different approach: hardware interrupts are converted to IPC messages and delivered to user-space handler threads.

This provides:
- Reduced kernel complexity
- Isolated interrupt handling (faults in handlers do not crash the kernel)
- Flexible interrupt assignment at runtime

The kernel retains responsibility for low-level interrupt acknowledgment and priority management while delegating the actual handling logic to user space.

## ARM Cortex-M Interrupt Architecture

ARM Cortex-M processors use the Nested Vectored Interrupt Controller (NVIC) for interrupt management:
- Up to 240 external interrupt sources (implementation defined)
- Programmable priority levels (typically 8-256 levels)
- Hardware priority grouping for preemption control
- Automatic context saving on exception entry

The NVIC vector table contains addresses of exception handlers.
F9 installs its trap handler at each vector entry, which then dispatches to the appropriate kernel or user-space handler.

### Exception Types

| Exception | IRQ Number | Purpose |
|-----------|------------|---------|
| Reset | -3 | System reset |
| NMI | -2 | Non-maskable interrupt |
| HardFault | -1 | Unrecoverable fault |
| MemManage | 4 | MPU violation |
| BusFault | 5 | Bus error |
| UsageFault | 6 | Undefined instruction |
| SVCall | 11 | System call |
| PendSV | 14 | Context switch |
| SysTick | 15 | System timer |
| IRQ0-IRQn | 16+ | External interrupts |

F9 reserves SysTick for kernel timer operations. External interrupts (IRQ0 and above) can be delegated to user threads.

## Requesting Interrupt Delegation

User threads request interrupt delegation via IPC to the special `THREAD_IRQ_REQUEST` thread ID:

```c
void request_interrupt(int irq_num, irq_handler_t handler, int priority)
{
    L4_Msg_t msg;
    L4_MsgClear(&msg);

    L4_Word_t data[5] = {
        irq_num,            /* IRQ number */
        my_thread_id,       /* Handler thread */
        IRQ_ACTION_ENABLE,  /* Action */
        (L4_Word_t)handler, /* Handler function pointer */
        priority            /* Interrupt priority */
    };

    ipc_msg_tag_t tag = { .s = { .label = USER_INTERRUPT_LABEL } };
    L4_MsgPut(&msg, tag.raw, 5, data, 0, NULL);
    L4_MsgLoad(&msg);
    L4_Ipc(TID_TO_GLOBALID(THREAD_IRQ_REQUEST), L4_NILTHREAD, 0, NULL);
}
```

## Kernel Data Structures

The kernel maintains a table of user interrupt registrations:

```c
struct user_irq {
    tcb_t *thr;              /* Handler thread */
    int irq;                 /* IRQ number */
    uint16_t action;         /* Enable/disable action */
    uint16_t priority;       /* Interrupt priority */
    irq_handler_t handler;   /* User handler function */
    struct user_irq *next;   /* Queue linkage */
};
```

The `user_irqs[]` array is indexed by IRQ number. A pending interrupt queue tracks interrupts awaiting delivery.

## Interrupt Delivery Flow

When an interrupt occurs:
1. NVIC saves context and vectors to the kernel exception handler
2. Kernel identifies the interrupt source from IPSR (Interrupt Program Status Register)
3. Kernel acknowledges the interrupt in NVIC
4. Kernel looks up the interrupt in `user_irqs[]`
5. If a handler is registered:
   - Push the interrupt onto the pending queue
   - Construct an IPC message with interrupt number and handler
   - Mark the handler thread as ready to receive
6. Scheduler prioritizes interrupt handler threads
7. Handler thread receives the IPC message
8. Handler thread executes the user-space handler function
9. Handler thread acknowledges completion (clears peripheral interrupt source)

## Configuration Function

The kernel's `user_interrupt_config()` processes delegation requests:

```c
void user_interrupt_config(tcb_t *from)
{
    ipc_msg_tag_t tag = { .raw = ipc_read_mr(from, 0) };
    if (tag.s.label != USER_INTERRUPT_LABEL)
        return;

    int irq = (uint16_t)ipc_read_mr(from, 1);
    l4_thread_t tid = ipc_read_mr(from, 2);
    int action = (uint16_t)ipc_read_mr(from, 3);
    irq_handler_t handler = (irq_handler_t)ipc_read_mr(from, 4);
    int priority = (uint16_t)ipc_read_mr(from, 5);

    /* Disable interrupt during reconfiguration */
    user_irq_disable(irq);

    if (!IS_VALID_IRQ_NUM(irq))
        return;

    struct user_irq *uirq = user_irq_fetch(irq);
    if (!uirq)
        return;

    /* Update registration */
    if (tid != L4_NILTHREAD)
        uirq->thr = thread_by_globalid(tid);
    uirq->action = action;
    if (handler)
        uirq->handler = handler;
    if (priority > 0)
        uirq->priority = priority;
}
```

## Waiting for Interrupts

Handler threads wait for interrupts using IPC receive:

```c
void interrupt_handler_thread(void)
{
    while (1) {
        L4_ThreadId_t sender;
        L4_MsgTag_t tag = L4_Ipc(L4_NILTHREAD,
                                 TID_TO_GLOBALID(THREAD_INTERRUPT),
                                 L4_NEVER,
                                 &sender);

        /* Extract interrupt info from message */
        L4_Msg_t msg;
        L4_MsgStore(tag, &msg);
        int irq = L4_MsgWord(&msg, 0);
        irq_handler_t handler = (irq_handler_t)L4_MsgWord(&msg, 1);

        /* Call the handler */
        if (handler)
            handler();

        /* Clear peripheral interrupt source */
        clear_peripheral_interrupt(irq);
    }
}
```

## Scheduling Priority

Interrupt handler threads receive elevated scheduling priority (`SCHED_PRIO_INTR = 1`).
The scheduler ensures they run promptly after the kernel delivers the IPC message.

The scheduling order:
1. Softirq thread (priority 0)
2. Interrupt handler threads (priority 1)
3. Root thread (priority 2)
4. User threads (priorities 3-30)
5. Idle thread (priority 31)

IRQ threads can use Preemption-Threshold Scheduling (PTS) to protect critical sections without disabling interrupts.
For complete scheduling documentation, see [scheduler.md](scheduler.md).

## Hardware Setup

Before requesting delegation, the peripheral and NVIC must be configured:

```c
void setup_uart_interrupt(void)
{
    /* 1. Configure peripheral to generate interrupts */
    uart_enable_rx_interrupt();

    /* 2. Set NVIC priority */
    NVIC_SetPriority(UART_IRQn, PRIORITY);

    /* 3. Enable interrupt in NVIC */
    NVIC_EnableIRQ(UART_IRQn);

    /* 4. Request kernel delegation */
    request_interrupt(UART_IRQn, uart_handler, PRIORITY);
}
```

## Interrupt Acknowledgment

The user handler is responsible for clearing the peripheral interrupt source. The kernel clears the pending bit in NVIC, but the peripheral may re-assert the interrupt if its source is not cleared. Failure to clear the peripheral interrupt will cause immediate re-triggering.

## NVIC Configuration

F9 configures NVIC priority grouping to allow preemption between interrupt priorities. The kernel reserves the highest priority levels for internal use (fault handlers, SysTick).

```c
/* Set priority grouping: 4 bits preemption, 0 bits subpriority */
NVIC_SetPriorityGrouping(3);
```

User interrupt priorities should be set below the kernel reserved levels to ensure proper system operation.

## Limitations

- SysTick reserved for kernel timer (cannot be delegated)
- Handler thread must be created before requesting delegation
- Nested interrupt delegation is not supported
- Real-time guarantees depend on scheduler behavior
- Maximum delegatable interrupts depends on NVIC implementation
