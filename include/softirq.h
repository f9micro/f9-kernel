/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SOFTIRQ_H_
#define SOFTIRQ_H_

#include <types.h>

typedef enum {
	KTE_SOFTIRQ,		/* Kernel timer event */
	ASYNC_SOFTIRQ,		/* Asynchronius event */
	SYSCALL_SOFTIRQ,

#ifdef CONFIG_KDB
	KDB_SOFTIRQ,		/* KDB should have least priority */
#endif

	NR_SOFTIRQ
} softirq_type_t;

typedef void (*softirq_handler_t)(void);

typedef struct {
	uint32_t 		  schedule;
	softirq_handler_t handler;
} softirq_t;

void softirq_register(softirq_type_t type, softirq_handler_t handler);
void softirq_schedule(softirq_type_t type);
int softirq_execute(void);

#endif /* SOFTIRQ_H_ */
