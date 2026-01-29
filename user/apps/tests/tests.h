/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __TESTS_H__
#define __TESTS_H__

#include <l4io.h>

/*
 * Test Framework
 *
 * Machine-parseable test output format for automated testing.
 * Output is parsed by scripts/qemu-test.py to determine pass/fail.
 *
 * Format:
 *   [TEST:START] suite_name
 *   [TEST:RUN] test_name
 *   [TEST:PASS] test_name
 *   [TEST:FAIL] test_name
 *   [TEST:SUMMARY] passed=N failed=M
 *   [TEST:EXIT] code
 */

/* Test context structure */
typedef struct {
    int passed;
    int failed;
    int skipped;
} test_context_t;

/* Global test context - defined in main.c */
extern test_context_t test_ctx;

/*
 * Test output macros
 *
 * Format matches libiui style:
 *   Test <name padded to 40 chars>[ OK ] or [FAIL]
 *
 * Machine-parseable markers [TEST:*] are also emitted for qemu-test.py
 */

/* ANSI color codes */
#define ANSI_GREEN "\033[32m"
#define ANSI_RED "\033[31m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_RESET "\033[0m"

#define TEST_START(suite)                      \
    do {                                       \
        printf("[TEST:START] %s\n", suite);    \
        printf("=== Running %s ===\n", suite); \
    } while (0)

#define TEST_RUN(name) printf("[TEST:RUN] %s\n", name)

#define TEST_PASS(name)                                                 \
    do {                                                                \
        test_ctx.passed++;                                              \
        printf("[TEST:PASS] %s\n", name);                               \
        printf("Test %-40s[ " ANSI_GREEN "OK" ANSI_RESET " ]\n", name); \
    } while (0)

#define TEST_FAIL(name)                                               \
    do {                                                              \
        test_ctx.failed++;                                            \
        printf("[TEST:FAIL] %s\n", name);                             \
        printf("Test %-40s[" ANSI_RED "FAIL" ANSI_RESET "]\n", name); \
    } while (0)

#define TEST_SKIP(name)                                                  \
    do {                                                                 \
        test_ctx.skipped++;                                              \
        printf("[TEST:SKIP] %s\n", name);                                \
        printf("Test %-40s[" ANSI_YELLOW "SKIP" ANSI_RESET "]\n", name); \
    } while (0)

#define TEST_SUMMARY()                                                         \
    printf("[TEST:SUMMARY] passed=%d failed=%d skipped=%d\n", test_ctx.passed, \
           test_ctx.failed, test_ctx.skipped)

#define TEST_EXIT(code) printf("[TEST:EXIT] %d\n", code)

/* Test assertion - non-blocking unlike L4_KDB_Enter */
#define TEST_ASSERT(name, condition) \
    do {                             \
        if (condition) {             \
            TEST_PASS(name);         \
        } else {                     \
            TEST_FAIL(name);         \
        }                            \
    } while (0)

/* Test function declarations */

/* IPC tests (test-ipc.c) */
void test_ipc_basic(void);
void test_ipc_multiword(void);

/* Thread tests (test-thread.c) */
void test_thread_self(void);
void test_thread_global_id(void);
void test_thread_pager(void);
void test_thread_create(void);

/* Timer tests (test-timer.c) */
void test_timer_period(void);
void test_timer_sleep(void);

/* KIP tests (test-kip.c) */
void test_kip_access(void);
void test_kip_processors(void);

/* IRQ tests (test-irq.c) - requires CONFIG_EXTI_INTERRUPT_TEST */
#ifdef CONFIG_EXTI_INTERRUPT_TEST
void test_irq_exti(void);
#endif

/* Functional safety tests (test-safety.c) */
void test_ipc_timeout_send(void);
void test_ipc_timeout_receive(void);
void test_timer_zero_sleep(void);
void test_timer_monotonicity(void);
void test_thread_priority(void);
void test_thread_yield(void);

/* Scheduler validation tests (test-sched.c) */
void test_sched_priority_order(void);
void test_sched_round_robin(void);
void test_sched_no_starvation(void);
void test_sched_yield_returns(void);
void test_sched_idle_fallback(void);
void test_sched_ipc_priority_boost(void);
void test_sched_syscall(void);

/* PTS (Preemption-Threshold Scheduling) tests */
void test_pts_threshold_set(void);
void test_pts_threshold_bounds(void);
void test_pts_reduced_preemption(void);
void test_pts_priority_inheritance(void);

/* ExchangeRegisters tests (test-exreg.c) */
void test_exreg_halt(void);
void test_exreg_resume(void);
void test_exreg_sp_ip(void);
void test_exreg_pager(void);
void test_exreg_status(void);
void test_exreg_cross_space(void);

/* ThreadControl tests (test-tcontrol.c) */
void test_tcontrol_create(void);
void test_tcontrol_config(void);
void test_tcontrol_delete(void);
void test_tcontrol_utcb(void);
void test_tcontrol_invalid(void);
void test_tcontrol_preemption(void);

/* Memory operation tests (test-memory.c) */
void test_memory_map(void);
void test_memory_grant(void);
void test_memory_rights(void);
void test_memory_unmap(void);
void test_memory_sharing(void);
void test_memory_xspace_write(void);
void test_memory_invalid(void);
void test_memory_mpu_exhaustion(void);
void test_memory_mpu_cleanup(void);

/* IPC pagefault tests (test-ipc-pf.c) */
void test_ipc_pf_unmapped(void);
void test_ipc_pf_receive(void);
void test_ipc_pf_map_retry(void);
void test_ipc_pf_abort(void);

/* IPC error tests (test-ipc-error.c) */
void test_ipc_error_nonexist_send(void);
void test_ipc_error_nonexist_recv(void);
void test_ipc_error_cancelled_send(void);
void test_ipc_error_cancelled_recv(void);
void test_ipc_error_overflow_send(void);
void test_ipc_error_timeout_sender(void);
void test_ipc_error_timeout_current(void);
void test_ipc_error_timeout_partner(void);
void test_ipc_error_aborted_send(void);
void test_ipc_error_aborted_recv(void);

/* ARM architecture tests (test-arm.c) */
void test_arm_mpu_config(void);
void test_arm_lazy_fpu(void);
void test_arm_irq_latency(void);
void test_arm_pendsv(void);
void test_arm_utcb_align(void);
void test_arm_stack_align(void);
void test_arm_unaligned(void);

/* Notification system tests (test-notification.c) */
void test_notification_timer_oneshot(void);
void test_notification_timer_periodic(void);
void test_notification_multi_timer(void);
void test_notification_statistics(void);
void test_notification_architecture(void);

/* Test helper functions (tests_helper_core.c) */
void test_skip(const char *name, const char *reason);

/* Fault test type constants */
#define FAULT_MPU 1
#define FAULT_CANARY 2

/* Fault tests (test-fault.c) - requires FAULT_TYPE define */
#ifdef FAULT_TYPE
void run_fault_test(void);
#endif

#endif /* __TESTS_H__ */
