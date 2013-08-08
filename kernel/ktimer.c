/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include INC_PLAT(systick.h)

#include <debug.h>
#include <ktimer.h>
#include <lib/ktable.h>
#include <softirq.h>
#include <platform/armv7m.h>
#include <platform/bitops.h>
#include <platform/irq.h>
#include <config.h>
#include <tickless_verify.h>

DECLARE_KTABLE(ktimer_event_t, ktimer_event_table, CONFIG_MAX_KT_EVENTS);

/* Next chain of events which will be executed */
ktimer_event_t *event_queue = NULL;

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
	init_systick(CONFIG_KTIMER_HEARTBEAT);
}

static void ktimer_disable(void)
{
	if (ktimer_enabled) {
		ktimer_enabled = 0;
	}
}

static void ktimer_enable(uint32_t delta)
{
	if (!ktimer_enabled) {
		ktimer_delta = delta;
		ktimer_time = 0;
		ktimer_enabled = 1;

#ifdef CONFIG_KDB
		tickless_verify_start(ktimer_now, ktimer_delta);
#endif	/* CONFIG_KDB */
	}
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

#ifdef CONFIG_KDB
			tickless_verify_stop(ktimer_now);
#endif	/* CONFIG_KDB */

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
		dbg_printf(DL_KDB,
			"Ktimer T=%d D=%d\n", ktimer_time, ktimer_delta);
	}
}

void kdb_show_tickless_verify(void)
{
	static int init = 0;
	int32_t avg;
	int times;

	if (init == 0) {
		dbg_printf(DL_KDB, "Init tickless verification...\n");
		tickless_verify_init();
		init++;
	}
	else {
		avg = tickless_verify_stat(&times);
		dbg_printf(DL_KDB, "Times: %d\nAverage: %d\n", times, avg);
	}
}
#endif	/* CONFIG_KDB */

static void ktimer_event_recalc(ktimer_event_t* event, uint32_t new_delta)
{
	while (event) {
		dbg_printf(DL_KTIMER,
			"KTE: Recalculated event %p D=%d -> %d\n",
			event, event->delta, event->delta - new_delta);
		event->delta -= new_delta;
		event = event->next;
	}
}

int ktimer_event_schedule(uint32_t ticks, ktimer_event_t *kte)
{
	long etime = 0, delta = 0;
	ktimer_event_t *event = NULL, *next_event = NULL;

	if (!ticks)
		return -1;
	ticks -= ktimer_time;
	kte->next = NULL;

	if (event_queue == NULL) {
		/* All other events are already handled, so simply schedule
		 * and enable timer
		 */
		dbg_printf(DL_KTIMER,
			"KTE: Scheduled dummy event %p on %d\n",
			kte, ticks);

		kte->delta = ticks;
		event_queue = kte;

		ktimer_enable(ticks);
	}
	else {
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
			} while (next_event &&
					ticks > (etime + next_event->delta));

			dbg_printf(DL_KTIMER,
				"KTE: Scheduled event %p [%p:%p] with "
				"D=%d and T=%d\n",
				kte, event, next_event, delta, ticks);

			/* Insert into chain and recalculate */
			event->next = kte;
		}
		else {
			/* Event should be scheduled before earlier event */
			dbg_printf(DL_KTIMER,
				"KTE: Scheduled early event %p with T=%d\n",
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

int ktimer_event_create(uint32_t ticks, ktimer_event_handler_t handler, void *data)
{
	ktimer_event_t *kte;

	if (!handler)
		return -1;

	kte = (ktimer_event_t *) ktable_alloc(&ktimer_event_table);

	/* No available slots */
	if (kte == NULL)
		return -1;

	kte->next = NULL;
	kte->handler = handler;
	kte->data = data;

	return ktimer_event_schedule(ticks, kte);
}

void ktimer_event_handler()
{
	ktimer_event_t *event = event_queue;
	ktimer_event_t *last_event = NULL;
	ktimer_event_t *next_event = NULL;
	uint32_t h_retvalue  = 0;

	if (!event_queue) {
		/* That is bug if we are here */
		dbg_printf(DL_KTIMER, "KTE: OOPS! handler found no events\n");

		ktimer_disable();
		return;
	}

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
				"KTE: Handled and rescheduled event %p @%ld\n",
				event, ktimer_now);
			ktimer_event_schedule(h_retvalue, event);
		}
		else {
			dbg_printf(DL_KTIMER,
				"KTE: Handled event %p @%ld\n",
				event, ktimer_now);
			ktable_free(&ktimer_event_table, event);
		}

		event = next_event;	/* Guaranteed to be next
					   regardless of re-scheduling */
	} while (next_event && next_event != last_event);

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

void ktimer_enter_tickless()
{
	uint32_t tickless_delta;
	uint32_t reload;

	irq_disable();

	systick_disable();

	if (ktimer_enabled && ktimer_delta <= KTIMER_MAXTICKS) {
		tickless_delta = ktimer_delta;
	}
	else {
		tickless_delta = KTIMER_MAXTICKS;
	}

	/* Minus 1 for current value */
	tickless_delta -= 1;

	reload = CONFIG_KTIMER_HEARTBEAT * tickless_delta;

	reload += systick_now();

	init_systick(reload);

	wait_for_interrupt();

	if (systick_flag_count()) {
		reload = CONFIG_KTIMER_HEARTBEAT - (reload - systick_now());
	}
	else {
		uint32_t tickless_rest = (systick_now() / CONFIG_KTIMER_HEARTBEAT);
		tickless_delta = tickless_delta - tickless_rest - 1;
		reload = systick_now() - CONFIG_KTIMER_HEARTBEAT * tickless_rest;
	}

	ktimer_time += tickless_delta;
	ktimer_delta -= tickless_delta;
	ktimer_now += tickless_delta;

	init_systick(reload);

	irq_enable();
}
#endif /* CONFIG_KTIMER_TICKLESS */
