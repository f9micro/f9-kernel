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
#include <user-gpioer.h>
#include <ktimer.h>

extern tcb_t *caller;

/* Imports from thread.c */
extern tcb_t *thread_map[];
extern int thread_count;

static uint32_t ipc_read_mr(tcb_t *from, int i)
{
	if (i >= 8)
		return from->utcb->mr[i - 8];
	return from->ctx.regs[i];
}

static void ipc_write_mr(tcb_t *to, int i, uint32_t data)
{
	if (i >= 8)
		to->utcb->mr[i - 8] = data;
	else
		to->ctx.regs[i] = data;
}

static void do_ipc(tcb_t *from, tcb_t *to)
{
	ipc_msg_tag_t tag;
	ipc_typed_item	typed_item;
	int untyped_idx, typed_idx, typed_item_idx;
	int typed_last, untyped_last;
	uint32_t typed_data;		/* typed item extra word */
	l4_thread_t from_recv_tid;

	/* Copy tag of message */
	tag.raw = ipc_read_mr(from, 0);
	untyped_last = tag.s.n_untyped + 1;
	typed_last = untyped_last + tag.s.n_typed;

	if (typed_last > IPC_MR_COUNT) {
		set_user_error(from, UE_IPC_MSG_OVERFLOW | UE_IPC_PHASE_SEND);
		set_user_error(to, UE_IPC_MSG_OVERFLOW | UE_IPC_PHASE_RECV);
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
			typed_data = mr_data;

			map_area(from->as, to->as,
			         typed_item.raw & 0xFFFFFFC0,
			         typed_data & 0xFFFFFFC0,
			         (typed_item.s.header & IPC_TI_GRANT) ? GRANT : MAP,
			         thread_ispriviliged(from));
			typed_item_idx = -1;
		}

		/* TODO: StringItem support */
	}

	if (!to->ctx.sp || !from->ctx.sp) {
		caller->state = T_RUNNABLE;
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
	tcb_t* from_thr = (tcb_t*) event->data;
	from_thr->state = T_RUNNABLE;
	sched_slot_dispatch(SSI_NORMAL_THREAD, from_thr);
	return 0;
}

void sys_ipc(uint32_t *param1)
{
	/* TODO: Checking of recv-mask */
	tcb_t *to_thr = NULL;
	l4_thread_t to_tid = param1[REG_R0], from_tid = param1[REG_R1];
	uint32_t timeout = param1[REG_R2];

	if (to_tid == L4_NILTHREAD && timeout) { /* Timeout/Sleep */
		ipc_time_t t = { .raw = timeout };
		caller->state = T_INACTIVE;
		ktimer_event_create((t.period.m << t.period.e) /
					((1000000)/(CORE_CLOCK/CONFIG_KTIMER_HEARTBEAT)), /* millisec to ticks */
					ipc_timeout, caller);
		return;
	}

	if (to_tid != L4_NILTHREAD) {
		to_thr = thread_by_globalid(to_tid);

		if (to_tid == TID_TO_GLOBALID(THREAD_LOG)) {
			user_log(caller);
			caller->state = T_RUNNABLE;
			return;
		} else if ((to_thr && to_thr->state == T_RECV_BLOCKED)
		           || to_tid == caller->t_globalid) {
			/* To thread who is waiting for us or sends to myself */
			do_ipc(caller, to_thr);
			return;
		} else if (to_thr && to_thr->state == T_INACTIVE &&
		           GLOBALID_TO_TID(to_thr->utcb->t_pager) == GLOBALID_TO_TID(caller->t_globalid)) {
			if (ipc_read_mr(caller, 0) == 0x00000003) {
				/* thread start protocol */

				memptr_t sp = ipc_read_mr(caller, 2);
				size_t stack_size = ipc_read_mr(caller, 3);

				dbg_printf(DL_IPC, "IPC: %t thread start\n", to_tid);

				to_thr->stack_base = sp - stack_size;
				to_thr->stack_size = stack_size;

				thread_init_ctx((void *) sp, (void *) ipc_read_mr(caller, 1), to_thr);
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

			return;
		}
	}

	if (from_tid != L4_NILTHREAD) {
		/* Only receive phases, simply lock myself */
		caller->state = T_RECV_BLOCKED;
		caller->ipc_from = from_tid;

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
				thr->ipc_from != L4_ANYTHREAD) {
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
