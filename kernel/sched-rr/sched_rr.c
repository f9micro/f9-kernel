/* Copyright (c) 2016 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <types.h>
#include <thread.h>
#include <ktimer.h>
#include <sched-rr/sched_rr.h>

void put(cqueue_t *q, void *data)
{
	if (!q->is_full(q)) {
		q->count++;
		q->rear = (q->rear + 1) % (MAX_QUEUE);
		q->Queue[q->rear] = (uint32_t) data;
	}
}

void *get(cqueue_t *q)
{
	uint32_t ret;

	if (!q->is_empty(q)) {
		q->count--;
		ret = q->Queue[++q->front % MAX_QUEUE];
		if (q->front % MAX_QUEUE == 0) {
			q->front = 0;
		}

		return (void *) ret;
	}

	return NULL;
}

int is_empty(cqueue_t *q)
{
	return q->count == 0;
}

int is_full(cqueue_t *q)
{
	return q->count == MAX_QUEUE - 1;
}

int length(cqueue_t *q)
{
	return q->count;
}

cqueue_t schedQueue = {
	.Queue = { 0 },
	.front = -1,
	.rear = -1,
	.count = 0,
	.put = put,
	.get = get,
	.is_empty = is_empty,
	.is_full = is_full,
	.length = length,
};

void rr_init_thread(tcb_t *thr)
{
	thr->timeslice_event = rr_exhaust_timeslice;
	thr->state = T_WAIT_SCHEDULE;
	schedQueue.put(&schedQueue, (void *) thr);
}

uint32_t rr_exhaust_timeslice(void *data)
{
	/* Get thread in data */
	tcb_t *thr = (tcb_t *) ((ktimer_event_t *)data)->data;

	/* Check thread state to schedule */
	if (thr->state == T_RUNNABLE) {
		thr->state = T_WAIT_SCHEDULE;
		schedQueue.put(&schedQueue, (void *) thr);
	}

	return 0;
}

tcb_t *rr_select(void)
{
	tcb_t *thr = NULL;

	while (!schedQueue.is_empty(&schedQueue)) {
		thr = (tcb_t *) schedQueue.get(&schedQueue);

		if (thr->state == T_WAIT_SCHEDULE) {
			thr->state = T_PENDING_SCHED;
			schedQueue.put(&schedQueue, (void *) thr);
		} else if (thr->state == T_PENDING_SCHED) {
			thr->state = T_RUNNABLE;
			ktimer_event_create(TIME_SLICE, thr->timeslice_event, thr);
			return thr;
		}
	}

	return NULL;
}
