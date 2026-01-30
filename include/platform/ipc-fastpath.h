/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_IPC_FASTPATH_H_
#define PLATFORM_IPC_FASTPATH_H_

#include <debug.h>
#include <ipc.h>
#include <platform/armv7m.h>
#include <platform/irq.h>
#include <sched.h>
#include <syscall.h>
#include <thread.h>
#include <types.h>

/**
 * Fastpath IPC optimization for ARM Cortex-M.
 *
 * Bypasses softirq scheduling for simple short-message IPC by performing
 * direct register transfer in the SVC handler context.
 *
 * CRITICAL: Message registers (MR0-MR7) are in R4-R11, NOT R0-R7.
 * - R0-R3: Syscall parameters (to_tid, from_tid, timeout, unused)
 * - R4-R11: Message registers (MR0-MR7) via ctx.regs[0-7]
 *
 * This implementation uses a NAKED wrapper to capture R4-R11 immediately
 * before any compiler-generated prologue can clobber them.
 *
 * Implementation is in header as static inline for zero call overhead.
 */

/**
 * ipc_fastpath_copy_mrs() - Copy message registers to receiver
 * @saved_mrs: Saved message registers R4-R11 (MR0-MR7)
 * @sender: Source thread (for msg_buffer access)
 * @receiver: Destination thread
 * @n_untyped: Number of untyped words to copy (0-39)
 *
 * Copies MR0-MR{n_untyped} from sender to receiver:
 * - MR0-MR7:   From saved_mrs to receiver->ctx.regs[0-7]
 * - MR8-MR39:  From sender->msg_buffer to receiver->msg_buffer (NEW)
 *
 * WCET: ~20 cycles (MR0-MR7) + ~100 cycles (MR8-MR39, if used)
 */
static inline void ipc_fastpath_copy_mrs(volatile uint32_t *saved_mrs,
                                         struct tcb *sender,
                                         struct tcb *receiver,
                                         int n_untyped)
{
    int count = n_untyped + 1; /* +1 for tag in MR0 */
    int i;

    /* Phase 1: Copy MR0-MR7 from saved registers (R4-R11) */
    for (i = 0; i < count && i < 8; i++)
        receiver->ctx.regs[i] = saved_mrs[i];

    /* Phase 2: Copy MR8-MR39 from sender's msg_buffer (if needed) */
    if (count > 8) {
        int buf_count = count - 8; /* Number of words in buffer */
        if (buf_count > 32)
            buf_count = 32; /* Clamp to buffer size */

        for (i = 0; i < buf_count; i++)
            receiver->msg_buffer[i] = sender->msg_buffer[i];
    }
}

/**
 * ipc_fastpath_helper() - Fastpath IPC implementation (C helper)
 * @caller: Current thread attempting IPC
 * @svc_param: SVC stack frame (R0-R3, R12, LR, PC, xPSR)
 * @saved_mrs: Pre-saved R4-R11 message registers
 *
 * Called by the naked wrapper after message registers have been captured.
 * Returns 1 if fastpath succeeded, 0 to fall back to slowpath.
 */
static inline int ipc_fastpath_helper(struct tcb *caller,
                                      uint32_t *svc_param,
                                      volatile uint32_t *saved_mrs)
{
    struct tcb *to_thr;
    l4_thread_t to_tid, from_tid;
    ipc_msg_tag_t tag;

    /* Extract IPC parameters from hardware stack (R0-R3) */
    to_tid = svc_param[REG_R0];
    from_tid = svc_param[REG_R1];

    /* Extract tag from saved MR0 (R4), NOT from R0! */
    tag.raw = saved_mrs[0];

    /* Fastpath Eligibility Check */

    /* Criterion 1: Simple send (to_tid valid, from_tid = NILTHREAD) */
    if (to_tid == L4_NILTHREAD || from_tid != L4_NILTHREAD)
        return 0; /* Slowpath: receive-only or send+receive */

    /* Criterion 2: No typed items (no MapItems/GrantItems) */
    if (tag.s.n_typed != 0)
        return 0; /* Slowpath: requires map_area() processing */

    /* Criterion 3: Short message (fits in MR0-MR39: registers + buffer)
     * MR0-MR7:   8 words × 4 bytes = 32 bytes (registers)
     * MR8-MR39: 32 words × 4 bytes = 128 bytes (buffer)
     * Total capacity: 160 bytes (40 words)
     */
    if (tag.s.n_untyped > 39)
        return 0; /* Slowpath: requires UTCB access */

    /* Criterion 4: Receiver exists and is blocked waiting */
    to_thr = thread_by_globalid(to_tid);
    if (!to_thr || to_thr->state != T_RECV_BLOCKED)
        return 0; /* Slowpath: receiver not ready */

    /* Criterion 5: Receiver is waiting for us */
    if (to_thr->ipc_from != L4_ANYTHREAD &&
        to_thr->ipc_from != caller->t_globalid)
        return 0; /* Slowpath: receiver waiting for someone else */

    /* Criterion 6: Special thread handling */
    if (to_tid == TID_TO_GLOBALID(THREAD_LOG) ||
        to_tid == TID_TO_GLOBALID(THREAD_IRQ_REQUEST))
        return 0; /* Slowpath: special kernel threads */

    /* Criterion 7: Thread start protocol */
    if (tag.raw == 0x00000005)
        return 0; /* Slowpath: thread initialization */

    /* All criteria met - Execute Fastpath */

    /* Phase 0: Dequeue caller (will re-enqueue later) */
    extern void sched_dequeue(struct tcb *);
    sched_dequeue(caller);

    /* Phase 1: Copy message registers from sender to receiver
     * - MR0-MR7:   From saved registers (R4-R11)
     * - MR8-MR39:  From sender's msg_buffer to receiver's msg_buffer (if
     * needed)
     */
    ipc_fastpath_copy_mrs(saved_mrs, caller, to_thr, tag.s.n_untyped);

    /* Phase 2: Update receiver context */
    /* Set R0 to sender ID (IPC protocol) */
    ((uint32_t *) to_thr->ctx.sp)[REG_R0] = caller->t_globalid;
    to_thr->utcb->sender = caller->t_globalid;

    /* Phase 3: Update thread states */

    /* Clear timeout events (no timeout in fastpath) */
    caller->timeout_event = 0;
    to_thr->timeout_event = 0;

    /* Receiver becomes runnable with IPC priority boost */
    to_thr->state = T_RUNNABLE;
    to_thr->ipc_from = L4_NILTHREAD;
    sched_set_priority(to_thr, SCHED_PRIO_IPC);
    sched_enqueue(to_thr);

    /* Caller continues (send-only, no reply expected)
     * Fastpath only handles from_tid==NILTHREAD (simple send).
     * For L4_Call (send+receive), slowpath handles blocking.
     *
     * Re-enqueue caller (was dequeued at SVC entry).
     * It's safe to enqueue current thread - sched has double-enqueue
     * protection.
     */
    caller->state = T_RUNNABLE;
    sched_enqueue(caller);

    /* Phase 4: Request context switch via PendSV */
    /* DON'T do immediate switch - let PendSV handle it normally */
    request_schedule();

    return 1; /* Fastpath succeeded */
}

/**
 * ipc_try_fastpath() - IPC fastpath using pre-saved R4-R11
 * @caller: Current thread attempting IPC
 * @svc_param: SVC stack frame (R0-R3, R12, LR, PC, xPSR)
 *
 * Reads message registers from __irq_saved_regs which were saved by
 * SVC_HANDLER before any C code ran, ensuring MR0-MR7 are untouched.
 *
 * Returns:
 *   1 if fastpath succeeded (caller should skip slowpath)
 *   0 if fastpath unavailable (caller must use slowpath)
 *
 * Eligibility criteria:
 * - Simple send (to_tid valid, from_tid == NILTHREAD)
 * - Short message (n_untyped <= 39, n_typed == 0)
 * - Receiver ready (T_RECV_BLOCKED, waiting for caller or ANYTHREAD)
 */
static inline int ipc_try_fastpath(struct tcb *caller, uint32_t *svc_param)
{
    extern volatile uint32_t __irq_saved_regs[8];

    /* Read from global __irq_saved_regs saved by SVC_HANDLER */
    return ipc_fastpath_helper(caller, svc_param, __irq_saved_regs);
}

#endif /* PLATFORM_IPC_FASTPATH_H_ */
