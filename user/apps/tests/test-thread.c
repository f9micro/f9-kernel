/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/ipc.h>
#include <l4/pager.h>
#include <l4/thread.h>
#include <l4/types.h>
#include <l4io.h>

#include "tests.h"

/* Shared state for thread tests */
__USER_BSS static volatile int worker_executed;
__USER_BSS static volatile L4_Word_t worker_tid_raw;

/*
 * Worker thread: records its TID and signals completion.
 */
__USER_TEXT
static void *worker_thread(void *arg)
{
    /* Record our thread ID */
    worker_tid_raw = L4_Myself().raw;
    worker_executed = 1;
    return NULL;
}

/*
 * Test: Verify L4_Myself() returns valid thread ID.
 */
__USER_TEXT
void test_thread_self(void)
{
    L4_ThreadId_t my_tid;

    TEST_RUN("thread_self");

    my_tid = L4_Myself();

    /* Thread ID should be non-zero and not nilthread */
    if (!L4_IsNilThread(my_tid)) {
        TEST_PASS("thread_self");
    } else {
        printf("L4_Myself() returned invalid TID: 0x%lx\n",
               (unsigned long) my_tid.raw);
        TEST_FAIL("thread_self");
    }
}

/*
 * Test: Verify L4_MyGlobalId() matches L4_Myself().
 */
__USER_TEXT
void test_thread_global_id(void)
{
    L4_ThreadId_t my_tid, global_tid;

    TEST_RUN("thread_global_id");

    my_tid = L4_Myself();
    global_tid = L4_MyGlobalId();

    if (my_tid.raw == global_tid.raw) {
        TEST_PASS("thread_global_id");
    } else {
        printf("L4_Myself()=0x%lx != L4_MyGlobalId()=0x%lx\n",
               (unsigned long) my_tid.raw, (unsigned long) global_tid.raw);
        TEST_FAIL("thread_global_id");
    }
}

/*
 * Test: Verify L4_Pager() returns valid pager thread.
 */
__USER_TEXT
void test_thread_pager(void)
{
    L4_ThreadId_t pager_tid;

    TEST_RUN("thread_pager");

    pager_tid = L4_Pager();

    /* Pager should be a valid thread (non-zero) */
    if (pager_tid.raw != 0) {
        TEST_PASS("thread_pager");
    } else {
        printf("L4_Pager() returned nil thread\n");
        TEST_FAIL("thread_pager");
    }
}

/*
 * Test: Create and start a worker thread.
 */
__USER_TEXT
void test_thread_create(void)
{
    L4_ThreadId_t worker_tid;
    int timeout;

    TEST_RUN("thread_create");

    worker_executed = 0;
    worker_tid_raw = 0;

    /* Create worker thread via pager */
    worker_tid = pager_create_thread();
    if (worker_tid.raw == 0) {
        printf("Failed to create worker thread\n");
        TEST_FAIL("thread_create");
        return;
    }

    /* Start the worker */
    pager_start_thread(worker_tid, worker_thread, NULL);

    /* Wait for worker to execute */
    timeout = 50;
    while (!worker_executed && timeout > 0) {
        L4_Sleep(L4_TimePeriod(10000)); /* 10ms */
        timeout--;
    }

    /* Verify worker executed and TID matches what we created */
    if (worker_executed && worker_tid_raw == worker_tid.raw) {
        TEST_PASS("thread_create");
    } else {
        printf("Worker TID mismatch: created=0x%lx reported=0x%lx\n",
               (unsigned long) worker_tid.raw, (unsigned long) worker_tid_raw);
        TEST_FAIL("thread_create");
    }
}
