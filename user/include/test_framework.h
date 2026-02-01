/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <l4io.h>

/**
 * Unified Test Framework for F9 Microkernel
 *
 * Machine-parseable test output format for automated testing.
 * Output is parsed by scripts/qemu-test.py to determine pass/fail.
 *
 * Format:
 *   [TEST:START] suite_name
 *   [TEST:RUN] test_name
 *   [TEST:PASS] test_name
 *   [TEST:FAIL] test_name
 *   [TEST:SKIP] test_name
 *   [TEST:SUMMARY] passed=N failed=M skipped=K
 *   [TEST:EXIT] code
 *
 * Human-readable format:
 *   Test <name padded to 40 chars>[ OK ] or [FAIL] or [SKIP]
 */

/* ANSI color codes */
#define ANSI_GREEN "\033[32m"
#define ANSI_RED "\033[31m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_RESET "\033[0m"

/* Test context for tracking results */
typedef struct {
    int passed;
    int failed;
    int skipped;
} test_context_t;

/* Initialize test context (call once at suite start) */
#define TEST_INIT(ctx)     \
    do {                   \
        (ctx).passed = 0;  \
        (ctx).failed = 0;  \
        (ctx).skipped = 0; \
    } while (0)

/* Suite start marker */
#define TEST_SUITE_START(suite)             \
    do {                                    \
        printf("[TEST:START] %s\n", suite); \
        printf("=== %s ===\n", suite);      \
    } while (0)

/* Individual test markers */
#define TEST_RUN(name) printf("[TEST:RUN] %s\n", name)

#define TEST_PASS_MSG(ctx, name)                                        \
    do {                                                                \
        (ctx).passed++;                                                 \
        printf("[TEST:PASS] %s\n", name);                               \
        printf("Test %-40s[ " ANSI_GREEN "OK" ANSI_RESET " ]\n", name); \
    } while (0)

#define TEST_FAIL_MSG(ctx, name)                                      \
    do {                                                              \
        (ctx).failed++;                                               \
        printf("[TEST:FAIL] %s\n", name);                             \
        printf("Test %-40s[" ANSI_RED "FAIL" ANSI_RESET "]\n", name); \
    } while (0)

#define TEST_SKIP_MSG(ctx, name)                                         \
    do {                                                                 \
        (ctx).skipped++;                                                 \
        printf("[TEST:SKIP] %s\n", name);                                \
        printf("Test %-40s[" ANSI_YELLOW "SKIP" ANSI_RESET "]\n", name); \
    } while (0)

/* Suite summary and exit */
#define TEST_SUMMARY(ctx)                                                   \
    printf("[TEST:SUMMARY] passed=%d failed=%d skipped=%d\n", (ctx).passed, \
           (ctx).failed, (ctx).skipped)

#define TEST_EXIT(code) printf("[TEST:EXIT] %d\n", code)

/* Test assertions - update context and report */
#define TEST_ASSERT(ctx, name, condition) \
    do {                                  \
        if (condition) {                  \
            TEST_PASS_MSG(ctx, name);     \
        } else {                          \
            TEST_FAIL_MSG(ctx, name);     \
        }                                 \
    } while (0)

/*
 * Function-based test macros (use __func__ for test name)
 * These are convenient for tests where each function is one test case.
 */

/* Start a test (emits RUN marker) */
#define TSTART() printf("[TEST:RUN] %s\n", __func__)

/* Pass current test and return */
#define TPASS(ctx)                                                          \
    do {                                                                    \
        (ctx).passed++;                                                     \
        printf("[TEST:PASS] %s\n", __func__);                               \
        printf("Test %-40s[ " ANSI_GREEN "OK" ANSI_RESET " ]\n", __func__); \
        return;                                                             \
    } while (0)

/* Fail current test with message and return */
#define TFAIL(ctx, msg)                                                   \
    do {                                                                  \
        (ctx).failed++;                                                   \
        printf("[TEST:FAIL] %s: %s\n", __func__, msg);                    \
        printf("Test %-40s[" ANSI_RED "FAIL" ANSI_RESET "]\n", __func__); \
        return;                                                           \
    } while (0)

/* Skip current test with reason and return */
#define TSKIP(ctx, reason)                                                   \
    do {                                                                     \
        (ctx).skipped++;                                                     \
        printf("[TEST:SKIP] %s: %s\n", __func__, reason);                    \
        printf("Test %-40s[" ANSI_YELLOW "SKIP" ANSI_RESET "]\n", __func__); \
        return;                                                              \
    } while (0)

/*
 * Assertion macros (fail and return on failure)
 */

/* Use statement expressions to capture values once, preventing double
 * evaluation
 */
#define ASSERT_EQ(ctx, actual, expected, msg)                                 \
    do {                                                                      \
        __typeof__(actual) _actual_val = (actual);                            \
        __typeof__(expected) _expected_val = (expected);                      \
        if (_actual_val != _expected_val) {                                   \
            printf("[TEST:FAIL] %s: %s (expected %d, got %d)\n", __func__,    \
                   msg, (int) _expected_val, (int) _actual_val);              \
            printf("Test %-40s[" ANSI_RED "FAIL" ANSI_RESET "]\n", __func__); \
            (ctx).failed++;                                                   \
            return;                                                           \
        }                                                                     \
    } while (0)

#define ASSERT_NE(ctx, actual, unexpected, msg)                               \
    do {                                                                      \
        __typeof__(actual) _actual_val = (actual);                            \
        __typeof__(unexpected) _unexpected_val = (unexpected);                \
        if (_actual_val == _unexpected_val) {                                 \
            printf("[TEST:FAIL] %s: %s (got %d, should differ)\n", __func__,  \
                   msg, (int) _actual_val);                                   \
            printf("Test %-40s[" ANSI_RED "FAIL" ANSI_RESET "]\n", __func__); \
            (ctx).failed++;                                                   \
            return;                                                           \
        }                                                                     \
    } while (0)

#define ASSERT_TRUE(ctx, cond, msg)                                           \
    do {                                                                      \
        if (!(cond)) {                                                        \
            printf("[TEST:FAIL] %s: %s\n", __func__, msg);                    \
            printf("Test %-40s[" ANSI_RED "FAIL" ANSI_RESET "]\n", __func__); \
            (ctx).failed++;                                                   \
            return;                                                           \
        }                                                                     \
    } while (0)

#define ASSERT_FALSE(ctx, cond, msg)                                          \
    do {                                                                      \
        if (cond) {                                                           \
            printf("[TEST:FAIL] %s: %s\n", __func__, msg);                    \
            printf("Test %-40s[" ANSI_RED "FAIL" ANSI_RESET "]\n", __func__); \
            (ctx).failed++;                                                   \
            return;                                                           \
        }                                                                     \
    } while (0)

#endif /* TEST_FRAMEWORK_H */
