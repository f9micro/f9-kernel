/* Copyright (c) 2013, 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <thread.h>
#include <lib/ktable.h>
#include <error.h>
#include <debug.h>
#include <sched.h>
#include <platform/irq.h>
#include <platform/armv7m.h>
#include <fpage_impl.h>
#include <init_hook.h>

/**
 * @file    thread.c
 * @brief   Main thread dispatcher
 *
 * Each thread has its own Thread Control Block (struct tcb_t) and
 * addressed by its global id. However, globalid are very wasteful -
 * we reserve 3 global ids for in-kernel purposes (IDLE, KERNEL and
 * ROOT), 240 for NVIC's interrupt thread and 256 for user threads,
 * which gives at least 4  * 512 = 2 KB
 *
 * On the other hand, we don't need so much threads, so we use
 * thread_map - array of pointers to tcb_t sorted by global id and
 * ktable of tcb_t. For each search operation we use binary search.
 *
 * Also dispatcher is responsible for switching contexts (but not
 * scheduling)
 *
 * Reference: A Physically addressed L4 Kernel, Abi Nourai, University of
 * NSW, Sydney (2005)
 */

DECLARE_KTABLE(tcb_t, thread_table, CONFIG_MAX_THREADS);

/* Always sorted, so we can use binary search on it */
tcb_t *thread_map[CONFIG_MAX_THREADS];
int thread_count;

/**
 * current are always points to TCB which was on processor before we had fallen
 * into interrupt handler. irq_save saves sp and r4-r11 (other are saved
 * automatically on stack). When handler finishes, it calls thread_ctx_switch,
 * which chooses next executable thread and returns its context
 *
 * irq_return recover's thread's context
 *
 * See also platform/irq.h
 */

volatile tcb_t *current; 			/* Currently on CPU */
void *current_utcb __USER_DATA;

/* KIP declarations */
static fpage_t *kip_fpage, *kip_extra_fpage;
extern kip_t kip;
extern char *kip_extra;

static tcb_t *thread_sched(sched_slot_t *);

void thread_init_subsys()
{
	fpage_t *last = NULL;

	ktable_init(&thread_table);

	kip.thread_info.s.system_base = THREAD_SYS;
	kip.thread_info.s.user_base = THREAD_USER;

	/* Create KIP fpages
	 * last is ignored, because kip fpages is aligned
	 */
	assign_fpages_ext(-1, NULL,
	                  (memptr_t) &kip, sizeof(kip_t),
	                  &kip_fpage, &last);
	assign_fpages_ext(-1, NULL,
	                  (memptr_t) kip_extra, CONFIG_KIP_EXTRA_SIZE,
	                  &kip_extra_fpage, &last);

	sched_slot_set_handler(SSI_NORMAL_THREAD, thread_sched);
}

INIT_HOOK(thread_init_subsys, INIT_LEVEL_KERNEL);

extern tcb_t *caller;

/*
 * Return upper_bound using binary search
 */
static int thread_map_search(l4_thread_t globalid, int from, int to)
{
	int tid = GLOBALID_TO_TID(globalid);

	/* Upper bound if beginning of array */
	if (to == from || GLOBALID_TO_TID(thread_map[from]->t_globalid) >= tid)
		return from;

	while (from <= to) {
		if ((to - from) <= 1)
			return to;

		int mid = from + (to - from) / 2;

		if (GLOBALID_TO_TID(thread_map[mid]->t_globalid) > tid)
			to = mid;
		else if (GLOBALID_TO_TID(thread_map[mid]->t_globalid) < tid)
			from = mid;
		else
			return mid;
	}

	/* not reached */
	return -1;
}

/*
 * Insert thread into thread map
 */
static void thread_map_insert(l4_thread_t globalid, tcb_t *thr)
{
	if (thread_count == 0) {
		thread_map[thread_count++] = thr;
	} else {
		int i = thread_map_search(globalid, 0, thread_count);
		int j = thread_count;

		/* Move forward
		 * Don't check if count is out of range,
		 * because we will fail on ktable_alloc
		 */

		for (; j > i; --j)
			thread_map[j] = thread_map[j - 1];

		thread_map[i] = thr;
		++thread_count;
	}
}

static void thread_map_delete(l4_thread_t globalid)
{
	if (thread_count == 1) {
		thread_count = 0;
	} else {
		int i = thread_map_search(globalid, 0, thread_count);
		--thread_count;
		for (; i < thread_count; i++)
			thread_map[i] = thread_map[i + 1];
	}
}

/*
 * Initialize thread
 */
tcb_t *thread_init(l4_thread_t globalid, utcb_t *utcb)
{
	tcb_t *thr = (tcb_t *) ktable_alloc(&thread_table);

	if (!thr) {
		set_caller_error(UE_OUT_OF_MEM);
		return NULL;
	}

	thread_map_insert(globalid, thr);
	thr->t_localid = 0x0;

	thr->t_child = NULL;
	thr->t_parent = NULL;
	thr->t_sibling = NULL;

	thr->t_globalid = globalid;
	if (utcb)
		utcb->t_globalid = globalid;

	thr->as = NULL;
	thr->utcb = utcb;
	thr->state = T_INACTIVE;

	thr->timeout_event = 0;

	dbg_printf(DL_THREAD, "T: New thread: %t @[%p] \n", globalid, thr);

	return thr;
}

void thread_deinit(tcb_t *thr)
{
	thread_map_delete(thr->t_globalid);
	ktable_free(&thread_table, (void *) thr);
}

/* Called from user thread */
tcb_t *thread_create(l4_thread_t globalid, utcb_t *utcb)
{
	int id = GLOBALID_TO_TID(globalid);

	assert(caller != NULL);

	if (id < THREAD_SYS ||
	    globalid == L4_ANYTHREAD ||
	    globalid == L4_ANYLOCALTHREAD) {
		set_caller_error(UE_TC_NOT_AVAILABLE);
		return NULL;
	}

	tcb_t *thr = thread_init(globalid, utcb);
	thr->t_parent = caller;

	/* Place under */
	if (caller->t_child) {
		tcb_t *t = caller->t_child;

		while (t->t_sibling != 0)
			t = t->t_sibling;
		t->t_sibling = thr;

		thr->t_localid = t->t_localid + (1 << 6);
	} else {
		/* That is first thread in child chain */
		caller->t_child = thr;

		thr->t_localid = (1 << 6);
	}

	return thr;
}

void thread_destroy(tcb_t *thr)
{
	tcb_t *parent, *child, *prev_child;

	/* remove thr from its parent and its siblings */
	parent = thr->t_parent;

	if (parent->t_child == thr) {
		parent->t_child = thr->t_sibling;
	} else {
		child = parent->t_child;
		while (child != thr) {
			prev_child = child;
			child = child->t_sibling;
		}
		prev_child->t_sibling = child->t_sibling;
	}

	/* move thr's children to caller */
	child = thr->t_child;

	if (child) {
		child->t_parent = caller;

		while (child->t_sibling) {
			child = child->t_sibling;
			child->t_parent = caller;
		}

		/* connect thr's children to caller's children */
		child->t_sibling = caller->t_child;
		caller->t_child = thr->t_child;
	}

	thread_deinit(thr);
}

void thread_space(tcb_t *thr, l4_thread_t spaceid, utcb_t *utcb)
{
	/* If spaceid == dest than create new address space
	 * else share address space between threads
	 */
	if (GLOBALID_TO_TID(thr->t_globalid) == GLOBALID_TO_TID(spaceid)) {
		thr->as = as_create(thr->t_globalid);

		/* Grant kip_fpage & kip_ext_fpage only to new AS */
		map_fpage(NULL, thr->as, kip_fpage, GRANT);
		map_fpage(NULL, thr->as, kip_extra_fpage, GRANT);

		dbg_printf(DL_THREAD,
		           "\tNew space: as: %p, utcb: %p \n", thr->as, utcb);
	} else {
		tcb_t *space = thread_by_globalid(spaceid);

		thr->as = space->as;
		++(space->as->shared);
	}

	/* If no caller, than it is mapping from kernel to root thread
	 * (some special case for root_utcb)
	 */
	if (caller)
		map_area(caller->as, thr->as, (memptr_t) utcb,
		         sizeof(utcb_t), GRANT, thread_ispriviliged(caller));
	else
		map_area(thr->as, thr->as, (memptr_t) utcb,
		         sizeof(utcb_t), GRANT, 1);
}

void thread_free_space(tcb_t *thr)
{
	/* free address space */
	as_destroy(thr->as);
}

void thread_init_ctx(void *sp, void *pc, void *regs, tcb_t *thr)
{
	/* Reserve 8 words for fake context */
	sp -= RESERVED_STACK;
	thr->ctx.sp = (uint32_t) sp;

	/* Set EXC_RETURN and CONTROL for thread and create initial stack for it
	 * When thread is dispatched, on first context switch
	 */
	if (GLOBALID_TO_TID(thr->t_globalid) >= THREAD_ROOT) {
		thr->ctx.ret = 0xFFFFFFFD;
		thr->ctx.ctl = 0x3;
	} else {
		thr->ctx.ret = 0xFFFFFFF9;
		thr->ctx.ctl = 0x0;
	}

	if (regs == NULL) {
		((uint32_t *) sp)[REG_R0] = 0x0;
		((uint32_t *) sp)[REG_R1] = 0x0;
		((uint32_t *) sp)[REG_R2] = 0x0;
		((uint32_t *) sp)[REG_R3] = 0x0;
	} else {
		((uint32_t *) sp)[REG_R0] = ((uint32_t *) regs)[0];
		((uint32_t *) sp)[REG_R1] = ((uint32_t *) regs)[1];
		((uint32_t *) sp)[REG_R2] = ((uint32_t *) regs)[2];
		((uint32_t *) sp)[REG_R3] = ((uint32_t *) regs)[3];
	}

	((uint32_t *) sp)[REG_R12] = 0x0;
	((uint32_t *) sp)[REG_LR] = 0xFFFFFFFF;
	((uint32_t *) sp)[REG_PC] = (uint32_t) pc;
	((uint32_t *) sp)[REG_xPSR] = 0x1000000; /* Thumb bit on */

}

/* Kernel has no fake context, instead of that we rewind
 * stack and reuse it for kernel thread
 *
 * Stack will be created after first interrupt
 */
void thread_init_kernel_ctx(void *sp, tcb_t *thr)
{
	sp -= RESERVED_STACK;

	thr->ctx.sp = (uint32_t) sp;
	thr->ctx.ret = 0xFFFFFFF9;
	thr->ctx.ctl = 0x0;
}

/*
 * Search thread by its global id
 */
tcb_t *thread_by_globalid(l4_thread_t globalid)
{
	int idx = thread_map_search(globalid, 0, thread_count);

	if (GLOBALID_TO_TID(thread_map[idx]->t_globalid)
	    != GLOBALID_TO_TID(globalid))
		return NULL;
	return thread_map[idx];
}

int thread_isrunnable(tcb_t *thr)
{
	return thr->state == T_RUNNABLE;
}

tcb_t *thread_current()
{
	return (tcb_t *) current;
}

int thread_ispriviliged(tcb_t *thread)
{
	return GLOBALID_TO_TID(thread->t_globalid) == THREAD_ROOT;
}

/* Switch context */
void thread_switch(tcb_t *thr)
{
	assert(thr != NULL);
	assert(thread_isrunnable(thr));

	current = thr;
	current_utcb = thr->utcb;
	if (current->as)
		as_setup_mpu(current->as, current->ctx.sp,
		             ((uint32_t *) current->ctx.sp)[REG_PC],
		             current->stack_base, current->stack_size);
}

/* Select normal thread to run
 *
 * NOTE: all threads are derived from root
 */
static tcb_t *thread_select(tcb_t *parent)
{
	tcb_t *thr = parent->t_child;
	if (thr == NULL)
		return NULL;

	while (1) {
		if (thread_isrunnable(thr))
			return thr;

		if (thr->t_child != NULL) {
			thr = thr->t_child;
			continue;
		}

		if (thr->t_sibling != NULL) {
			thr = thr->t_sibling;
			continue;
		}

		do {
			if (thr->t_parent == parent)
				return NULL;
			thr = thr->t_parent;
		} while (thr->t_sibling == NULL);

		thr = thr->t_sibling;
	}
}

static tcb_t *thread_sched(sched_slot_t *slot)
{
	extern tcb_t *root;

	return thread_select(root);
}

#ifdef CONFIG_KDB

static char *kdb_get_thread_type(tcb_t *thr)
{
	int id = GLOBALID_TO_TID(thr->t_globalid);

	if (id == THREAD_KERNEL)
		return "KERN";
	else if (id == THREAD_ROOT)
		return "ROOT";
	else if (id == THREAD_IDLE)
		return "IDLE";
	else if (id >= THREAD_USER)
		return "[USR]";
	else if (id >= THREAD_SYS)
		return "[SYS]";

	return "???";
}

void kdb_dump_threads(void)
{
	tcb_t *thr;
	int idx;

	char *state[] = { "FREE", "RUN", "SVC", "RECV", "SEND" };

	dbg_printf(DL_KDB, "%5s %8s %8s %6s %s\n",
	           "type", "global", "local", "state", "parent");

	for_each_in_ktable(thr, idx, (&thread_table)) {
		dbg_printf(DL_KDB, "%5s %t %t %6s %t\n",
		           kdb_get_thread_type(thr),
		           thr->t_globalid, thr->t_localid, state[thr->state],
		           (thr->t_parent) ? thr->t_parent->t_globalid : 0);
	}
}

#endif	/* CONFIG_KDB */
