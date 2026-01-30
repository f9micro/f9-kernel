/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <kip.h>
#include <lib/ktable.h>
#include <memory.h>
#include <types.h>

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

#define L4_NILTHREAD 0
#define L4_ANYLOCALTHREAD 0xFFFFFFC0
#define L4_ANYTHREAD 0xFFFFFFFF

#define GLOBALID_TO_TID(id) (id >> 14)
#define TID_TO_GLOBALID(id) (id << 14)

#define THREAD_BY_TID(id) thread_by_globalid(TID_TO_GLOBALID(id))

/* Stack canary for overflow detection.
 * Placed at stack_base (lowest address). Checked on context switch.
 */
#define STACK_CANARY 0xDEADBEEF

typedef enum {
    THREAD_IDLE,
    THREAD_KERNEL,
    THREAD_ROOT,
    THREAD_INTERRUPT,
    THREAD_IRQ_REQUEST,
    THREAD_LOG,
    THREAD_SYS = 16,                     /* Systembase */
    THREAD_USER = CONFIG_INTR_THREAD_MAX /* Userbase */
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
    /* Hot scheduler fields - Cache Line 0 */
    struct {
        struct tcb *prev, *next;
    } sched_link; /* 8 bytes (0-7) */

    thread_state_t state;      /* 4 bytes (8-11) */
    uint8_t priority;          /* 1 byte  (12) - effective priority */
    uint8_t base_priority;     /* 1 byte  (13) - natural priority */
    uint8_t preempt_threshold; /* 1 byte  (14) - preemption threshold */
    uint8_t user_priority;     /* 1 byte  (15) - original user priority */

    l4_thread_t t_globalid; /* 4 bytes (16-19) */
    l4_thread_t t_localid;  /* 4 bytes (20-23) */

    memptr_t stack_base; /* 4 bytes (24-27) */
    size_t stack_size;   /* 4 bytes (28-31) */
    /* End of Cache Line 0 (32 bytes) */

    context_t ctx;

    /* PTS (Preemption-Threshold Scheduling) fields */
    uint8_t user_preempt_threshold; /* Original user-set threshold */
    uint8_t inherit_priority;       /* Priority Inheritance Protocol */
    uint8_t _pts_pad[2];            /* Alignment padding */

    as_t *as;
    struct utcb *utcb;

    l4_thread_t ipc_from;

    struct tcb *t_sibling;
    struct tcb *t_parent;
    struct tcb *t_child;

    uint32_t timeout_event;

    /* Event-chaining callback for notification objects.
     * Invoked after IPC delivery with interrupts enabled.
     * SAFETY: Must be internal kernel handler only.
     * RE-ENTRANCY: Callback must be re-entrant safe.
     * CONSTRAINT: Callback MUST NOT destroy its own TCB.
     *
     * Callback signature:
     *   void callback(tcb_t *tcb, uint32_t notify_bits, uint32_t notify_data)
     * - notify_bits: Notification bit mask being delivered
     * - notify_data: Optional event-specific data (0 for IPC notifications)
     */
    void (*ipc_notify)(struct tcb *tcb,
                       uint32_t notify_bits,
                       uint32_t notify_data);

    /* Notification bit mask for event multiplexing.
     * Used by user-space reactor pattern (multi-source events).
     */
    uint32_t notify_bits;

    /* Optional event-specific data payload.
     * Used for extended notifications (e.g., IRQ number for high IRQs).
     * Set by notification_post() and retrieved by notification_get_extended().
     */
    uint32_t notify_data;

    /* Recursion protection: prevent infinite callback nesting.
     * 0 = not in callback, >0 = callback depth.
     * Updated atomically with IRQ masking to prevent races.
     */
    uint8_t notify_depth;

    /* Lifecycle tracking: generation counter for use-after-free detection.
     * Incremented on thread destruction. Used to detect TCB invalidation
     * after callback execution (debug/safety feature).
     */
    uint8_t notify_generation;

    /* Fast-path optimization: pending notification flag.
     * Set when notify_bits != 0, cleared when notify_bits == 0.
     * Allows IPC path to skip notification checks with single word read.
     */
    uint8_t notify_pending;

    uint8_t _notify_pad[1]; /* Alignment padding */

    /* Short message buffer for IPC fastpath optimization.
     * Extends fastpath coverage from 32 bytes (registers only) to 160 bytes.
     * Maps to L4 virtual registers MR8-MR39 (32 additional 4-byte registers).
     *
     * Memory layout:
     * - MR0-MR7:   ctx.regs[0-7]     (32 bytes, R4-R11 hardware registers)
     * - MR8-MR39:  msg_buffer[0-31]  (128 bytes, TCB-embedded buffer)
     * - MR40-MR47: utcb->mr[0-7]     (32 bytes, UTCB overflow)
     *
     * Fastpath eligibility: n_untyped <= 39 (160 bytes total)
     * Expected fastpath coverage: 70% → 95%
     *
     * RAM impact: 128 bytes × 20 threads = 2.5 KB (1.3% of 192KB)
     */
    uint32_t msg_buffer[32];
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
        return 1; /* No stack tracking, skip check */
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

/**
 * Priority Inheritance Protocol (PIP) functions.
 * Used when threads block on synchronization primitives (IPC, mutexes).
 */

/**
 * Boost holder's priority when waiter blocks on it.
 * Temporarily raises holder's priority to waiter's if higher.
 *
 * @param waiter Thread that is blocking (high priority)
 * @param holder Thread holding the resource (low priority)
 */
void thread_priority_inherit(tcb_t *waiter, tcb_t *holder);

/**
 * Restore holder's original priority after releasing resource.
 * Recalculates preempt_threshold considering inheritance.
 *
 * @param holder Thread releasing the resource
 */
void thread_priority_disinherit(tcb_t *holder);

#endif /* THREAD_H_ */
