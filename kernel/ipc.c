/* Copyright (c) 2013, 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include INC_PLAT(systick.h)

#include <debug.h>
#include <error.h>
#include <interrupt.h>
#include <ipc.h>
#include <ktimer.h>
#include <memory.h>
#include <notification.h>
#include <platform/armv7m.h>
#include <platform/irq.h>
#include <sched.h>
#include <thread.h>
#include <types.h>
#include <user-log.h>

extern tcb_t *caller;

/* Imports from thread.c */
extern tcb_t *thread_map[];
extern int thread_count;

/**
 * Make thread runnable and enqueue to scheduler.
 * Used after IPC operations that unblock threads.
 */
static inline void thread_make_runnable(tcb_t *thr)
{
    thr->state = T_RUNNABLE;
    sched_enqueue(thr);
}

/* Make sender runnable, restoring base priority.
 * IPC senders should not retain receiver's priority boost.
 */
static inline void thread_make_sender_runnable(tcb_t *thr)
{
    if (thr->priority != thr->base_priority)
        sched_set_priority(thr, thr->base_priority);
    thread_make_runnable(thr);
}

/* Read message register with short buffer support.
 * MR0-MR7:   Hardware registers R4-R11 (ctx.regs[0-7])
 * MR8-MR39:  Short message buffer (msg_buffer[0-31]) - NEW
 * MR40-MR47: UTCB overflow (utcb->mr[0-7])
 */
uint32_t ipc_read_mr(tcb_t *from, int i)
{
    if (i < 8)
        return from->ctx.regs[i];
    if (i < 40)
        return from->msg_buffer[i - 8];
    return from->utcb->mr[i - 40];
}

/* Write message register with short buffer support.
 * MR0-MR7:   Hardware registers R4-R11 (ctx.regs[0-7])
 * MR8-MR39:  Short message buffer (msg_buffer[0-31]) - NEW
 * MR40-MR47: UTCB overflow (utcb->mr[0-7])
 */
void ipc_write_mr(tcb_t *to, int i, uint32_t data)
{
    if (i < 8)
        to->ctx.regs[i] = data;
    else if (i < 40)
        to->msg_buffer[i - 8] = data;
    else
        to->utcb->mr[i - 40] = data;
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

static inline void do_ipc_error(tcb_t *from,
                                tcb_t *to,
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
    ipc_typed_item typed_item;
    int untyped_idx, typed_idx, typed_item_idx;
    uint32_t typed_data; /* typed item extra word */
    l4_thread_t from_recv_tid;

    /* Clear timeout event when ipc is established. */
    from->timeout_event = 0;
    to->timeout_event = 0;

    /* Copy tag of message */
    ipc_msg_tag_t tag = {.raw = ipc_read_mr(from, 0)};
    int untyped_last = tag.s.n_untyped + 1;
    int typed_last = untyped_last + tag.s.n_typed;

    dbg_printf(DL_IPC,
               "IPC: do_ipc tag:%p n_untyped:%d n_typed:%d MR1:%p MR2:%p\n",
               tag.raw, tag.s.n_untyped, tag.s.n_typed, ipc_read_mr(from, 1),
               ipc_read_mr(from, 2));

    if (typed_last > IPC_MR_COUNT) {
        do_ipc_error(from, to, UE_IPC_MSG_OVERFLOW | UE_IPC_PHASE_SEND,
                     UE_IPC_MSG_OVERFLOW | UE_IPC_PHASE_RECV, T_RUNNABLE,
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
            memptr_t map_base, map_size;
            typed_data = mr_data;

            map_base = typed_item.raw & 0xFFFFFFC0;
            map_size = typed_data & 0xFFFFFFC0;

            /* S1 security fix: REJECT unaligned map base addresses.
             * L4 spec uses 64-byte alignment (0xFFFFFFC0 mask), but
             * kernel requires CONFIG_SMALLEST_FPAGE_SIZE (256-byte).
             * Unaligned addresses cannot be safely mapped - kernel
             * must not silently adjust boundaries.
             */
            if (!addr_is_fpage_aligned(map_base)) {
                dbg_printf(DL_IPC, "IPC: REJECT unaligned map base %p\n",
                           map_base);
                do_ipc_error(from, to, UE_IPC_ABORTED | UE_IPC_PHASE_SEND,
                             UE_IPC_ABORTED | UE_IPC_PHASE_RECV, T_RUNNABLE,
                             T_RUNNABLE);
                return;
            }

            dbg_printf(DL_IPC,
                       "IPC: map_area from %t to %t base:%p size:%p priv:%d\n",
                       from->t_globalid, to->t_globalid, map_base, map_size,
                       thread_ispriviliged(from));

            ret = map_area(from->as, to->as, map_base, map_size,
                           (typed_item.s.header & IPC_TI_GRANT) ? GRANT : MAP,
                           thread_ispriviliged(from));
            typed_item_idx = -1;

            dbg_printf(DL_IPC, "IPC: map_area returned %d\n", ret);

            if (ret < 0) {
                do_ipc_error(from, to, UE_IPC_ABORTED | UE_IPC_PHASE_SEND,
                             UE_IPC_ABORTED | UE_IPC_PHASE_RECV, T_RUNNABLE,
                             T_RUNNABLE);
                return;
            }
        }

        /* TODO: StringItem support */
    }

    if (!to->ctx.sp || !from->ctx.sp) {
        do_ipc_error(from, to, UE_IPC_ABORTED | UE_IPC_PHASE_SEND,
                     UE_IPC_ABORTED | UE_IPC_PHASE_RECV, T_RUNNABLE,
                     T_RUNNABLE);
        return;
    }

    to->utcb->sender = from->t_globalid;

    /* Conditionally boost receiver priority for IPC fast path.
     * Only boost if receiver was waiting for ANY message (ipc_from ==
     * ANYTHREAD). If waiting for a specific reply, skip boost - thread was just
     * processing an IPC and will return to user code immediately after
     * receiving reply.
     *
     * This prevents priority inversion where reply receivers accumulate
     * priority 3 and starve lower-priority threads indefinitely.
     */
    /* Write receiver's R0 (sender ID) and UTCB sender BEFORE making runnable.
     * If enqueue happens first and scheduler runs preemptively,
     * receiver could see stale R0 value.
     */
    ((uint32_t *) to->ctx.sp)[REG_R0] = from->t_globalid;
    to->utcb->sender = from->t_globalid;

    /* Check ipc_from BEFORE clearing it for priority boost decision */
    if (to->ipc_from == L4_ANYTHREAD)
        sched_set_priority(to, SCHED_PRIO_IPC);
    to->ipc_from = L4_NILTHREAD;
    thread_make_runnable(to);

    /* If from has receive phases, lock myself */
    from_recv_tid = ((uint32_t *) from->ctx.sp)[REG_R1];
    if (from_recv_tid == L4_NILTHREAD) {
        /* Sender doesn't have receive phase - restore base priority.
         * This prevents IPC priority boost from accumulating across calls.
         */
        thread_make_sender_runnable(from);
    } else {
        /* Sender has receive phase - restore base priority before blocking.
         * When woken up, receiver will be boosted appropriately.
         */
        if (from->priority != from->base_priority)
            sched_set_priority(from, from->base_priority);
        from->state = T_RECV_BLOCKED;
        from->ipc_from = from_recv_tid;

        dbg_printf(DL_IPC, "IPC: %t receiving\n", from->t_globalid);
    }

    dbg_printf(DL_IPC, "IPC: %t→%t done\n", from->t_globalid, to->t_globalid);

    /* Event-chaining: invoke notification callback after IPC delivery.
     * 1. Check callback existence, pending flag, and recursion depth
     * 2. Callback executes with interrupts enabled (nested operations allowed)
     * 3. Invoke callback (can cascade to other notifications)
     * 4. Preemption check after callback returns
     *
     * SAFETY: Callback must be internal kernel handler only.
     * RECURSION: Limited to prevent stack overflow.
     * RE-ENTRANCY: Callback must handle concurrent invocations.
     * CONSTRAINT: Callback MUST NOT destroy its own TCB.
     */
    if (to->ipc_notify && to->notify_pending && to->notify_depth < 3) {
        uint32_t basepri;
        uint8_t generation_before;
        notify_handler_t callback;

        /* Atomically increment depth and capture generation.
         * BASEPRI masking prevents race with nested interrupt-driven IPC.
         * Zero-latency ISRs (0x0-0x2) can still preempt during this operation.
         */
        basepri = irq_kernel_critical_enter();
        to->notify_depth++;
        generation_before = to->notify_generation;
        callback = to->ipc_notify;
        irq_kernel_critical_exit(basepri);

        /* Recursion protection: prevent unbounded callback nesting.
         * Max depth 3 allows: serial → network → timer notification chains.
         * Deeper chains ignored to prevent stack overflow.
         */

        /* Callback executes with interrupts ENABLED to allow
         * nested notifications and prevent priority inversion.
         * TCB LIVENESS: Callback must not destroy its own TCB.
         * If TCB is destroyed, generation counter will change.
         *
         * Pass notify_bits and 0 for notify_data (IPC has no event data).
         */
        uint32_t bits = to->notify_bits;
        callback(to, bits, 0);

        /* Atomically decrement depth only if TCB still valid.
         * Generation counter detects TCB destruction during callback.
         * If TCB was destroyed, skip depth decrement (would be use-after-free).
         *
         * SAFETY: We must verify 'to' is still a valid TCB before accessing it.
         * Search thread_map to confirm the pointer hasn't been freed and
         * reused.
         */
        basepri = irq_kernel_critical_enter();

        /* Verify TCB is still valid by checking thread_map */
        int tcb_valid = 0;
        for (int i = 1; i < thread_count; ++i) {
            if (thread_map[i] == to) {
                tcb_valid = 1;
                break;
            }
        }

        /* Only decrement if TCB is valid AND generation hasn't changed */
        if (tcb_valid && to->notify_generation == generation_before)
            to->notify_depth--;

        irq_kernel_critical_exit(basepri);

        /* Check for preemption after notification.
         * Callback may have made higher-priority threads runnable.
         * schedule() is safe to call even if 'to' was destroyed.
         */
        schedule();
    }
}

uint32_t ipc_timeout(void *data)
{
    ktimer_event_t *event = (ktimer_event_t *) data;
    tcb_t *thr = (tcb_t *) event->data;

    dbg_printf(DL_KDB, "IPC: timeout tid=%t st=%d\n", thr->t_globalid,
               thr->state);

    if (thr->timeout_event == (uint32_t) data) {
        if (thr->state == T_RECV_BLOCKED)
            user_ipc_error(thr, UE_IPC_TIMEOUT | UE_IPC_PHASE_RECV);

        if (thr->state == T_SEND_BLOCKED)
            user_ipc_error(thr, UE_IPC_TIMEOUT | UE_IPC_PHASE_SEND);

        thread_make_runnable(thr);
        thr->timeout_event = 0;
    }

    return 0;
}

static void sys_ipc_timeout(uint32_t timeout)
{
    ktimer_event_t *kevent;
    ipc_time_t t = {.raw = timeout};

    /* millisec to ticks */
    uint32_t ticks = (t.period.m << t.period.e) /
                     ((1000000) / (CORE_CLOCK / CONFIG_KTIMER_HEARTBEAT));

    kevent = ktimer_event_create(ticks, ipc_timeout, caller);

    dbg_printf(DL_KDB, "IPC: sched timeout ticks=%d ev=%p\n", ticks, kevent);

    caller->timeout_event = (uint32_t) kevent;
}

void sys_ipc(uint32_t *param1)
{
    /* TODO: Checking of recv-mask */
    tcb_t *to_thr = NULL;
    l4_thread_t to_tid = param1[REG_R0], from_tid = param1[REG_R1];
    uint32_t timeout = param1[REG_R2];

    dbg_printf(DL_KDB, "IPC: %t->%t from=%t timeout=%p\n", caller->t_globalid,
               to_tid, from_tid, timeout);

    if (to_tid == L4_NILTHREAD && from_tid == L4_NILTHREAD) {
        dbg_printf(DL_KDB, "IPC: sleep tid=%t timeout=%p\n", caller->t_globalid,
                   timeout);
        caller->state = T_INACTIVE;
        if (timeout)
            sys_ipc_timeout(timeout);
        return;
    }

    if (to_tid != L4_NILTHREAD) {
        to_thr = thread_by_globalid(to_tid);

        if (to_tid == TID_TO_GLOBALID(THREAD_LOG)) {
            user_log(caller);
            thread_make_runnable(caller);
            return;
        } else if (to_tid == TID_TO_GLOBALID(THREAD_IRQ_REQUEST)) {
            user_interrupt_config(caller);
            thread_make_runnable(caller);
            return;
        } else if (to_thr && (to_thr->state == T_RECV_BLOCKED ||
                              to_tid == caller->t_globalid)) {
            /* To thread who is waiting for us or sends to myself */
            do_ipc(caller, to_thr);
            return;
        } else if (to_thr && to_thr->state == T_INACTIVE && to_thr->utcb &&
                   GLOBALID_TO_TID(to_thr->utcb->t_pager) ==
                       GLOBALID_TO_TID(caller->t_globalid)) {
            dbg_printf(DL_KDB,
                       "IPC: INACTIVE thread %t accepted (pager match)\n",
                       to_tid);
            uint32_t tag = ipc_read_mr(caller, 0);
            dbg_printf(DL_KDB, "IPC: startup tag=%p\n", tag);
            if (tag == 0x00000005) {
                /* Thread start protocol from pager:
                 * mr1: thread_container (wrapper), mr2: sp,
                 * mr3: stack size, mr4: entry point, mr5: entry arg */

                uint32_t mr1_container = ipc_read_mr(caller, 1);
                memptr_t sp = ipc_read_mr(caller, 2);
                size_t stack_size = ipc_read_mr(caller, 3);
                uint32_t entry_point = ipc_read_mr(caller, 4);
                uint32_t entry_arg = ipc_read_mr(caller, 5);
                uint32_t regs[4]; /* r0, r1, r2, r3 */

                dbg_printf(DL_KDB,
                           "IPC: start sp=%p size=%p entry=%p container=%p\n",
                           sp, stack_size, entry_point, mr1_container);

                /* Security check: Ensure stack is in user-writable memory */
                int pid = mempool_search(sp - stack_size, stack_size);
                mempool_t *mp = mempool_getbyid(pid);

                dbg_printf(DL_KDB, "IPC: mempool pid=%d mp=%p\n", pid, mp);

                if (!mp || !(mp->flags & MP_UW)) {
                    dbg_printf(
                        DL_KDB,
                        "IPC: REJECT invalid stack for %t: %p (pool %s)\n",
                        to_tid, sp - stack_size, mp ? mp->name : "N/A");
                    user_ipc_error(caller, UE_IPC_ABORTED | UE_IPC_PHASE_SEND);
                    thread_make_runnable(caller);
                    return;
                }

                to_thr->stack_base = sp - stack_size;
                to_thr->stack_size = stack_size;
                thread_init_canary(to_thr);

                dbg_printf(DL_KDB, "IPC: %t stack_base:%p stack_size:%p\n",
                           to_tid, to_thr->stack_base, to_thr->stack_size);

                regs[REG_R0] = (uint32_t) &kip;
                regs[REG_R1] = (uint32_t) to_thr->utcb;
                regs[REG_R2] =
                    entry_point; /* Actual entry passed to container */
                regs[REG_R3] = entry_arg;

                dbg_printf(DL_KDB, "IPC: calling thread_init_ctx\n");
                thread_init_ctx((void *) sp, (void *) mr1_container, regs,
                                to_thr);
                dbg_printf(DL_KDB, "IPC: thread_init_ctx done\n");

                dbg_printf(DL_KDB, "IPC: making caller %t runnable\n",
                           caller->t_globalid);
                thread_make_runnable(caller);

                /* Start thread */
                dbg_printf(DL_KDB, "IPC: making to_thr %t runnable\n",
                           to_thr->t_globalid);
                thread_make_runnable(to_thr);

                dbg_printf(DL_KDB, "IPC: startup complete for %t\n", to_tid);
                return;
            } else {
                /* Non-start IPC to INACTIVE thread: process
                 * typed items (MapItems) only, without full
                 * do_ipc() which requires valid ctx.sp.
                 * This allows mapping memory to thread before
                 * it's started.
                 */
                ipc_msg_tag_t tag = {.raw = ipc_read_mr(caller, 0)};
                int untyped_last = tag.s.n_untyped + 1;
                int typed_last = untyped_last + tag.s.n_typed;
                ipc_typed_item typed_item;
                int typed_item_idx = -1;

                /* Bounds check to prevent OOB MR access */
                if (typed_last > IPC_MR_COUNT) {
                    user_ipc_error(caller,
                                   UE_IPC_MSG_OVERFLOW | UE_IPC_PHASE_SEND);
                    thread_make_runnable(caller);
                    return;
                }

                dbg_printf(DL_IPC, "IPC: %t to INACTIVE %t (non-start)\n",
                           caller->t_globalid, to_thr->t_globalid);

                /* Process typed items (MapItems only) */
                for (int typed_idx = untyped_last; typed_idx < typed_last;
                     ++typed_idx) {
                    uint32_t mr_data = ipc_read_mr(caller, typed_idx);

                    if (typed_item_idx == -1) {
                        typed_item.raw = mr_data;
                        ++typed_item_idx;
                    } else if (typed_item.s.header & IPC_TI_MAP_GRANT) {
                        memptr_t map_base = typed_item.raw & 0xFFFFFFC0;
                        memptr_t map_size = mr_data & 0xFFFFFFC0;
                        int ret;

                        /* S1 fix: reject unaligned addresses */
                        if (!addr_is_fpage_aligned(map_base)) {
                            dbg_printf(
                                DL_IPC,
                                "IPC: REJECT unaligned map to INACTIVE %p\n",
                                map_base);
                            thread_make_runnable(caller);
                            return;
                        }

                        ret = map_area(
                            caller->as, to_thr->as, map_base, map_size,
                            (typed_item.s.header & IPC_TI_GRANT) ? GRANT : MAP,
                            thread_ispriviliged(caller));
                        typed_item_idx = -1;

                        if (ret < 0) {
                            dbg_printf(DL_IPC,
                                       "IPC: map to INACTIVE failed: %d\n",
                                       ret);
                        }
                    }
                }

                /* Keep thread INACTIVE, sender continues */
                thread_make_runnable(caller);
                return;
            }
        } else {
            /* Check if we should block or return error.
             * Blocking on non-existent threads or T_INACTIVE
             * threads without timeout leads to infinite stalls.
             */
            if (!to_thr) {
                /* Thread doesn't exist - return error */
                dbg_printf(DL_IPC, "IPC: %t send to non-existent %t\n",
                           caller->t_globalid, to_tid);
                user_ipc_error(caller, UE_IPC_ABORTED | UE_IPC_PHASE_SEND);
                thread_make_runnable(caller);
                return;
            }

            if (to_thr->state == T_INACTIVE && !timeout) {
                /* T_INACTIVE thread with no timeout - would block forever */
                dbg_printf(DL_IPC, "IPC: %t send to INACTIVE %t (no timeout)\n",
                           caller->t_globalid, to_tid);
                if (to_thr->utcb) {
                    dbg_printf(
                        DL_IPC,
                        "IPC: INACTIVE reject: caller=%t pager=%t utcb=%p\n",
                        caller->t_globalid, to_thr->utcb->t_pager,
                        to_thr->utcb);
                } else {
                    dbg_printf(DL_IPC, "IPC: INACTIVE reject: utcb=NULL\n");
                }
                user_ipc_error(caller, UE_IPC_ABORTED | UE_IPC_PHASE_SEND);
                thread_make_runnable(caller);
                return;
            }

            /* Block and wait for receiver */
            dbg_printf(DL_IPC, "IPC: %t SEND_BLOCKED to %t (state=%d)\n",
                       caller->t_globalid, to_tid, to_thr->state);
            caller->state = T_SEND_BLOCKED;
            caller->utcb->intended_receiver = to_tid;

            if (timeout)
                sys_ipc_timeout(timeout);

            return;
        }
    }

    if (from_tid != L4_NILTHREAD) {
        tcb_t *thr = NULL;

        if (from_tid == L4_ANYTHREAD) {
            /* Find out if there is any sending thread waiting
             * for caller
             */
            for (int i = 1; i < thread_count; ++i) {
                thr = thread_map[i];
                if (thr && thr->state == T_SEND_BLOCKED &&
                    thr->utcb->intended_receiver == caller->t_globalid) {
                    do_ipc(thr, caller);
                    return;
                }
            }
        } else if (from_tid != TID_TO_GLOBALID(THREAD_INTERRUPT)) {
            thr = thread_by_globalid(from_tid);

            if (thr && thr->state == T_SEND_BLOCKED &&
                thr->utcb->intended_receiver == caller->t_globalid) {
                do_ipc(thr, caller);
                return;
            }
        }

        /* Only receive phases, simply lock myself */
        caller->state = T_RECV_BLOCKED;
        caller->ipc_from = from_tid;

        if (from_tid == TID_TO_GLOBALID(THREAD_INTERRUPT)) {
            /* Threaded interrupt is ready */
            user_interrupt_handler_update(caller);
        }

        if (timeout)
            sys_ipc_timeout(timeout);

        dbg_printf(DL_IPC, "IPC: %t receiving\n", caller->t_globalid);

        return;
    }

    caller->state = T_SEND_BLOCKED;
}

uint32_t ipc_deliver(void *data)
{
    l4_thread_t receiver;
    tcb_t *from_thr = NULL, *to_thr = NULL;

    for (int i = 1; i < thread_count; ++i) {
        tcb_t *thr = thread_map[i];
        if (!thr)
            continue;
        switch (thr->state) {
        case T_RECV_BLOCKED:
            if (thr->ipc_from != L4_NILTHREAD &&
                thr->ipc_from != L4_ANYTHREAD &&
                thr->ipc_from != TID_TO_GLOBALID(THREAD_INTERRUPT)) {
                from_thr = thread_by_globalid(thr->ipc_from);
                /* NOTE: Must check from_thr intend to send*/
                if (from_thr && from_thr->state == T_SEND_BLOCKED &&
                    from_thr->utcb->intended_receiver == thr->t_globalid)
                    do_ipc(from_thr, thr);
            }
            break;
        case T_SEND_BLOCKED:
            receiver = thr->utcb->intended_receiver;
            if (receiver != L4_NILTHREAD && receiver != L4_ANYTHREAD) {
                to_thr = thread_by_globalid(receiver);
                if (to_thr && to_thr->state == T_RECV_BLOCKED)
                    do_ipc(thr, to_thr);
            }
            break;
        default:
            break;
        }
    }

    return 4096;
}
