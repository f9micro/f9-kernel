/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4io.h>
#include <user_runtime.h>

#include "tests.h"

/* Global test context */
__USER_BSS test_context_t test_ctx;

/*
 * Run all tests and report results.
 */
#ifndef FAULT_TYPE
__USER_TEXT
static void run_all_tests(void)
{
    test_ctx.passed = 0;
    test_ctx.failed = 0;
    test_ctx.skipped = 0;

    TEST_START("test_suite");

    /* Thread identity tests (no thread creation needed) */
    test_thread_self();
    test_thread_global_id();
    test_thread_pager();

    /* Timer tests */
    test_timer_period();
    test_timer_sleep();

    /* KIP tests */
    test_kip_access();
    test_kip_processors();

    /* IPC tests (also validates thread creation via pager) */
    test_ipc_basic();
    /* TODO: test_ipc_multiword() has timing issues, needs debugging */

    /* Functional safety tests */
    test_ipc_timeout_send();
    test_ipc_timeout_receive();
    test_timer_zero_sleep();
    test_timer_monotonicity();
    test_thread_priority();
    test_thread_yield();

    /* Scheduler validation tests (based on formal invariants) */
    test_sched_syscall();
    test_sched_yield_returns();
    test_sched_idle_fallback();
    test_sched_round_robin();
    test_sched_no_starvation();
    test_sched_ipc_priority_boost();
    /* Note: test_sched_priority_order() requires more thread resources */

    /* Preemption-Threshold Scheduling (PTS) tests */
    test_pts_threshold_set();
    test_pts_threshold_bounds();
    test_pts_reduced_preemption();
    test_pts_priority_inheritance();

    /* ExchangeRegisters tests */
    test_exreg_halt();
    test_exreg_resume();
    test_exreg_sp_ip();
    test_exreg_pager();
    test_exreg_status();
    test_exreg_cross_space();

    /* ThreadControl tests */
    test_tcontrol_create();
    test_tcontrol_config();
    test_tcontrol_delete();
    test_tcontrol_utcb();
    test_tcontrol_invalid();
    test_tcontrol_preemption();

    /* Memory operation tests */
    test_memory_map();
    test_memory_grant();
    test_memory_rights();
    test_memory_unmap();
    test_memory_sharing();
    test_memory_xspace_write();
    test_memory_invalid();
    test_memory_mpu_exhaustion();
    test_memory_mpu_cleanup();

    /* IPC pagefault tests */
    test_ipc_pf_unmapped();
    test_ipc_pf_receive();
    test_ipc_pf_map_retry();
    test_ipc_pf_abort();

    /* IPC error tests */
    test_ipc_error_nonexist_send();
    test_ipc_error_nonexist_recv();
    test_ipc_error_cancelled_send();
    test_ipc_error_cancelled_recv();
    test_ipc_error_overflow_send();
    test_ipc_error_timeout_sender();
    test_ipc_error_timeout_current();
    test_ipc_error_timeout_partner();
    test_ipc_error_aborted_send();
    test_ipc_error_aborted_recv();

    /* ARM architecture tests */
    test_arm_mpu_config();
    test_arm_lazy_fpu();
    test_arm_irq_latency();
    test_arm_pendsv();
    test_arm_utcb_align();
    test_arm_stack_align();
    test_arm_unaligned();

#ifdef CONFIG_EXTI_INTERRUPT_TEST
    /* IRQ test (requires hardware EXTI support) */
    test_irq_exti();
#endif

    /* Summary and exit */
    TEST_SUMMARY();
    TEST_EXIT(test_ctx.failed > 0 ? 1 : 0);
}
#endif /* !FAULT_TYPE */

/*
 * Main entry point for test suite.
 * Runs either smoke tests or fault tests based on FAULT_TYPE.
 */
__USER_TEXT
static void *test_main(void *user)
{
    printf("\nF9 Microkernel Tests\n");

#ifdef FAULT_TYPE
    /* Run fault test (does not return - triggers kernel panic) */
    run_fault_test();
#else
    /* Run test suite */
    run_all_tests();
#endif

    return NULL;
}

/*
 * User space declaration.
 * TID 257: after l4test (256)
 *
 * For test suite:
 *   RES_FPAGE: 8192 bytes for stack/UTCB (supports 10 thread nodes)
 *   HEAP_FPAGE: 512 bytes for thread pool metadata
 *
 * For canary fault test:
 *   Smaller stack to make overflow easier to trigger
 */
#if defined(FAULT_TYPE) && (FAULT_TYPE == FAULT_CANARY)
/* Smaller stack for canary test (must match STACK_SIZE_WORDS in test-fault.c)
 */
DECLARE_USER(257,
             tests,
             test_main,
             DECLARE_FPAGE(0x0, 2048) DECLARE_FPAGE(0x0, 512));
#else
/* Normal stack for test suite and MPU fault test */
DECLARE_USER(257,
             tests,
             test_main,
             DECLARE_FPAGE(0x0, 8192) DECLARE_FPAGE(0x0, 512));
#endif
