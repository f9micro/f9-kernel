/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <types.h>
#include <lib/ktable.h>
#include <memory.h>
#include <kip.h>

#include <l4/utcb.h>

/**
 * @file thread.h
 * @brief Thread dispatcher definitions
 *
 * Stack Overflow Protection:
 * A canary value is placed at the bottom of each thread's stack (lowest
 * address, since ARM stacks grow downward). The canary is checked on
 * every context switch - if corrupted, the stack has overflowed.
 *
 * Thread ID type is declared in @file types.h and called l4_thread_t
 *
 * For Global Thread ID only high 18 bits are used and lower are reserved,
 * so we call higher meaningful value TID and use GLOBALID_TO_TID and
 * TID_TO_GLOBALID macroses for convertion.
 *
 * Constants:
 *   - L4_NILTHREAD  - nilthread
 *   - L4_ANYLOCALTHREAD - anylocalthread
 *   - L4_ANYTHREAD  - anythread
 */

#define L4_NILTHREAD		0
#define L4_ANYLOCALTHREAD	0xFFFFFFC0
#define L4_ANYTHREAD		0xFFFFFFFF

#define GLOBALID_TO_TID(id)	(id >> 14)
#define TID_TO_GLOBALID(id)	(id << 14)

#define THREAD_BY_TID(id)	 thread_by_globalid(TID_TO_GLOBALID(id))

/* Stack canary for overflow detection.
 * Placed at stack_base (lowest address). Checked on context switch.
 */
#define STACK_CANARY		0xDEADBEEF

typedef enum {
	THREAD_IDLE,
	THREAD_KERNEL,
	THREAD_ROOT,
	THREAD_INTERRUPT,
	THREAD_IRQ_REQUEST,
	THREAD_LOG,
	THREAD_SYS	= 16,				/* Systembase */
	THREAD_USER	= CONFIG_INTR_THREAD_MAX	/* Userbase */
} thread_tag_t;

typedef enum {
	T_INACTIVE,
	T_RUNNABLE,
	T_SVC_BLOCKED,
	T_RECV_BLOCKED,
	T_SEND_BLOCKED
} thread_state_t;

typedef struct {
	uint32_t sp;
	uint32_t ret;
	uint32_t ctl;
	uint32_t regs[8];
#ifdef CONFIG_FPU
	uint64_t fp_regs[8];
	uint32_t fp_flag;
#endif
} context_t;

/**
 * Thread control block
 *
 * TCB is a tree of threads, linked by t_sibling (siblings) and t_parent/t_child
 * Contains pointers to thread's UTCB (User TCB) and address space
 */
struct tcb {
	l4_thread_t t_globalid;
	l4_thread_t t_localid;

	thread_state_t state;

	memptr_t stack_base;
	size_t stack_size;

	context_t ctx;

	as_t *as;
	struct utcb *utcb;

	l4_thread_t ipc_from;

	struct tcb *t_sibling;
	struct tcb *t_parent;
	struct tcb *t_child;

	uint32_t timeout_event;
};
typedef struct tcb tcb_t;

/* Initialize stack canary at bottom of stack.
 * Must be called after stack_base is set.
 */
static inline void thread_init_canary(tcb_t *thr)
{
	if (thr->stack_base)
		*((uint32_t *) thr->stack_base) = STACK_CANARY;
}

/* Check stack canary. Returns 1 if valid, 0 if corrupted. */
static inline int thread_check_canary(tcb_t *thr)
{
	if (!thr->stack_base)
		return 1;  /* No stack tracking, skip check */
	return *((uint32_t *) thr->stack_base) == STACK_CANARY;
}

void thread_init_subsys(void);

tcb_t *thread_by_globalid(l4_thread_t globalid);

tcb_t *thread_init(l4_thread_t globalid, utcb_t *utcb);
tcb_t *thread_create(l4_thread_t globalid, utcb_t *utcb);
void thread_destroy(tcb_t *thr);
void thread_space(tcb_t *thr, l4_thread_t spaceid, utcb_t *utcb);
void thread_init_ctx(void *sp, void *pc, void *rx, tcb_t *thr);
void thread_init_kernel_ctx(void *sp, tcb_t *thr);
void thread_switch(tcb_t *thr);

int thread_ispriviliged(tcb_t *thr);
int thread_isrunnable(tcb_t *thr);
tcb_t *thread_current(void);

int schedule(void);

#endif /* THREAD_H_ */
