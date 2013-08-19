/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/irq.h>
#include <softirq.h>
#include <thread.h>
#include <ktimer.h>

/*
 * @file systhread.c
 * @brief Three main system threads: kernel, root and idle
 * */

tcb_t *idle;
tcb_t *kernel;
tcb_t *root;

extern void root_thread(void);
utcb_t root_utcb __KIP;

static void kernel_thread(void);
static void idle_thread(void);

void create_root_thread(void)
{
	root = thread_init(TID_TO_GLOBALID(THREAD_ROOT), &root_utcb);
	thread_space(root, TID_TO_GLOBALID(THREAD_ROOT), &root_utcb);
	as_map_user(root->as);

	thread_init_ctx((void *) &root_stack_end, root_thread, root);

	sched_slot_dispatch(SSI_ROOT_THREAD, root);
	root->state = T_RUNNABLE;
}

void create_kernel_thread(void)
{
	kernel = thread_init(TID_TO_GLOBALID(THREAD_KERNEL), NULL);

	thread_init_kernel_ctx(&kernel_stack_end, kernel);

	/* This will prevent running other threads
	 * than kernel until it will be initialized
	 */
	sched_slot_dispatch(SSI_SOFTIRQ, kernel);
	kernel->state = T_RUNNABLE;
}

void create_idle_thread(void)
{
	idle = thread_init(TID_TO_GLOBALID(THREAD_IDLE), NULL);
	thread_init_ctx((void *) &idle_stack_end, idle_thread, idle);

	sched_slot_dispatch(SSI_IDLE, idle);
	idle->state = T_RUNNABLE;
}

void switch_to_kernel(void) __NAKED;
void switch_to_kernel(void)
{
	create_kernel_thread();

	current = kernel;
	init_ctx_switch(&kernel->ctx, kernel_thread);
}

void set_kernel_state(thread_state_t state)
{
	kernel->state = state;
}

static void kernel_thread(void)
{
	while (1) {
		/* If all softirqs processed, call SVC to
		 * switch context immediately
		 */
		softirq_execute();
		irq_svc();
	}
}

static void idle_thread(void)
{
	while (1)
#ifdef CONFIG_KTIMER_TICKLESS
		ktimer_enter_tickless();
#else
		wait_for_interrupt();
#endif /* CONFIG_KTIMER_TICKLESS */
}
