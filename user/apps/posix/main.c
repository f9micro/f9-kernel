/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 * PSE51 (POSIX Minimal Realtime System Profile) Compliance Test Suite
 *
 * Tests compliance with IEEE Std 1003.13-2003 PSE51 Profile:
 * - POSIX Threads (pthread_create, pthread_join, pthread_mutex_*)
 * - Semaphores (sem_init, sem_wait, sem_post)
 * - Thread attributes and synchronization primitives
 *
 * See: https://pubs.opengroup.org/onlinepubs/9699919799/
 */

#include <l4/utcb.h>
#include <l4io.h>
#include <platform/link.h>
#include <types.h>
#include <user_runtime.h>
#include "posix_tests.h"

#define STACK_SIZE 512

/* Global test context */
__USER_BSS test_context_t posix_test_ctx;

/* Main test entry point */
__USER_TEXT
void *posix_test_main(void *arg)
{
    (void) arg;

    TEST_INIT(posix_test_ctx);
    TEST_SUITE_START("posix_compliance");

    printf("\n");
    printf("========================================\n");
    printf("  F9 Microkernel PSE51 Compliance Tests\n");
    printf("========================================\n");
    printf("Profile: POSIX Minimal Realtime System (PSE51)\n");
    printf("Standard: IEEE Std 1003.13-2003\n");
    printf("\n");

    run_pthread_tests();
    run_semaphore_tests();

    printf("\n");
    printf("========================================\n");
    printf("  All PSE51 Compliance Tests Complete\n");
    printf("========================================\n");
    printf("\n");

    TEST_SUMMARY(posix_test_ctx);
    TEST_EXIT(posix_test_ctx.failed > 0 ? 1 : 0);

    return NULL;
}

/* Register POSIX test application with user runtime system */
DECLARE_USER(0,
             posix_tests,
             posix_test_main,
             DECLARE_FPAGE(0x0,
                           8192) /* Resource pool: stack + UTCB for threads */
             DECLARE_FPAGE(0x0, 2048)); /* Heap for thread management */
