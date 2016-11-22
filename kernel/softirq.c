/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <types.h>
#include <platform/irq.h>
#include <platform/bitops.h>
#include <softirq.h>
#include <debug.h>
#include <systhread.h>

static softirq_t softirq[NR_SOFTIRQ];

void softirq_register(softirq_type_t type, softirq_handler_t handler)
{
	softirq[type].handler = handler;
	softirq[type].schedule = 0;
}

void softirq_schedule(softirq_type_t type)
{
	atomic_set(&(softirq[type].schedule), 1);
	set_kernel_state(T_RUNNABLE);
}

static char *softirq_names[NR_SOFTIRQ] __attribute__((used)) = {
	"Kernel timer events",
	"Asynchronous events",
	"System calls",
#ifdef CONFIG_KDB
	"KDB enters",
#endif
};

int softirq_execute()
{
	uint32_t softirq_schedule = 0, executed = 0;
retry:
	for (int i = 0; i < NR_SOFTIRQ; ++i) {
		if (atomic_get(&(softirq[i].schedule)) != 0 &&
		    softirq[i].handler) {
			softirq[i].handler();

			executed = 1;
			atomic_set(&(softirq[i].schedule), 0);

			dbg_printf(DL_SOFTIRQ,
			           "SOFTIRQ: executing %s\n", softirq_names[i]);
		}
	}

	/* Must ensure that no interrupt reschedule its softirq */
	irq_disable();

	softirq_schedule = 0;
	for (int i = 0; i < NR_SOFTIRQ; ++i) {
		softirq_schedule |= softirq[i].schedule;
	}

	set_kernel_state((softirq_schedule) ? T_RUNNABLE : T_INACTIVE);
	irq_enable();

	if (softirq_schedule)
		goto retry;

	return executed;
}

#ifdef CONFIG_KDB
void kdb_dump_softirq(void)
{
	for (int i = 0; i < NR_SOFTIRQ; ++i) {
		dbg_printf(DL_KDB, "%32s %s\n", softirq_names[i],
		           atomic_get(&(softirq[i].schedule)) ?
		           "scheduled" : "not scheduled");
	}
}
#endif
