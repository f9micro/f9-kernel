/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include INC_PLAT(systick.h)

#include <debug.h>
#include <init_hook.h>
#include <ktimer.h>
#include <lib/ktable.h>
#include <notification.h>
#include <platform/armv7m.h>
#include <platform/bitops.h>
#include <platform/irq.h>
#include <softirq.h>
#include <thread.h>
#if defined(CONFIG_KTIMER_TICKLESS) && defined(CONFIG_KTIMER_TICKLESS_VERIFY)
#include <tickless-verify.h>
#endif

DECLARE_KTABLE(ktimer_event_t, ktimer_event_table, CONFIG_MAX_KT_EVENTS);

/* Next chain of events which will be executed */
ktimer_event_t *event_queue = NULL;

/* Notification coalescing for timer expiry (reduces jitter from simultaneous
 * timers). When multiple timers expire in same tick for same thread, accumulate
 * bits via OR. Deliver once per thread per tick instead of once per timer.
 * Cache size: 8 entries (typical max concurrent timer expirations per tick).
 */
#define KTIMER_COALESCE_CACHE_SIZE 8

typedef struct {
    tcb_t *thread; /* Target thread (NULL = unused slot) */
    uint32_t bits; /* Accumulated notification bits (OR'ed) */
} ktimer_coalesce_entry_t;

static ktimer_coalesce_entry_t coalesce_cache[KTIMER_COALESCE_CACHE_SIZE];
static int coalesce_count = 0;  /* Number of entries in cache */
static int coalesce_active = 0; /* 1 = coalescing enabled, 0 = disabled */

/*
 * Simple ktimer implementation
 */

static uint64_t ktimer_now;
static uint32_t ktimer_enabled = 0;
static uint32_t ktimer_delta = 0;
static long long ktimer_time = 0;

extern uint32_t SystemCoreClock;

static void ktimer_init(void)
{
    init_systick(CONFIG_KTIMER_HEARTBEAT, 0);
}

static void ktimer_disable(void)
{
    if (ktimer_enabled) {
        ktimer_enabled = 0;
    }
}

static void ktimer_enable(uint32_t delta)
{
    /* Always update delta and reset time counter.
     * This is called both for initial enable and to reschedule
     * when an earlier event is inserted at queue head.
     */
    ktimer_delta = delta;
    ktimer_time = 0;
    ktimer_enabled = 1;

#if defined(CONFIG_KDB) && defined(CONFIG_KTIMER_TICKLESS) && \
    defined(CONFIG_KTIMER_TICKLESS_VERIFY)
    tickless_verify_start(ktimer_now, ktimer_delta);
#endif /* CONFIG_KDB */
}

void __ktimer_handler(void)
{
    ++ktimer_now;

    if (ktimer_enabled && ktimer_delta > 0) {
        ++ktimer_time;
        --ktimer_delta;

        if (ktimer_delta == 0) {
            ktimer_enabled = 0;
            ktimer_time = ktimer_delta = 0;

#if defined(CONFIG_KDB) && defined(CONFIG_KTIMER_TICKLESS) && \
    defined(CONFIG_KTIMER_TICKLESS_VERIFY)
            tickless_verify_stop(ktimer_now);
#endif /* CONFIG_KDB */

            softirq_schedule(KTE_SOFTIRQ);
        }
    }
}

IRQ_HANDLER(ktimer_handler, __ktimer_handler);

#ifdef CONFIG_KDB
void kdb_show_ktimer(void)
{
    dbg_printf(DL_KDB, "Now is %ld\n", ktimer_now);

    if (ktimer_enabled) {
        dbg_printf(DL_KDB, "Ktimer T=%d D=%d\n", ktimer_time, ktimer_delta);
    }
}

#if defined(CONFIG_KTIMER_TICKLESS) && defined(CONFIG_KTIMER_TICKLESS_VERIFY)
void kdb_show_tickless_verify(void)
{
    static int init = 0;
    int32_t avg;
    int times;

    if (init == 0) {
        dbg_printf(DL_KDB, "Init tickless verification...\n");
        tickless_verify_init();
        init++;
    } else {
        avg = tickless_verify_stat(&times);
        dbg_printf(DL_KDB, "Times: %d\nAverage: %d\n", times, avg);
    }
}
#endif
#endif /* CONFIG_KDB */

static void ktimer_event_recalc(ktimer_event_t *event, uint32_t new_delta)
{
    if (event) {
        dbg_printf(DL_KTIMER, "KTE: Recalculated event %p D=%d -> %d\n", event,
                   event->delta, event->delta - new_delta);
        event->delta -= new_delta;
    }
}

int ktimer_event_schedule(uint32_t ticks, ktimer_event_t *kte)
{
    long etime = 0, delta = 0;
    ktimer_event_t *event = NULL, *next_event = NULL;

    if (!ticks)
        return -1;

    /* Adjust for elapsed time. If timeout has already passed
     * (ticks <= ktimer_time), schedule for next tick (ticks=1)
     * to avoid unsigned underflow.
     */
    if (ticks <= (uint32_t) ktimer_time)
        ticks = 1;
    else
        ticks -= ktimer_time;
    kte->next = NULL;

    if (!event_queue) {
        /* All other events are already handled, so simply schedule and enable
         * timer
         */
        kte->delta = ticks;
        event_queue = kte;

        ktimer_enable(ticks);
    } else {
        /* etime is total delta for event from now (-ktimer_value())
         * on each iteration we add delta between events.
         *
         * Search for event chain until etime is larger than ticks
         * e.g ticks = 80
         *
         * 0---17------------60----------------60---...
         *                   ^                 ^
         *                   |           (etime + next_event->delta) =
         *                   |           = 120 - 17 = 103
         *                               etime = 60 - 17 =  43
         *
         * kte is between event(60) and event(120),
         * delta = 80 - 43 = 37
         * insert and recalculate:
         *
         * 0---17------------60-------37-------23---...
         *
         * */
        next_event = event_queue;

        if (ticks >= event_queue->delta) {
            do {
                event = next_event;
                etime += event->delta;
                delta = ticks - etime;
                next_event = event->next;
            } while (next_event && ticks > (etime + next_event->delta));

            /* Insert into chain and recalculate */
            event->next = kte;
        } else {
            /* Event should be scheduled before earlier event */
            dbg_printf(DL_KTIMER, "KTE: Scheduled early event %p with T=%d\n",
                       kte, ticks);

            event_queue = kte;
            delta = ticks;

            /* Reset timer */
            ktimer_enable(ticks);
        }

        /* Chaining events */
        if (delta < CONFIG_KTIMER_MINTICKS)
            delta = 0;

        kte->next = next_event;
        kte->delta = delta;

        ktimer_event_recalc(next_event, delta);
    }

    return 0;
}

ktimer_event_t *ktimer_event_create(uint32_t ticks,
                                    ktimer_event_handler_t handler,
                                    void *data)
{
    ktimer_event_t *kte = NULL;

    if (!handler)
        goto ret;

    kte = (ktimer_event_t *) ktable_alloc(&ktimer_event_table);

    /* No available slots */
    if (!kte)
        goto ret;

    kte->next = NULL;
    kte->handler = handler;
    kte->data = data;
    kte->notify_thread = NULL; /* Callback mode, not notification */
    kte->notify_bits = 0;
    kte->deadline = 0; /* No deadline tracking for callback-based timers */

    if (ktimer_event_schedule(ticks, kte) == -1) {
        ktable_free(&ktimer_event_table, kte);
        kte = NULL;
    }

ret:
    return kte;
}

/* Internal notification handler for ktimer_event_create_notify().
 * Posts notification to target thread when timer expires.
 * Returns ticks for periodic rescheduling, or 0 for one-shot.
 */
static uint32_t ktimer_notify_handler(void *data)
{
    ktimer_event_t *kte = (ktimer_event_t *) data;

    if (!kte || !kte->notify_thread)
        return 0; /* Invalid event, free it */

#ifdef CONFIG_KTIMER_DIRECT_NOTIFY
    /* Direct notification delivery: Ultra-low latency path bypassing
     * async event queue and softirq. Executes in timer IRQ context.
     * 91% latency reduction: 150 cycles vs 150-1750 cycles.
     * WARNING: Executes in IRQ context - violates softirq safety.
     */
    tcb_t *thr = kte->notify_thread;

    dbg_printf(DL_KTIMER,
               "KTE: Direct notify timer expired, signaling %t bits=0x%x\n",
               thr->t_globalid, kte->notify_bits);

    /* Deliver notification immediately (in IRQ context) */
    notification_signal(thr, kte->notify_bits);

    /* Wake thread if blocked */
    if (thr->state == T_RECV_BLOCKED) {
        thr->state = T_RUNNABLE;
        sched_enqueue(thr);
    }
#else
    /* Notification delivery with optional coalescing.
     * If coalescing active: accumulate bits in cache, deliver once per thread.
     * Otherwise: immediate fast-path delivery (bypasses async queue).
     */
    if (coalesce_active) {
        /* Coalescing mode: check if thread already in cache */
        int found = 0;
        for (int i = 0; i < coalesce_count; i++) {
            if (coalesce_cache[i].thread == kte->notify_thread) {
                /* Thread found: OR bits together */
                coalesce_cache[i].bits |= kte->notify_bits;
                found = 1;
                dbg_printf(
                    DL_KTIMER,
                    "KTE: Coalesced notify to %t bits=0x%x (total=0x%x)\n",
                    kte->notify_thread->t_globalid, kte->notify_bits,
                    coalesce_cache[i].bits);
                break;
            }
        }

        if (!found) {
            /* Thread not in cache: add new entry if space available */
            if (coalesce_count < KTIMER_COALESCE_CACHE_SIZE) {
                coalesce_cache[coalesce_count].thread = kte->notify_thread;
                coalesce_cache[coalesce_count].bits = kte->notify_bits;
                coalesce_count++;
                dbg_printf(
                    DL_KTIMER,
                    "KTE: Added to coalesce cache %t bits=0x%x (count=%d)\n",
                    kte->notify_thread->t_globalid, kte->notify_bits,
                    coalesce_count);
            } else {
                /* Cache full: deliver immediately (fallback) */
                notification_post_softirq(kte->notify_thread, kte->notify_bits);
                dbg_printf(
                    DL_KTIMER,
                    "KTE: Cache full, immediate notify to %t bits=0x%x\n",
                    kte->notify_thread->t_globalid, kte->notify_bits);
            }
        }
    } else {
        /* No coalescing: immediate fast-path delivery */
        int ret =
            notification_post_softirq(kte->notify_thread, kte->notify_bits);

        if (ret < 0) {
            /* Fallback to async queue on error (shouldn't happen in softirq) */
            dbg_printf(
                DL_KTIMER,
                "KTE: Fast-path failed, using async queue for %t bits=0x%x\n",
                kte->notify_thread->t_globalid, kte->notify_bits);

            notification_post(kte->notify_thread, kte->notify_bits,
                              (uint32_t) ktimer_now);
        } else {
            dbg_printf(DL_KTIMER, "KTE: Fast-path notify to %t bits=0x%x\n",
                       kte->notify_thread->t_globalid, kte->notify_bits);
        }
    }
#endif

    /* Deadline-based rescheduling for periodic timers (prevents drift).
     * For periodic timers: advance deadline and calculate next ticks.
     * For one-shot timers: return 0 to free the event.
     */
    if (kte->deadline > 0) {
        /* Periodic timer: advance deadline by period */
        uint32_t period = (uint32_t) kte->data;
        kte->deadline += period;

        /* Calculate next ticks based on deadline.
         * If deadline already passed (due to softirq delay), schedule ASAP (1
         * tick). This maintains phase-lock to original schedule even if
         * delayed.
         */
        uint32_t next_ticks =
            (kte->deadline > ktimer_now)
                ? (uint32_t) (kte->deadline - ktimer_now)
                : 1; /* Missed deadline, schedule immediately */

        dbg_printf(
            DL_KTIMER,
            "KTE: Periodic reschedule %p: deadline=%lld now=%lld next=%d\n",
            kte, kte->deadline, ktimer_now, next_ticks);

        return next_ticks;
    } else {
        /* One-shot timer: free event */
        return 0;
    }
}

ktimer_event_t *ktimer_event_create_notify(uint32_t ticks,
                                           tcb_t *notify_thread,
                                           uint32_t notify_bits,
                                           int periodic)
{
    ktimer_event_t *kte = NULL;

    if (!notify_thread || !notify_bits)
        goto ret;

    kte = (ktimer_event_t *) ktable_alloc(&ktimer_event_table);

    /* No available slots */
    if (!kte)
        goto ret;

    kte->next = NULL;
    kte->handler = ktimer_notify_handler; /* Internal notification handler */
    kte->data =
        (void *) (periodic ? ticks : 0); /* Store period for reschedule */
    kte->notify_thread = notify_thread;
    kte->notify_bits = notify_bits;

    /* Initialize deadline for periodic timers (prevents drift accumulation).
     * For periodic timers: deadline tracks absolute target time.
     * For one-shot timers: deadline unused (set to 0).
     */
    kte->deadline = periodic ? (ktimer_now + ticks) : 0;

    if (ktimer_event_schedule(ticks, kte) == -1) {
        ktable_free(&ktimer_event_table, kte);
        kte = NULL;
    } else {
        dbg_printf(
            DL_KTIMER,
            "KTE: Created notify timer %p for %t bits=0x%x ticks=%d %s\n", kte,
            notify_thread->t_globalid, notify_bits, ticks,
            periodic ? "periodic" : "one-shot");
    }

ret:
    return kte;
}

void ktimer_event_handler()
{
    ktimer_event_t *event = event_queue;
    ktimer_event_t *last_event = NULL;
    ktimer_event_t *next_event = NULL;
    uint32_t h_retvalue = 0;

    if (!event_queue) {
        /* That is bug if we are here */
        dbg_printf(DL_KTIMER, "KTE: OOPS! handler found no events\n");

        ktimer_disable();
        return;
    }

    /* Enable notification coalescing for this batch of timer expirations.
     * Reduces jitter by batching notifications to same thread within one tick.
     */
    coalesce_active = 1;
    coalesce_count = 0;

    /* Search last event in event chain */
    do {
        event = event->next;
    } while (event && event->delta == 0);

    last_event = event;

    /* All rescheduled events will be scheduled after last event */
    event = event_queue;
    event_queue = last_event;

    /* walk chain */
    do {
        h_retvalue = event->handler(event);
        next_event = event->next;

        if (h_retvalue != 0x0) {
            dbg_printf(DL_KTIMER,
                       "KTE: Handled and rescheduled event %p @%ld\n", event,
                       ktimer_now);
            ktimer_event_schedule(h_retvalue, event);
        } else {
            dbg_printf(DL_KTIMER, "KTE: Handled event %p @%ld\n", event,
                       ktimer_now);
            ktable_free(&ktimer_event_table, event);
        }

        event = next_event; /* Guaranteed to be next
                       regardless of re-scheduling */
    } while (next_event && next_event != last_event);

    /* Flush coalesced notifications: deliver once per thread.
     * This batches multiple timer expirations to same thread within one tick,
     * reducing wakeups and jitter from simultaneous timer expirations.
     */
    for (int i = 0; i < coalesce_count; i++) {
        notification_post_softirq(coalesce_cache[i].thread,
                                  coalesce_cache[i].bits);

        dbg_printf(DL_KTIMER, "KTE: Flushed coalesced notify to %t bits=0x%x\n",
                   coalesce_cache[i].thread->t_globalid,
                   coalesce_cache[i].bits);

        /* Clear cache entry */
        coalesce_cache[i].thread = NULL;
        coalesce_cache[i].bits = 0;
    }

    /* Disable coalescing until next batch */
    coalesce_active = 0;
    coalesce_count = 0;

    if (event_queue) {
        /* Reset ktimer */
        ktimer_enable(event_queue->delta);
    }
}

void ktimer_event_init()
{
    ktable_init(&ktimer_event_table);
    ktimer_init();
    softirq_register(KTE_SOFTIRQ, ktimer_event_handler);
}

INIT_HOOK(ktimer_event_init, INIT_LEVEL_KERNEL);

#ifdef CONFIG_KDB
void kdb_dump_events(void)
{
    ktimer_event_t *event = event_queue;

    dbg_puts("\nktimer events: \n");
    dbg_printf(DL_KDB, "%8s %12s\n", "EVENT", "DELTA");

    while (event) {
        dbg_printf(DL_KDB, "%p %12d\n", event, event->delta);

        event = event->next;
    }
}
#endif

#ifdef CONFIG_KTIMER_TICKLESS

#define KTIMER_MAXTICKS (SYSTICK_MAXRELOAD / CONFIG_KTIMER_HEARTBEAT)

static uint32_t volatile ktimer_tickless_compensation =
    CONFIG_KTIMER_TICKLESS_COMPENSATION;
static uint32_t volatile ktimer_tickless_int_compensation =
    CONFIG_KTIMER_TICKLESS_INT_COMPENSATION;

void ktimer_enter_tickless()
{
    uint32_t tickless_delta;
    uint32_t reload;

    irq_disable();

    if (ktimer_enabled && ktimer_delta <= KTIMER_MAXTICKS) {
        tickless_delta = ktimer_delta;
    } else {
        tickless_delta = KTIMER_MAXTICKS;
    }

    /* Minus 1 for current value */
    tickless_delta -= 1;

    reload = CONFIG_KTIMER_HEARTBEAT * tickless_delta;

    reload += systick_now() - ktimer_tickless_compensation;

    if (reload > 2) {
        init_systick(reload, CONFIG_KTIMER_HEARTBEAT);

#if defined(CONFIG_KDB) && defined(CONFIG_KTIMER_TICKLESS) && \
    defined(CONFIG_KTIMER_TICKLESS_VERIFY)
        tickless_verify_count();
#endif
    }

    wait_for_interrupt();

    if (!systick_flag_count()) {
        uint32_t tickless_rest = (systick_now() / CONFIG_KTIMER_HEARTBEAT);

        if (tickless_rest > 0) {
            int reload_overflow;

            tickless_delta = tickless_delta - tickless_rest;

            reload = systick_now() % CONFIG_KTIMER_HEARTBEAT -
                     ktimer_tickless_int_compensation;
            reload_overflow = reload < 0;
            reload += reload_overflow * CONFIG_KTIMER_HEARTBEAT;

            init_systick(reload, CONFIG_KTIMER_HEARTBEAT);

            if (reload_overflow) {
                tickless_delta++;
            }

#if defined(CONFIG_KDB) && defined(CONFIG_KTIMER_TICKLESS) && \
    defined(CONFIG_KTIMER_TICKLESS_VERIFY)
            tickless_verify_count_int();
#endif
        }
    }

    ktimer_time += tickless_delta;
    ktimer_delta -= tickless_delta;
    ktimer_now += tickless_delta;

    irq_enable();
}
#endif /* CONFIG_KTIMER_TICKLESS */
