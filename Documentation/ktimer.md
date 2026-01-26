# Kernel Timer (ktimer)

The kernel timer subsystem manages time-based events and provides the foundation for preemptive scheduling, IPC timeouts, and other timing-dependent kernel operations.

## Design Philosophy

In L4 microkernels, hardware interrupts are delivered as synchronous IPC messages to user-level handler threads (see [interrupt.md](interrupt.md)). The timer tick interrupt is the only interrupt managed internally by the kernel. F9 follows this L4 convention, with ktimer serving as the internal handler for timer tick interrupts.

## Scheduler Integration

The scheduler operates on a priority-based model (see [threads.md](threads.md) for details):
1. Starting from the highest priority, the scheduler checks for runnable threads
2. If a runnable thread exists at that priority, it is selected for dispatch
3. If no runnable threads exist, the idle thread is dispatched
4. When a thread blocks, yields, or exhausts its timeslice, the next thread in that priority class runs

### Timeslice Accounting

Timeslice tracking occurs in the periodic timer interrupt handler by subtracting the timer period from the remaining timeslice. When the remaining timeslice reaches zero or becomes negative, the timeslice is exhausted, triggering immediate rescheduling.

This accounting is slightly imprecise: if a thread blocks between timer ticks, the next thread starts with only a partial timer period until the next tick.

## Event Queue

F9 maintains a delta queue of kernel timer events (kte). The queue is ordered by time, with the soonest event at the head. Each entry stores a delta value representing ticks until that event relative to the previous entry.

### Event Structure

```c
typedef struct ktimer_event {
    struct ktimer_event *next;
    ktimer_event_handler_t handler;
    uint32_t delta;
    void *data;
} ktimer_event_t;
```

### Scheduling Events

When scheduling a new timer event:
1. The current tick count is added to the requested ticks
2. The queue is traversed, accumulating deltas
3. When accumulated time exceeds the target, the event is inserted
4. The following event's delta is recalculated

### Handler Functions

Event handlers have the signature:

```c
uint32_t handler(void *data);
```

The return value controls rescheduling:
- Return 0: Event is complete, do not reschedule
- Return non-zero: Reschedule the event with the returned tick count

This allows both one-shot and periodic events using the same mechanism.

## Timeout Management

Threads blocked during IPC (see [ipc.md](ipc.md)) can be unblocked by completing their IPC or by timeout expiration. Rather than checking all blocked threads on every tick, F9 maintains the timeout list with the nearest expiry at the head. This requires checking only a single timeout per tick.

This design also enables tickless implementations where the timer is set to `min(timeslice_length, earliest_timeout)` on each kernel exit.

## Hardware Timer Support

The ktimer abstraction supports different ARM Cortex-M timer configurations:

### SysTick Timer

The Cortex-M SysTick timer is the primary timer source for F9:

- 24-bit countdown timer
- Configurable clock source (processor clock or external reference)
- Generates SysTick exception on underflow
- Automatic reload from SYST_RVR register

```c
/* SysTick registers */
#define SYST_CSR   0xE000E010  /* Control and Status */
#define SYST_RVR   0xE000E014  /* Reload Value */
#define SYST_CVR   0xE000E018  /* Current Value */
#define SYST_CALIB 0xE000E01C  /* Calibration Value */
```

### Timer Configuration

F9 configures SysTick during initialization:

1. Calculate reload value from `CONFIG_KTIMER_HEARTBEAT`
2. Clear current value register
3. Enable SysTick with processor clock and interrupt

The SysTick exception priority is set high to ensure timely preemption.

## Configuration Options

| Option | Description |
|--------|-------------|
| `CONFIG_KTIMER_HEARTBEAT` | Hardware cycles per ktimer tick |
| `CONFIG_KTIMER_MINTICKS` | Minimum ktimer ticks unit for time events |
| `CONFIG_KTIMER_TICKLESS` | Enable tickless operation |

## Tickless Operation

When tickless mode is enabled:
1. The kernel enters tickless state before CPU idle
2. The next timer interrupt interval is set to the delta of the next event (or `KTIMER_MAXTICKS`)
3. System time is adjusted when the kernel wakes

### Trade-offs

Tickless operation increases instruction count on the path to and from the idle loop. Systems with aggressive real-time response constraints often prefer periodic ticks for more predictable timing behavior.

## Related Documentation

- [threads.md](threads.md) - Thread scheduling and timeslices
- [ipc.md](ipc.md) - IPC timeout handling
- [interrupt.md](interrupt.md) - Interrupt delivery model
