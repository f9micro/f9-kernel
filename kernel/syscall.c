/* Copyright (c) 2013, 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <debug.h>
#include <init_hook.h>
#include <ipc.h>
#include <ktimer.h>
#include <l4/utcb.h>
#include <memory.h>
#include <platform/armv7m.h>
#include <platform/ipc-fastpath.h>
#include <platform/irq.h>
#include <sched.h>
#include <softirq.h>
#include <syscall.h>
#include <thread.h>

/* L4 Schedule result codes */
#define L4_SCHEDRESULT_ERROR 0
#define L4_SCHEDRESULT_DEAD 1
#define L4_SCHEDRESULT_INACTIVE 2
#define L4_SCHEDRESULT_RUNNING 3
#define L4_SCHEDRESULT_PENDING_SEND 4
#define L4_SCHEDRESULT_SENDING 5
#define L4_SCHEDRESULT_WAITING 6
#define L4_SCHEDRESULT_RECEIVING 7

tcb_t *caller;

/* Always returns 0; fastpath and slowpath both use PendSV for context switching
 */
int __svc_handler(void)
{
    extern tcb_t *kernel;
    uint32_t *svc_param;
    uint8_t svc_num;

    /* Kernel requests context switch, satisfy it */
    if (thread_current() == kernel)
        return 0;

    caller = thread_current();

    /* CRITICAL: Read SVC frame from CURRENT PSP, not stale ctx.sp!
     * ctx.sp is only updated during context switch, but user thread may have
     * pushed/popped since then. Hardware SVC frame is on the CURRENT stack.
     */
    uint32_t psp;
    __asm__ __volatile__("mrs %0, psp" : "=r"(psp));
    svc_param = (uint32_t *) psp;
    svc_num = ((char *) svc_param[REG_PC])[-2];

    if (svc_num == SYS_IPC) {
        extern volatile uint32_t __irq_saved_regs[8];

        /* Try fastpath with saved message registers */
        if (ipc_fastpath_helper(caller, svc_param, __irq_saved_regs)) {
            /* Fastpath succeeded - MRs copied, threads enqueued, PendSV
             * requested. Caller is already T_RUNNABLE and enqueued by fastpath.
             * Just return normally - PendSV will do the context switch. */
            return 0; /* Normal return, PendSV will switch */
        }

        /* Fastpath failed, use slowpath */
        /* Slowpath will dequeue caller in softirq handler */
        sched_dequeue(caller);
        caller->state = T_SVC_BLOCKED;
        softirq_schedule(SYSCALL_SOFTIRQ);
        return 0;
    } else {
        /* Non-IPC syscall */
        sched_dequeue(caller);
        caller->state = T_SVC_BLOCKED;
        softirq_schedule(SYSCALL_SOFTIRQ);
        return 0;
    }
}

/* Custom SVC handler with fastpath support */
void svc_handler(void) __NAKED;
void svc_handler(void)
{
    /* Save LR and R4-R11 before any C code */
    __asm__ __volatile__("push {lr}");
    __asm__ __volatile__(
        "ldr r0, =__irq_saved_regs\n\t"
        "stm r0, {r4-r11}" ::
            : "r0", "memory");

    /* Call C handler - always returns 0 (context switch via PendSV) */
    __svc_handler();

    /* Restore R4-R11 BEFORE returning so PendSV saves original values */
    __asm__ __volatile__(
        "ldr r0, =__irq_saved_regs\n\t"
        "ldm r0, {r4-r11}\n\t"
        /* Now set PendSV and return normally */
        "ldr r0, =0xE000ED04\n\t" /* SCB_ICSR */
        "ldr r1, [r0]\n\t"
        "orr r1, #0x10000000\n\t" /* SCB_ICSR_PENDSVSET */
        "str r1, [r0]\n\t"
        "pop {lr}\n\t"
        "bx lr" ::
            : "r0", "r1", "memory");
}

void syscall_init()
{
    softirq_register(SYSCALL_SOFTIRQ, syscall_handler);
}

INIT_HOOK(syscall_init, INIT_LEVEL_KERNEL);

/**
 * Schedule system call handler.
 * Implements L4 Schedule interface with PTS support.
 *
 * Parameters (from L4 spec):
 *   R0: dest - target thread ID
 *   R1: time_control - timeslice control
 *   R2: processor_control - processor number
 *   R3: prio_control - priority and stride
 *   R4: preemption_control - preemption threshold (PTS)
 *   R5: old_control (output pointer)
 *
 * WCET Analysis:
 *   - thread_by_globalid(): O(1) lookup (~10 instructions)
 *   - Priority update: O(1) via sched_set_priority() (~50 instructions)
 *     - sched_is_queued(): O(1) pointer check
 *     - sched_dequeue(): O(1) list ops + bitmap clear
 *     - sched_enqueue(): O(1) list ops + bitmap set
 *   - Threshold update: O(1) via sched_preemption_change() (~30 instructions)
 *     - Validation: O(1) range checks
 *     - IRQ-safe critical section: O(1) MSR instructions
 *     - Threshold calculation: O(1) min() operation
 *     - Optional reschedule: O(1) CLZ + bitmap ops + PendSV trigger
 *   - State return: O(1) switch (~10 instructions)
 *
 *   Total WCET: O(1) - approximately 100 instructions
 *   At 168 MHz: ~0.6 microseconds worst case (bounded and deterministic)
 */
static void sys_schedule(uint32_t *param1, uint32_t *param2)
{
    l4_thread_t dest = param1[REG_R0];
    uint32_t prio_control = param1[REG_R3];
    uint32_t preemption_control = param2[0];        /* R4 */
    uint32_t *old_control = (uint32_t *) param2[1]; /* R5 */

    tcb_t *target = thread_by_globalid(dest);
    if (!target) {
        param1[REG_R0] = L4_SCHEDRESULT_ERROR;
        return;
    }

    /* Extract priority from prio_control (lower 8 bits) */
    uint8_t new_priority = prio_control & 0xFF;

    /* Extract preemption threshold from preemption_control (bits 16-23) */
    uint8_t new_threshold = (preemption_control >> 16) & 0xFF;

    /* Save old control value if requested */
    if (old_control)
        *old_control = (target->preempt_threshold << 16) | target->priority;

    /* Update priority if specified (0xFF means "don't change") */
    if (new_priority != 0xFF && new_priority < SCHED_PRIORITY_LEVELS) {
        sched_set_priority(target, new_priority);
        target->user_priority = new_priority;
        target->base_priority = new_priority;
    }

    /* Update preemption threshold if specified (0xFF means "don't change") */
    if (new_threshold != 0xFF) {
        if (sched_preemption_change(target, new_threshold, NULL) < 0) {
            param1[REG_R0] = L4_SCHEDRESULT_ERROR;
            return;
        }
    }

    /* Return thread state */
    switch (target->state) {
    case T_INACTIVE:
        param1[REG_R0] = L4_SCHEDRESULT_INACTIVE;
        break;
    case T_RUNNABLE:
        param1[REG_R0] = L4_SCHEDRESULT_RUNNING;
        break;
    case T_RECV_BLOCKED:
        param1[REG_R0] = L4_SCHEDRESULT_RECEIVING;
        break;
    case T_SEND_BLOCKED:
        param1[REG_R0] = L4_SCHEDRESULT_SENDING;
        break;
    case T_SVC_BLOCKED:
        param1[REG_R0] = L4_SCHEDRESULT_WAITING;
        break;
    default:
        param1[REG_R0] = L4_SCHEDRESULT_ERROR;
        break;
    }
}

static void sys_thread_control(uint32_t *param1, uint32_t *param2)
{
    l4_thread_t dest = param1[REG_R0];
    l4_thread_t space = param1[REG_R1];
    l4_thread_t pager = param1[REG_R3];

    if (space != L4_NILTHREAD) {
        /* Creation of thread */
        void *utcb = (void *) param2[0]; /* R4 */
        mempool_t *utcb_pool =
            mempool_getbyid(mempool_search((memptr_t) utcb, UTCB_SIZE));

        if (!utcb_pool || !(utcb_pool->flags & (MP_UR | MP_UW))) {
            /* Incorrect UTCB relocation */
            param1[REG_R0] = 0;
            return;
        }

        /* Reject unaligned UTCB addresses to prevent over-mapping */
        if (!addr_is_fpage_aligned((memptr_t) utcb)) {
            /* UTCB must be aligned to fpage boundary */
            param1[REG_R0] = 0;
            return;
        }

        tcb_t *thr = thread_create(dest, utcb);
        if (!thr) {
            /* Thread creation failed */
            param1[REG_R0] = 0;
            return;
        }
        thread_space(thr, space, utcb);
        thr->utcb->t_pager = pager;
        param1[REG_R0] = 1;
    } else {
        /* Removal of thread */
        tcb_t *thr = thread_by_globalid(dest);
        if (!thr) {
            /* Thread not found */
            param1[REG_R0] = 0;
            return;
        }
        /* thread_destroy() handles AS refcount via as_put() */
        thread_destroy(thr);
        param1[REG_R0] = 1;
    }
}

/**
 * Timer notification syscall handler.
 * Creates a timer that delivers notifications to the calling thread.
 *
 * Parameters:
 *   R0: ticks - timer delay/period in system ticks
 *   R1: notify_bits - notification bit mask to signal
 *   R2: periodic - 0 for one-shot, 1 for periodic timer
 *
 * Returns (R0):
 *   Non-zero timer handle on success
 *   0 on failure (invalid parameters or resource exhaustion)
 *
 * Performance Analysis:
 *   - thread_current(): O(1) - ~5 instructions
 *   - Parameter validation: O(1) - ~10 instructions
 *   - ktimer_event_create_notify(): O(1) - ~150 instructions
 *     - ktable_alloc(): O(1) bitmap scan
 *     - ktimer_event_schedule(): O(k) where k = active timers
 *       Typically k < 10, worst case O(64) for CONFIG_MAX_KT_EVENTS
 *   - Return: O(1) - ~5 instructions
 *
 *   Total WCET: O(k) where k = active timers
 *   Typical case (k < 10): ~200 cycles = 1.2μs @ 168MHz
 *   Worst case (k = 64): ~500 cycles = 3.0μs @ 168MHz
 *
 * Safety:
 *   - Validates notify_bits (non-zero required)
 *   - Validates ticks (non-zero required)
 *   - Validates periodic flag (0 or 1)
 *   - Current thread always valid (checked by kernel)
 *   - ktimer pool exhaustion returns 0 (graceful degradation)
 */
static void sys_timer_notify(uint32_t *param1)
{
    uint32_t ticks = param1[REG_R0];
    uint32_t notify_bits = param1[REG_R1];
    uint32_t periodic = param1[REG_R2];
    tcb_t *current = thread_current();

    /* Validate parameters */
    if (ticks == 0 || notify_bits == 0) {
        param1[REG_R0] = 0; /* Invalid parameters */
        return;
    }

    /* Clamp periodic to boolean */
    periodic = (periodic != 0) ? 1 : 0;

    /* Create notification timer */
    ktimer_event_t *timer =
        ktimer_event_create_notify(ticks, current, notify_bits, periodic);

    /* Return timer handle (or 0 on failure) */
    param1[REG_R0] = (uint32_t) timer;

    dbg_printf(DL_SYSCALL,
               "SYS_TIMER_NOTIFY: ticks=%d bits=0x%x periodic=%d -> %p\n",
               ticks, notify_bits, periodic, timer);
}

void syscall_handler()
{
    uint32_t *svc_param1 = (uint32_t *) caller->ctx.sp;
    uint32_t svc_num = ((char *) svc_param1[REG_PC])[-2];
    uint32_t *svc_param2 = caller->ctx.regs;

    if (svc_num == SYS_THREAD_CONTROL) {
        /* Simply call thread_create
         * TODO: checking globalid
         * TODO: pagers and schedulers
         */
        sys_thread_control(svc_param1, svc_param2);
        caller->state = T_RUNNABLE;
        sched_enqueue(caller);
    } else if (svc_num == SYS_SCHEDULE) {
        /* Schedule system call - priority and preemption threshold control */
        sys_schedule(svc_param1, svc_param2);
        caller->state = T_RUNNABLE;
        sched_enqueue(caller);
    } else if (svc_num == SYS_TIMER_NOTIFY) {
        /* Timer notification syscall - create notification timer */
        sys_timer_notify(svc_param1);
        caller->state = T_RUNNABLE;
        sched_enqueue(caller);
    } else if (svc_num == SYS_IPC) {
        sys_ipc(svc_param1);
    } else {
        dbg_printf(DL_SYSCALL, "SVC: %d called [%d, %d, %d, %d]\n", svc_num,
                   svc_param1[REG_R0], svc_param1[REG_R1], svc_param1[REG_R2],
                   svc_param1[REG_R3]);
        caller->state = T_RUNNABLE;
        sched_enqueue(caller);
    }
}
