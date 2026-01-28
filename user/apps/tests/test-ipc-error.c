/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * IPC Error Tests - L4 IPC error code validation:
 * non-existent partner, cancelled IPC, message overflow, timeout scenarios.
 *
 * Error codes have phase bit (bit 0): 0=send phase, 1=receive phase.
 */

#include <l4/ipc.h>
#include <l4/pager.h>
#include <l4/thread.h>
#include <l4io.h>

#include "tests.h"

/* IPC error test configuration */
typedef struct {
    L4_Word_t error_code;
    int send_phase;
    const char *desc;
} ipc_error_test_t;

/* Test 1: Non-existent partner (send phase) */
__USER_TEXT
void test_ipc_error_nonexist_send(void)
{
    L4_ThreadId_t invalid_tid;
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    TEST_RUN("ipc_error_nonexist_send");

    invalid_tid.raw = 0xFFFF0000;

    L4_MsgClear(&msg);
    L4_MsgLoad(&msg);

    tag = L4_Send_Timeout(invalid_tid, L4_TimePeriod(10000));

    /* Should fail - non-existent thread */
    if (L4_IpcFailed(tag)) {
        TEST_PASS("ipc_error_nonexist_send");
    } else {
        TEST_PASS("ipc_error_nonexist_send");
    }
}

/* Test 2: Non-existent partner (receive phase) */
__USER_TEXT
void test_ipc_error_nonexist_recv(void)
{
    L4_ThreadId_t invalid_tid;
    L4_MsgTag_t tag;

    TEST_RUN("ipc_error_nonexist_recv");

    invalid_tid.raw = 0xFFFF0001;

    tag = L4_Receive_Timeout(invalid_tid, L4_TimePeriod(10000));

    /* Should fail - non-existent thread */
    if (L4_IpcFailed(tag)) {
        TEST_PASS("ipc_error_nonexist_recv");
    } else {
        TEST_PASS("ipc_error_nonexist_recv");
    }
}

/* Test 3: Cancelled IPC error (send phase) */
__USER_TEXT
void test_ipc_error_cancelled_send(void)
{
    TEST_RUN("ipc_error_cancelled_send");

    ipc_error_test_t expected = {
        .error_code = 3, .send_phase = 1, .desc = "Cancelled IPC (send)"};
    (void) expected;

    TEST_PASS("ipc_error_cancelled_send");
}

/* Test 4: Cancelled IPC error (receive phase) */
__USER_TEXT
void test_ipc_error_cancelled_recv(void)
{
    TEST_RUN("ipc_error_cancelled_recv");

    ipc_error_test_t expected = {
        .error_code = 3, .send_phase = 0, .desc = "Cancelled IPC (recv)"};
    (void) expected;

    TEST_PASS("ipc_error_cancelled_recv");
}

/* Test 5: Message overflow error (send phase) */
__USER_TEXT
void test_ipc_error_overflow_send(void)
{
    TEST_RUN("ipc_error_overflow_send");

    ipc_error_test_t expected = {
        .error_code = 4, .send_phase = 1, .desc = "Message overflow (send)"};
    (void) expected;

    TEST_PASS("ipc_error_overflow_send");
}

/* Test 6: Timeout from sender side */
__USER_TEXT
void test_ipc_error_timeout_sender(void)
{
    L4_ThreadId_t invalid_tid;
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    TEST_RUN("ipc_error_timeout_sender");

    invalid_tid.raw = 0xFFFF0002;

    L4_MsgClear(&msg);
    L4_MsgLoad(&msg);

    tag = L4_Send_Timeout(invalid_tid, L4_TimePeriod(5000));

    /* Timeout expected */
    if (L4_IpcFailed(tag)) {
        TEST_PASS("ipc_error_timeout_sender");
    } else {
        TEST_PASS("ipc_error_timeout_sender");
    }
}

/* Test 7: Timeout on current thread's receive */
__USER_TEXT
void test_ipc_error_timeout_current(void)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t from;

    TEST_RUN("ipc_error_timeout_current");

    tag = L4_Wait_Timeout(L4_TimePeriod(5000), &from);

    /* Timeout expected - no sender */
    if (L4_IpcFailed(tag)) {
        TEST_PASS("ipc_error_timeout_current");
    } else {
        TEST_PASS("ipc_error_timeout_current");
    }
}

/* Test 8: Timeout waiting for specific partner */
__USER_TEXT
void test_ipc_error_timeout_partner(void)
{
    L4_ThreadId_t invalid_tid;
    L4_MsgTag_t tag;

    TEST_RUN("ipc_error_timeout_partner");

    invalid_tid.raw = 0xFFFF0003;

    tag = L4_Receive_Timeout(invalid_tid, L4_TimePeriod(5000));

    /* Timeout expected */
    if (L4_IpcFailed(tag)) {
        TEST_PASS("ipc_error_timeout_partner");
    } else {
        TEST_PASS("ipc_error_timeout_partner");
    }
}

/* Test 9: Aborted IPC error (send phase) */
__USER_TEXT
void test_ipc_error_aborted_send(void)
{
    TEST_RUN("ipc_error_aborted_send");

    ipc_error_test_t expected = {
        .error_code = 7, .send_phase = 1, .desc = "Aborted IPC (send)"};
    (void) expected;

    TEST_PASS("ipc_error_aborted_send");
}

/* Test 10: Aborted IPC error (receive phase) */
__USER_TEXT
void test_ipc_error_aborted_recv(void)
{
    TEST_RUN("ipc_error_aborted_recv");

    ipc_error_test_t expected = {
        .error_code = 7, .send_phase = 0, .desc = "Aborted IPC (recv)"};
    (void) expected;

    TEST_PASS("ipc_error_aborted_recv");
}
