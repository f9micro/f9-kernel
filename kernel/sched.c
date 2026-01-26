/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <sched.h>
#include <thread.h>
#include <error.h>
#include <init_hook.h>
#include <platform/irq.h>
#include <debug.h>

/**
 * @file sched.c
 * @brief Priority Bitmap Scheduler (Optimized)
 *
 * 32-level priority scheduler with O(1) highest-priority selection.
 * Uses Cortex-M CLZ instruction for efficient bitmap scanning.
 *
 * Performance optimizations:
 *   - Strict queue invariant: only runnable threads in ready queues
 *   - Conditional bitmap updates: only write when queue state changes
 *   - Branch-free CLZ: __builtin_clz returns 32 for 0, no check needed
 *   - Zero-offset sched_link: first field in TCB for fast pointer math
 *   - Conditional rotation: only rotate on timeslice expiry (yield)
 *
 * Data structures:
 *   - ready_bitmap: One bit per priority level (bit 31 = prio 0, bit 0 = prio 31)
 *   - ready_queue[]: Circular doubly-linked list per priority level
 */

/* Priority bitmap: bit set means queue has runnable threads */
static uint32_t ready_bitmap;

/* Ready queue heads for each priority level (circular doubly-linked) */
static tcb_t *ready_queue[SCHED_PRIORITY_LEVELS];

/**
 * Count leading zeros using Cortex-M CLZ instruction.
 * Returns 32 if input is 0.
 *
 * Note: __builtin_clz has undefined behavior for 0, but ARM CLZ
 * instruction correctly returns 32. Use inline asm for correctness.
 */
static inline uint32_t clz32(uint32_t x)
{
	uint32_t result;
	__asm__ __volatile__ ("clz %0, %1" : "=r"(result) : "r"(x));
	return result;
}

/**
 * Initialize scheduler.
 */
void sched_init(void)
{
	ready_bitmap = 0;

	for (int i = 0; i < SCHED_PRIORITY_LEVELS; ++i)
		ready_queue[i] = NULL;
}

INIT_HOOK(sched_init, INIT_LEVEL_KERNEL_EARLY);

/**
 * Check if thread is in a ready queue.
 * NULL prev pointer means not queued.
 */
int sched_is_queued(tcb_t *thread)
{
	return thread->sched_link.prev != (void *)0;
}

/**
 * Initialize scheduler link to "not queued" state.
 * NULL pointers indicate thread is not in any queue.
 */
static inline void sched_link_init(tcb_t *thread)
{
	thread->sched_link.prev = NULL;
	thread->sched_link.next = NULL;
}

/**
 * Enqueue thread to ready queue at its priority level.
 * Adds to tail for FIFO ordering within priority.
 *
 * Optimization: Only updates bitmap when queue was empty.
 * IRQ-safe: protects critical section from interrupt corruption.
 */
void sched_enqueue(tcb_t *thread)
{
	uint8_t prio;
	tcb_t *head;
	uint32_t flags;

	if (!thread)
		return;

	flags = irq_save_flags();

	/* Strict invariant: only runnable threads in ready queues
	 * Check inside critical section to prevent race with state changes
	 */
	if (thread->state != T_RUNNABLE) {
		irq_restore_flags(flags);
		panic("SCHED: Enqueueing non-runnable thread %t (state %d)\n",
		      thread->t_globalid, thread->state);
	}

	/* Don't double-enqueue */
	if (sched_is_queued(thread)) {
		irq_restore_flags(flags);
		return;
	}

	prio = thread->priority;
	if (prio >= SCHED_PRIORITY_LEVELS)
		prio = SCHED_PRIO_IDLE;

	head = ready_queue[prio];

	if (!head) {
		/* Empty queue: thread becomes sole element */
		ready_queue[prio] = thread;
		thread->sched_link.prev = thread;
		thread->sched_link.next = thread;

		/* Optimization: Only set bitmap when queue was empty */
		ready_bitmap |= (1UL << (31 - prio));
	} else {
		/* Insert at tail (before head in circular list) */
		tcb_t *tail = head->sched_link.prev;

		thread->sched_link.next = head;
		thread->sched_link.prev = tail;
		tail->sched_link.next = thread;
		head->sched_link.prev = thread;
		/* Bitmap already set - no update needed */
	}

	irq_restore_flags(flags);
}

/**
 * Dequeue thread from ready queue.
 * Must be called before thread enters blocked state.
 * IRQ-safe: protects critical section from interrupt corruption.
 */
void sched_dequeue(tcb_t *thread)
{
	uint8_t prio;
	tcb_t *prev, *next;
	uint32_t flags;

	if (!thread)
		return;

	flags = irq_save_flags();

	/* Not in queue */
	if (!sched_is_queued(thread)) {
		irq_restore_flags(flags);
		return;
	}

	prio = thread->priority;
	if (prio >= SCHED_PRIORITY_LEVELS)
		prio = SCHED_PRIO_IDLE;

	prev = thread->sched_link.prev;
	next = thread->sched_link.next;

	if (thread == next) {
		/* Only element in queue */
		ready_queue[prio] = NULL;
		ready_bitmap &= ~(1UL << (31 - prio));
	} else {
		/* Unlink from list */
		prev->sched_link.next = next;
		next->sched_link.prev = prev;

		/* Update head if we removed it */
		if (ready_queue[prio] == thread)
			ready_queue[prio] = next;
		/* Bitmap stays set - queue still has threads */
	}

	/* Mark as not queued */
	sched_link_init(thread);

	irq_restore_flags(flags);
}

/**
 * Yield current thread's timeslice.
 * Rotates thread to back of its priority queue for round-robin.
 * IRQ-safe: protects critical section from interrupt corruption.
 */
void sched_yield(void)
{
	tcb_t *curr = thread_current();
	uint8_t prio;
	tcb_t *head;
	uint32_t flags;

	if (!curr)
		return;

	flags = irq_save_flags();

	if (!sched_is_queued(curr)) {
		irq_restore_flags(flags);
		return;
	}

	prio = curr->priority;
	if (prio >= SCHED_PRIORITY_LEVELS)
		prio = SCHED_PRIO_IDLE;

	head = ready_queue[prio];

	/* Only rotate if more than one thread at this priority */
	if (head && head->sched_link.next != head) {
		/* Invariant: running thread must be queue head for fair rotation.
		 * If curr != head, priority was changed while running - still safe
		 * because we rotate whatever is at head, maintaining FIFO order.
		 */
		if (head != curr) {
			/* Current thread not at head - it was re-prioritized.
			 * Still rotate head for other waiters at this priority.
			 */
			dbg_printf(DL_SCHEDULE,
				   "SCHED: yield curr %t != head %t at prio %d\n",
				   curr->t_globalid, head->t_globalid, prio);
		}
		ready_queue[prio] = head->sched_link.next;
	}

	irq_restore_flags(flags);
}

/**
 * Select next thread to run.
 *
 * O(1) selection: CLZ gives highest priority, return queue head.
 * Strict invariant: all queued threads are runnable.
 *
 * IRQ-safe: protects bitmap/queue read from concurrent modifications
 * by SysTick or other interrupt handlers that may call sched_enqueue().
 */
tcb_t *schedule_select(void)
{
	uint32_t prio;
	tcb_t *thread;
	uint32_t flags;

	flags = irq_save_flags();

	/* CLZ returns 32 if bitmap is 0 (no branches needed) */
	prio = clz32(ready_bitmap);

	if (prio < SCHED_PRIORITY_LEVELS) {
		thread = ready_queue[prio];

		/* Safety check for consistency */
		if (!thread) {
			irq_restore_flags(flags);
			panic("SCHED: Inconsistent bitmap/queue at prio %d\n", prio);
		}

		irq_restore_flags(flags);
		/* Strict invariant: queued threads are always runnable */
		return thread;
	}

	irq_restore_flags(flags);
	/* Not reached: idle thread should always be runnable */
	panic("Reached end of schedule_select()\n");
	return NULL;
}

/**
 * Change thread priority safely.
 * Handles queue migration atomically if thread is queued.
 * IRQ-safe: protects critical section from interrupt corruption.
 *
 * @param thread Thread to modify
 * @param new_prio New priority level (0 = highest, 31 = lowest)
 */
void sched_set_priority(tcb_t *thread, uint8_t new_prio)
{
	uint32_t flags;
	int was_queued;

	if (!thread)
		return;

	if (new_prio >= SCHED_PRIORITY_LEVELS)
		new_prio = SCHED_PRIO_IDLE;

	flags = irq_save_flags();

	/* Check if priority actually changes */
	if (thread->priority == new_prio) {
		irq_restore_flags(flags);
		return;
	}

	/* Remove from old queue if queued */
	was_queued = sched_is_queued(thread);
	if (was_queued)
		sched_dequeue(thread);

	/* Update priority */
	thread->priority = new_prio;

	/* Re-add to new queue if was queued */
	if (was_queued)
		sched_enqueue(thread);

	irq_restore_flags(flags);
}

/**
 * Main scheduler entry point.
 * Selects next thread and switches to it.
 */
int schedule(void)
{
	tcb_t *scheduled = schedule_select();
	thread_switch(scheduled);
	return 1;
}
