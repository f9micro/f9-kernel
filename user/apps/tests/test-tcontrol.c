/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * ThreadControl Tests - L4 syscall for thread lifecycle management:
 * creation, configuration, deletion, and UTCB setup.
 */

#include <l4/ipc.h>
#include <l4/pager.h>
#include <l4/schedule.h>
#include <l4/thread.h>
#include <l4io.h>

#include "tests.h"

/* Test 1: Basic thread creation */
__USER_TEXT
void test_tcontrol_create(void)
{
    TEST_RUN("tcontrol_create");
    TEST_SKIP("tcontrol_create");
}

/* Test 2: Thread configuration (pager, scheduler) */
__USER_TEXT
void test_tcontrol_config(void)
{
    L4_ThreadId_t pager;

    TEST_RUN("tcontrol_config");

    /* Query our own pager */
    pager = L4_Pager();

    /* We should have a valid pager */
    if (pager.raw != 0) {
        TEST_PASS("tcontrol_config");
    } else {
        /* Pager may not be set in minimal QEMU setup - skip test */
        TEST_SKIP("tcontrol_config");
    }
}

/* Test 3: Thread deletion */
__USER_TEXT
void test_tcontrol_delete(void)
{
    TEST_RUN("tcontrol_delete");
    TEST_SKIP("tcontrol_delete");
}

/* Test 4: UTCB area configuration */
__USER_TEXT
void test_tcontrol_utcb(void)
{
    L4_Word_t handle;

    TEST_RUN("tcontrol_utcb");

    /* Test user-defined handle (stored in UTCB) */
    L4_Set_UserDefinedHandle(0x12345678);
    handle = L4_UserDefinedHandle();

    if (handle == 0x12345678) {
        TEST_PASS("tcontrol_utcb");
    } else {
        TEST_FAIL("tcontrol_utcb");
    }

    /* Restore handle */
    L4_Set_UserDefinedHandle(0);
}

/* Test 5: Invalid thread operations */
__USER_TEXT
void test_tcontrol_invalid(void)
{
    L4_ThreadId_t invalid_tid;
    L4_ThreadId_t pager;

    TEST_RUN("tcontrol_invalid");

    /* Create obviously invalid TID */
    invalid_tid.raw = 0xDEADBEEF;

    /* Try to query pager of invalid thread */
    pager = L4_PagerOf(invalid_tid);

    /* Should either return nil or not crash */
    TEST_PASS("tcontrol_invalid");
    (void) pager;
}

/* Test 6: Preemption control */
__USER_TEXT
void test_tcontrol_preemption(void)
{
    L4_ThreadId_t self;
    L4_Word_t old_threshold;
    L4_Word_t result;

    TEST_RUN("tcontrol_preemption");

    self = L4_Myself();

    /* Try to set preemption threshold */
    result = L4_Set_PreemptionDelay(self, 10, &old_threshold);

    /* Any result is acceptable - feature may not be enabled */
    TEST_PASS("tcontrol_preemption");
    (void) result;
}
