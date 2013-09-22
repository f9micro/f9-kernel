/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/armv7m.h>
#include <debug.h>
#include <error.h>
#include <types.h>
#include <thread.h>
#include <memory.h>
#include <ipc.h>
#include <sched.h>

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

	/* Copy tag of message */
	tag.raw = ipc_read_mr(from, 0);
	untyped_last = tag.s.n_untyped + 1;
	typed_last = untyped_last + tag.s.n_typed;

	if (typed_last > IPC_MR_COUNT) {
		set_user_error(from, UE_IPC_MSG_OVERFLOW | UE_IPC_PHASE_SEND);
		set_user_error(to, UE_IPC_MSG_OVERFLOW | UE_IPC_PHASE_RECV);
		return;
	}

	ipc_write_mr(to, 0, tag.s.label << 16);

	/* Copy untyped words */
	for (untyped_idx = 1; untyped_idx < untyped_last; ++untyped_idx) {
		ipc_write_mr(to, untyped_idx, ipc_read_mr(from, untyped_idx));
	}

	typed_item_idx = -1;
	/* Copy typed words
	 * FSM: j - number of byte */
	for (typed_idx = untyped_idx; typed_idx < typed_last; ++typed_idx) {
		if (typed_item_idx == -1) {
			/* If typed_item_idx == -1 - read ti's tag */
			typed_item.raw = ipc_read_mr(from, typed_idx);
			++typed_item_idx;
		}
		else if (typed_item.s.header & IPC_TI_MAP_GRANT) {
			/* MapItem / GrantItem have 1xxx in header */
			typed_data = ipc_read_mr(from, typed_idx);

			map_area(from->as, to->as,
			         typed_item.raw & 0xFFFFFFC0,
			         typed_data & 0xFFFFFFC0,
			         (typed_item.s.header & IPC_TI_GRANT)? GRANT : MAP,
			         thread_ispriviliged(from));
			typed_item_idx = -1;
		}

		/* TODO: StringItem support */
	}

	to->utcb->sender = from->t_globalid;

	to->state = T_RUNNABLE;
	to->ipc_from = L4_NILTHREAD;
	from->state = T_RUNNABLE;

	/* Dispatch communicating threads */
	sched_slot_dispatch(SSI_NORMAL_THREAD, from);
	sched_slot_dispatch(SSI_IPC_THREAD, to);

	dbg_printf(DL_IPC,
		"IPC: %t to %t\n", caller->t_globalid, to->t_globalid);
}

void sys_ipc(uint32_t *param1)
{
	/* TODO: Checking of recv-mask */
	tcb_t *to_thr = NULL;
	l4_thread_t to_tid = param1[REG_R0], from_tid = param1[REG_R1];

	if (to_tid != L4_NILTHREAD) {
		to_thr = thread_by_globalid(to_tid);

		if ((to_thr && to_thr->state == T_RECV_BLOCKED)
				|| to_tid == caller->t_globalid ) {
			/* To thread who is waiting for us or sends to myself */
			do_ipc(caller, to_thr);
		}
		else if (to_thr && to_thr->state == T_INACTIVE &&
			GLOBALID_TO_TID(to_thr->utcb->t_pager) == GLOBALID_TO_TID(caller->t_globalid)) {
			if (ipc_read_mr(caller, 0) == 0x00000002) {
				/* thread start protocol */

				dbg_printf(DL_IPC, "IPC: %t thread start\n", to_tid);

				thread_init_ctx((void *) ipc_read_mr(caller, 2),
					            (void *) ipc_read_mr(caller, 1), to_thr);
				caller->state = T_RUNNABLE;

				/* Start thread */
				to_thr->state = T_RUNNABLE;
			}
			else {
				do_ipc(caller, to_thr);
				to_thr->state = T_INACTIVE;
			}
		}
		else  {
			/* No waiting, block myself */
			caller->state = T_SEND_BLOCKED;
			caller->utcb->intended_receiver = to_tid;

			dbg_printf(DL_IPC, "IPC: %t sending\n", caller->t_globalid);
		}
	}

	if (from_tid != L4_NILTHREAD) {
		/* Only receive phases, simply lock myself */
		caller->state = T_RECV_BLOCKED;
		caller->ipc_from = from_tid;

		dbg_printf(DL_IPC, "IPC: %t receiving\n", caller->t_globalid);
	}
}

uint32_t ipc_deliver(void *data)
{
	tcb_t *thr = NULL, *from_thr = NULL;
	int i;

	for (i = 1; i < thread_count; ++i) {
		thr = thread_map[i];

		if (thr->state == T_RECV_BLOCKED && thr->ipc_from != L4_NILTHREAD &&
				thr->ipc_from != L4_ANYTHREAD) {
			from_thr = thread_by_globalid(thr->ipc_from);

			do_ipc(from_thr, thr);
		}
	}

	return 4096;
}
