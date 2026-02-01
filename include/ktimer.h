/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef KTIMER_H_
#define KTIMER_H_

#include <types.h>

/* Forward declaration */
struct tcb;

void ktimer_handler(void);

/* Get current kernel time in ticks since boot.
 * Returns 64-bit monotonically increasing tick count.
 * Used by SYS_SYSTEM_CLOCK syscall for userspace time queries.
 */
uint64_t ktimer_get_now(void);

/* Returns 0 if successfully handled
 * or number ticks if need to be rescheduled
 */
typedef uint32_t (*ktimer_event_handler_t)(void *data);


typedef struct ktimer_event {
    struct ktimer_event *next;
    ktimer_event_handler_t handler;

    uint32_t delta;
    void *data;

    /* Notification mode: if notify_thread is non-NULL, timer uses
     * async event notification instead of calling handler directly.
     * This integrates with Event-Chaining + ASYNC_SOFTIRQ subsystem.
     */
    struct tcb *notify_thread; /* Target thread for notification */
    uint32_t notify_bits;      /* Notification bit mask to signal */

    /* Deadline tracking for periodic timers (prevents drift accumulation).
     * For periodic timers, deadline tracks absolute target time.
     * Reschedule based on: next_ticks = max(1, deadline - now)
     * This maintains phase-lock to original schedule even if softirq delayed.
     */
    uint64_t deadline; /* Absolute deadline (in ticks since boot) */
} ktimer_event_t;

void ktimer_event_init(void);

int ktimer_event_schedule(uint32_t ticks, ktimer_event_t *kte);

/* Callback-based timer (traditional API) */
ktimer_event_t *ktimer_event_create(uint32_t ticks,
                                    ktimer_event_handler_t handler,
                                    void *data);

/* Notification-based timer (Event-Chaining + ASYNC_SOFTIRQ integration).
 * When timer expires, posts async event to notify_thread with notify_bits.
 * Thread receives notification via Event-Chaining callback.
 *
 * @param ticks Timer period in ticks
 * @param notify_thread Target thread to notify (must be valid TCB)
 * @param notify_bits Notification bit mask to signal
 * @param periodic If 0, one-shot timer. If non-zero, reschedule with same
 * period.
 * @return Allocated timer event, or NULL if pool exhausted
 *
 * NOTE: For one-shot timers, event is freed automatically after firing.
 *       For periodic timers, event remains allocated until explicitly freed.
 */
ktimer_event_t *ktimer_event_create_notify(uint32_t ticks,
                                           struct tcb *notify_thread,
                                           uint32_t notify_bits,
                                           int periodic);

void ktimer_event_handler(void);

#ifdef CONFIG_KTIMER_TICKLESS
void ktimer_enter_tickless(void);
#endif /* CONFIG_KTIMER_TICKLESS */

#endif /* KTIMER_H_ */
