/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <sched.h>
#include <thread.h>
#include <error.h>
#include <init_hook.h>

/**
 * @file sched.c
 * @brief Thread scheduler
 *
 * We use several scheduler slots for each
 */

static sched_slot_t slots[NUM_SCHED_SLOTS];

void sched_init()
{
	for (int slot_id = 0; slot_id < NUM_SCHED_SLOTS; ++slot_id) {
		slots[slot_id].ss_scheduled = NULL;
		slots[slot_id].ss_handler =   NULL;
	}
}

INIT_HOOK(sched_init, INIT_LEVEL_KERNEL_EARLY);

tcb_t *schedule_select()
{
	/* For each scheduler slot try to dispatch thread from it */
	for (int slot_id = 0; slot_id < NUM_SCHED_SLOTS; ++slot_id) {
		tcb_t *scheduled = slots[slot_id].ss_scheduled;

		if (scheduled && thread_isrunnable(scheduled)) {
			/* Found thread, try to dispatch it */
			return scheduled;
		} else if (slots[slot_id].ss_handler) {
			/* No appropriate thread found (i.e. timeslice
			 * exhausted, no softirqs in kernel),
			 * try to redispatch another thread
			 */
			scheduled = slots[slot_id].ss_handler(&slots[slot_id]);

			if (scheduled) {
				slots[slot_id].ss_scheduled = scheduled;
				return scheduled;
			}
		}
	}

	/* not reached (last slot is IDLE which is always runnable) */
	panic("Reached end of schedule()\n");
	return NULL;
}

int schedule()
{
	tcb_t *scheduled = schedule_select();
	thread_switch(scheduled);
	return 1;
}

/*
 * Dispatch thread in slot
 */
void sched_slot_dispatch(sched_slot_id_t slot_id, tcb_t *thread)
{
	slots[slot_id].ss_scheduled = thread;
}

void sched_slot_set_handler(sched_slot_id_t slot_id, sched_handler_t handler)
{
	slots[slot_id].ss_handler = handler;
}
