/* Copyright (c) 2013, 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include INC_PLAT(systick.h)

#include <platform/armv7m.h>
#include <debug.h>
#include <error.h>
#include <types.h>
#include <thread.h>
#include <memory.h>
#include <ipc.h>
#include <sched.h>
#include <user-log.h>
#include <ktimer.h>
#include <interrupt.h>

extern tcb_t *caller;

/* Imports from thread.c */
extern tcb_t *thread_map[];
extern int thread_count;

uint32_t ipc_read_mr(tcb_t *from, int i)
{
	if (i >= 8)
		return from->utcb->mr[i - 8];
	return from->ctx.regs[i];
}

void ipc_write_mr(tcb_t *to, int i, uint32_t data)
{
	if (i >= 8)
		to->utcb->mr[i - 8] = data;
	else
		to->ctx.regs[i] = data;
}

static void user_ipc_error(tcb_t *thr, enum user_error_t error)
{
	ipc_msg_tag_t tag;

	/* Set ipc unsuccessful */
	tag.raw = ipc_read_mr(thr, 0);
	tag.s.flags |= 0x8;
	ipc_write_mr(thr, 0, tag.raw);

	set_user_error(thr, error);
}

static inline void do_ipc_error(tcb_t *from, tcb_t *to,
                                enum user_error_t from_err,
                                enum user_error_t to_err,
                                thread_state_t from_state,
                                thread_state_t to_state)
{
	user_ipc_error(from, from_err);
	user_ipc_error(to, to_err);
	from->state = from_state;
	to->state = to_state;
}


static void do_ipc(tcb_t *from, tcb_t *to)
{
	ipc_msg_tag_t tag;
	ipc_typed_item	typed_item;
	int untyped_idx, typed_idx, typed_item_idx;
	int typed_last, untyped_last;
	uint32_t typed_data;		/* typed item extra word */
	l4_thread_t from_recv_tid;

	/* Clear timeout event when ipc is established. */
	from->timeout_event = 0;
	to->timeout_event = 0;

	/* Copy tag of message */
	tag.raw = ipc_read_mr(from, 0);
	untyped_last = tag.s.n_untyped + 1;
	typed_last = untyped_last + tag.s.n_typed;

	if (typed_last > IPC_MR_COUNT) {
		do_ipc_error(from, to,
		             UE_IPC_MSG_OVERFLOW | UE_IPC_PHASE_SEND,
		             UE_IPC_MSG_OVERFLOW | UE_IPC_PHASE_RECV,
		             T_RUNNABLE,
		             T_RUNNABLE);
		return;
	}

	ipc_write_mr(to, 0, tag.raw);

	/* Copy untyped words */
	for (untyped_idx = 1; untyped_idx < untyped_last; ++untyped_idx) {
		ipc_write_mr(to, untyped_idx, ipc_read_mr(from, untyped_idx));
	}

	typed_item_idx = -1;
	/* Copy typed words
	 * FSM: j - number of byte */
	for (typed_idx = untyped_idx; typed_idx < typed_last; ++typed_idx) {
		uint32_t mr_data = ipc_read_mr(from, typed_idx);

		/* Write typed mr data to 'to' thread */
		ipc_write_mr(to, typed_idx, mr_data);

		if (typed_item_idx == -1) {
			/* If typed_item_idx == -1 - read typed item's tag */
			typed_item.raw = mr_data;
			++typed_item_idx;
		} else if (typed_item.s.header & IPC_TI_MAP_GRANT) {
			/* MapItem / GrantItem have 1xxx in header */
			int ret;
			typed_data = mr_data;

			ret = map_area(from->as, to->as,
			         typed_item.raw & 0xFFFFFFC0,
			         typed_data & 0xFFFFFFC0,
			         (typed_item.s.header & IPC_TI_GRANT) ? GRANT : MAP,
			         thread_ispriviliged(from));
			typed_item_idx = -1;

			if (ret < 0) {
				do_ipc_error(from, to,
				             UE_IPC_ABORTED | UE_IPC_PHASE_SEND,
				             UE_IPC_ABORTED | UE_IPC_PHASE_RECV,
				             T_RUNNABLE,
				             T_RUNNABLE);
				return;
			}
		}

		/* TODO: StringItem support */
	}

	if (!to->ctx.sp || !from->ctx.sp) {
		do_ipc_error(from, to,
		             UE_IPC_ABORTED | UE_IPC_PHASE_SEND,
		             UE_IPC_ABORTED | UE_IPC_PHASE_RECV,
		             T_RUNNABLE,
		             T_RUNNABLE);
		return;
	}

	to->utcb->sender = from->t_globalid;

	to->state = T_RUNNABLE;
	to->ipc_from = L4_NILTHREAD;
	((uint32_t*)to->ctx.sp)[REG_R0] = from->t_globalid;

	/* If from has receive phases, lock myself */
	from_recv_tid = ((uint32_t*)from->ctx.sp)[REG_R1];
	if (from_recv_tid == L4_NILTHREAD) {
		from->state = T_RUNNABLE;
	} else {
		from->state = T_RECV_BLOCKED;
		from->ipc_from = from_recv_tid;

		dbg_printf(DL_IPC, "IPC: %t receiving\n", from->t_globalid);
	}

	/* Dispatch communicating threads */
	sched_slot_dispatch(SSI_NORMAL_THREAD, from);
	sched_slot_dispatch(SSI_IPC_THREAD, to);

	dbg_printf(DL_IPC,
	           "IPC: %t to %t\n", caller->t_globalid, to->t_globalid);
}

uint32_t ipc_timeout(void *data)
{
	ktimer_event_t *event = (ktimer_event_t *) data;
	tcb_t* thr = (tcb_t*) event->data;

	if (thr->timeout_event == (uint32_t)data) {

		if (thr->state == T_RECV_BLOCKED)
			user_ipc_error(thr, UE_IPC_TIMEOUT | UE_IPC_PHASE_RECV);

		if (thr->state == T_SEND_BLOCKED)
			user_ipc_error(thr, UE_IPC_TIMEOUT | UE_IPC_PHASE_SEND);

		thr->state = T_RUNNABLE;
		thr->timeout_event = 0;
	}

	return 0;
}

static void sys_ipc_timeout(uint32_t timeout)
{
	ktimer_event_t *kevent;
	ipc_time_t t = { .raw = timeout };
	uint32_t ticks;

	/* millisec to ticks */
	ticks = (t.period.m << t.period.e) /
	        ((1000000)/(CORE_CLOCK/CONFIG_KTIMER_HEARTBEAT));

	kevent = ktimer_event_create(ticks, ipc_timeout, caller);

	caller->timeout_event = (uint32_t)kevent;
}

void sys_ipc(uint32_t *param1)
{
	/* TODO: Checking of recv-mask */
	tcb_t *to_thr = NULL;
	l4_thread_t to_tid = param1[REG_R0], from_tid = param1[REG_R1];
	uint32_t timeout = param1[REG_R2];

	if (to_tid == L4_NILTHREAD &&
		from_tid == L4_NILTHREAD) {
		caller->state = T_INACTIVE;
		if (timeout)
			sys_ipc_timeout(timeout);
		return;
	}

	if (to_tid != L4_NILTHREAD) {
		to_thr = thread_by_globalid(to_tid);

		if (to_tid == TID_TO_GLOBALID(THREAD_LOG)) {
			user_log(caller);
			caller->state = T_RUNNABLE;
			return;
#ifdef CONFIG_BOARD_STM32F4DISCOVERY
		} else if (to_tid == TID_TO_GLOBALID(THREAD_IRQ_REQUEST)) {
			user_interrupt_config(caller);
			caller->state = T_RUNNABLE;
			return;
#endif
		} else if ((to_thr && to_thr->state == T_RECV_BLOCKED)
		           || to_tid == caller->t_globalid) {
			/* To thread who is waiting for us or sends to myself */
			do_ipc(caller, to_thr);
			return;
		} else if (to_thr && to_thr->state == T_INACTIVE &&
		           GLOBALID_TO_TID(to_thr->utcb->t_pager) == GLOBALID_TO_TID(caller->t_globalid)) {
			if (ipc_read_mr(caller, 0) == 0x00000005) {
				/* mr1: thread func, mr2: stack addr, mr3: stack size*/
				/* mr4: thread entry, mr5: thread args */
				/* thread start protocol */

				memptr_t sp = ipc_read_mr(caller, 2);
				size_t stack_size = ipc_read_mr(caller, 3);
				uint32_t regs[4];	/* r0, r1, r2, r3 */

				dbg_printf(DL_IPC, "IPC: %t thread start\n", to_tid);

				to_thr->stack_base = sp - stack_size;
				to_thr->stack_size = stack_size;

				regs[REG_R0] = (uint32_t)&kip;
				regs[REG_R1] = (uint32_t)to_thr->utcb;
				regs[REG_R2] = ipc_read_mr(caller, 4);
				regs[REG_R3] = ipc_read_mr(caller, 5);
				thread_init_ctx((void *) sp, (void *) ipc_read_mr(caller, 1),
				                regs, to_thr);

				caller->state = T_RUNNABLE;

				/* Start thread */
				to_thr->state = T_RUNNABLE;

				return;
			} else {
				do_ipc(caller, to_thr);
				to_thr->state = T_INACTIVE;

				return;
			}
		} else  {
			/* No waiting, block myself */
			caller->state = T_SEND_BLOCKED;
			caller->utcb->intended_receiver = to_tid;
			dbg_printf(DL_IPC, "IPC: %t sending\n", caller->t_globalid);

			if (timeout)
				sys_ipc_timeout(timeout);

			return;
		}
	}

	if (from_tid != L4_NILTHREAD) {
		tcb_t *thr = NULL;

		if (from_tid == L4_ANYTHREAD) {
			int i;
			/* Find out if there is any sending thread waiting for caller */
			for (i = 1; i < thread_count; ++i) {
				thr = thread_map[i];
				if (thr->state == T_SEND_BLOCKED &&
				    thr->utcb->intended_receiver == caller->t_globalid) {
					do_ipc(thr, caller);
					return;
				}
			}
#ifdef CONFIG_BOARD_STM32F4DISCOVERY
		} else if (from_tid != TID_TO_GLOBALID(THREAD_INTERRUPT)) {
#else
		} else {
#endif
			thr = thread_by_globalid(from_tid);

			if (thr->state == T_SEND_BLOCKED &&
			    thr->utcb->intended_receiver == caller->t_globalid) {
				do_ipc(thr, caller);
				return;
			}
		}

		/* Only receive phases, simply lock myself */
		caller->state = T_RECV_BLOCKED;
		caller->ipc_from = from_tid;

#ifdef CONFIG_BOARD_STM32F4DISCOVERY
		if (from_tid == TID_TO_GLOBALID(THREAD_INTERRUPT)) {
			/* Threaded interrupt is ready */
			user_interrupt_handler_update(caller);
		}
#endif

		if (timeout)
			sys_ipc_timeout(timeout);

		dbg_printf(DL_IPC, "IPC: %t receiving\n", caller->t_globalid);

		return;
	}

	caller->state = T_SEND_BLOCKED;
}

uint32_t ipc_deliver(void *data)
{
	tcb_t *thr = NULL, *from_thr = NULL, *to_thr = NULL;
	l4_thread_t receiver;
	int i;

	for (i = 1; i < thread_count; ++i) {
		thr = thread_map[i];
		switch (thr->state) {
		case T_RECV_BLOCKED:
			if (thr->ipc_from != L4_NILTHREAD &&
				thr->ipc_from != L4_ANYTHREAD &&
				thr->ipc_from != TID_TO_GLOBALID(THREAD_INTERRUPT)) {
				from_thr = thread_by_globalid(thr->ipc_from);
				if (from_thr->state == T_SEND_BLOCKED)
					do_ipc(from_thr, thr);
			}
			break;
		case T_SEND_BLOCKED:
			receiver = thr->utcb->intended_receiver;
			if (receiver != L4_NILTHREAD && receiver != L4_ANYTHREAD) {
				to_thr = thread_by_globalid(receiver);
				if (to_thr->state == T_RECV_BLOCKED)
					do_ipc(thr, to_thr);
			}
			break;
		default:
			break;
		}
	}

	return 4096;
}
