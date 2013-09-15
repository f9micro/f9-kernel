/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SCHED_H_
#define SCHED_H_

#include <thread.h>

typedef enum {
	SSI_SOFTIRQ,			/* Kernel thread */
	SSI_INTR_THREAD,
	SSI_ROOT_THREAD,
	SSI_IPC_THREAD,
	SSI_NORMAL_THREAD,
	SSI_IDLE,

	NUM_SCHED_SLOTS
} sched_slot_id_t;

struct sched_slot;

typedef tcb_t *(*sched_handler_t)(struct sched_slot *slot);

typedef struct sched_slot {
	tcb_t *ss_scheduled;
	sched_handler_t ss_handler;
} sched_slot_t;

void sched_init(void);

tcb_t* schedule_select(void);
int schedule(void);
void sched_slot_dispatch(sched_slot_id_t slot_id, tcb_t *thread);
void sched_slot_set_handler(sched_slot_id_t slot_id, sched_handler_t handler);

#endif /* SCHED_H_ */
