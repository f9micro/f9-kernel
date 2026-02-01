/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef POSIX_TESTS_H
#define POSIX_TESTS_H

#include <test_framework.h>

/**
 * PSE51/PSE52 POSIX Compliance Test Suite
 *
 * Uses unified test framework from user/include/test_framework.h
 * Output format matches user/apps/tests/ for consistent parsing.
 */

/* Global test context - defined in main.c */
extern test_context_t posix_test_ctx;

/* Convenience macros using global context */
#define TEST_CASE_START() TSTART()
#define TEST_PASS() TPASS(posix_test_ctx)
#define TEST_FAIL(msg) TFAIL(posix_test_ctx, msg)
#define TEST_SKIP(reason) TSKIP(posix_test_ctx, reason)

#define ASSERT_EQUAL(actual, expected, msg) \
    ASSERT_EQ(posix_test_ctx, actual, expected, msg)

#define ASSERT_NOT_EQUAL(actual, expected, msg) \
    ASSERT_NE(posix_test_ctx, actual, expected, msg)

/* Re-define ASSERT_TRUE/ASSERT_FALSE to use 2-arg form for backward compat */
#undef ASSERT_TRUE
#undef ASSERT_FALSE

#define ASSERT_TRUE(cond, msg)                                                \
    do {                                                                      \
        if (!(cond)) {                                                        \
            printf("[TEST:FAIL] %s: %s\n", __func__, msg);                    \
            printf("Test %-40s[" ANSI_RED "FAIL" ANSI_RESET "]\n", __func__); \
            posix_test_ctx.failed++;                                          \
            return;                                                           \
        }                                                                     \
    } while (0)

#define ASSERT_FALSE(cond, msg)                                               \
    do {                                                                      \
        if (cond) {                                                           \
            printf("[TEST:FAIL] %s: %s\n", __func__, msg);                    \
            printf("Test %-40s[" ANSI_RED "FAIL" ANSI_RESET "]\n", __func__); \
            posix_test_ctx.failed++;                                          \
            return;                                                           \
        }                                                                     \
    } while (0)

/* Test runner declarations */
void run_pthread_tests(void);
void run_semaphore_tests(void);

#endif /* POSIX_TESTS_H */
