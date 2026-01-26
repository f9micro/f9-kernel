/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <ktimer.h>
#include <platform/irq.h>
#include <sched.h>
#include <softirq.h>
#include <thread.h>

/*
 * @file systhread.c
 * @brief Three main system threads: kernel, root and idle
 * */

tcb_t *idle;
tcb_t *kernel;
tcb_t *root;

extern void root_thread(void);
utcb_t root_utcb __KIP;

static void kernel_thread(void);
static void idle_thread(void);

void create_root_thread(void)
{
    root = thread_init(TID_TO_GLOBALID(THREAD_ROOT), &root_utcb);
    thread_space(root, TID_TO_GLOBALID(THREAD_ROOT), &root_utcb);
    as_map_user(root->as);

    uint32_t regs[4] = {
        [REG_R0] = (uint32_t) &kip,
        [REG_R1] = (uint32_t) root->utcb,
        [REG_R2] = 0,
        [REG_R3] = 0,
    };

    thread_init_ctx((void *) &root_stack_end, root_thread, regs, root);

    root->stack_base = (memptr_t) &root_stack_start;
    root->stack_size =
        (uint32_t) &root_stack_end - (uint32_t) &root_stack_start;
    thread_init_canary(root);

    root->priority = SCHED_PRIO_ROOT;
    root->base_priority = SCHED_PRIO_ROOT;
    root->state = T_RUNNABLE;
    sched_enqueue(root);
}

void create_kernel_thread(void)
{
    kernel = thread_init(TID_TO_GLOBALID(THREAD_KERNEL), NULL);

    thread_init_kernel_ctx(&kernel_stack_end, kernel);

    /* Initialize stack tracking for kernel thread.
     * Kernel stack follows idle stack in memory layout.
     */
    kernel->stack_base = (memptr_t) &idle_stack_end;
    kernel->stack_size =
        (uint32_t) &kernel_stack_end - (uint32_t) &idle_stack_end;
    thread_init_canary(kernel);

    /* This will prevent running other threads
     * than kernel until it will be initialized
     */
    kernel->priority = SCHED_PRIO_SOFTIRQ;
    kernel->base_priority = SCHED_PRIO_SOFTIRQ;
    kernel->state = T_RUNNABLE;
    sched_enqueue(kernel);
}

void create_idle_thread(void)
{
    idle = thread_init(TID_TO_GLOBALID(THREAD_IDLE), NULL);
    thread_init_ctx((void *) &idle_stack_end, idle_thread, NULL, idle);

    idle->stack_base = (memptr_t) &idle_stack_start;
    idle->stack_size =
        (uint32_t) &idle_stack_end - (uint32_t) &idle_stack_start;
    thread_init_canary(idle);

    idle->priority = SCHED_PRIO_IDLE;
    idle->base_priority = SCHED_PRIO_IDLE;
    idle->state = T_RUNNABLE;
    sched_enqueue(idle);
}

void switch_to_kernel(void) __NAKED;
void switch_to_kernel(void)
{
    create_kernel_thread();

    current = kernel;
    init_ctx_switch(&kernel->ctx, kernel_thread);
}

void set_kernel_state(thread_state_t state)
{
    /* Strict queue invariant: dequeue before blocking */
    if (state != T_RUNNABLE)
        sched_dequeue(kernel);

    kernel->state = state;

    if (state == T_RUNNABLE)
        sched_enqueue(kernel);
}

static void kernel_thread(void)
{
    while (1) {
        /* If all softirqs processed, call SVC to
         * switch context immediately
         */
        softirq_execute();
        irq_svc();
    }
}

static void idle_thread(void)
{
    while (1)
#ifdef CONFIG_KTIMER_TICKLESS
        ktimer_enter_tickless();
#else
        wait_for_interrupt();
#endif /* CONFIG_KTIMER_TICKLESS */
}
