/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SCHED_H_
#define SCHED_H_

#include <types.h>

/* Forward declaration - full definition in thread.h */
struct tcb;

/**
 * @file sched.h
 * @brief Priority Bitmap Scheduler
 *
 * 32-level priority scheduler with O(1) highest-priority selection.
 * Uses Cortex-M CLZ instruction for efficient bitmap scanning.
 *
 * Priority 0 is highest, priority 31 is lowest (idle).
 * Multiple threads at same priority use round-robin scheduling.
 */

/* Number of priority levels (0 = highest, 31 = lowest) */
#define SCHED_PRIORITY_LEVELS 32

/* Priority assignments for system threads */
#define SCHED_PRIO_SOFTIRQ 0     /* Kernel softirq thread */
#define SCHED_PRIO_INTR 1        /* Interrupt handler threads */
#define SCHED_PRIO_ROOT 2        /* Root thread */
#define SCHED_PRIO_IPC 3         /* IPC fast path */
#define SCHED_PRIO_NORMAL_MIN 4  /* Normal threads start here */
#define SCHED_PRIO_NORMAL_MAX 30 /* Normal threads end here */
#define SCHED_PRIO_IDLE 31       /* Idle thread (always lowest) */

/* Default priority for user threads */
#define SCHED_PRIO_DEFAULT 16

/**
 * Linked list node for ready queue.
 * Embedded in TCB for zero-allocation enqueueing.
 */
typedef struct sched_link {
    struct sched_link *prev, *next;
} sched_link_t;

/* Scheduler initialization */
void sched_init(void);

/* Core scheduling functions */
struct tcb *schedule_select(void);
int schedule(void);

/**
 * Enqueue thread to ready queue at its priority level.
 * Thread must have valid priority set.
 */
void sched_enqueue(struct tcb *thread);

/**
 * Dequeue thread from ready queue.
 * Called when thread blocks or is destroyed.
 */
void sched_dequeue(struct tcb *thread);

/**
 * Check if thread is currently in a ready queue.
 */
int sched_is_queued(struct tcb *thread);

/**
 * Yield current thread's timeslice.
 * Rotates thread to back of its priority queue for round-robin.
 */
void sched_yield(void);

/**
 * Change thread priority safely.
 * Handles queue migration atomically if thread is queued.
 * Use this instead of directly modifying thread->priority.
 *
 * @param thread Thread to modify
 * @param new_prio New priority level (0 = highest, 31 = lowest)
 */
void sched_set_priority(struct tcb *thread, uint8_t new_prio);

#endif /* SCHED_H_ */
