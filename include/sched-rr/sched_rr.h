/* Copyright (c) 2016 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SCHED_RR_H_
#define SCHED_RR_H_

#define MAX_QUEUE (CONFIG_MAX_THREADS + 1)
#define TIME_SLICE CONFIG_THREAD_TIME_SLICE

typedef struct cqueue {
	uint32_t Queue[MAX_QUEUE];
	int front;
	int rear;
	int count;

	void (*put)(struct cqueue *q, void *data);
	void *(*get)(struct cqueue *q);
	int (*is_empty)(struct cqueue *q);
	int (*is_full)(struct cqueue *q);
	int (*length)(struct cqueue *q);
} cqueue_t;


tcb_t *rr_select(void);
void rr_init_thread(tcb_t *thr);
uint32_t rr_exhaust_timeslice(void *data);

#endif
