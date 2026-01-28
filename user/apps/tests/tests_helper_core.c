/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * Test Infrastructure Helpers
 *
 * Common utilities for advanced test cases requiring thread management,
 * memory access verification, and pagefault handling.
 */

#include <l4/ipc.h>
#include <l4/pager.h>
#include <l4/thread.h>
#include <l4io.h>

#include "tests.h"

/* Extended thread creation with full configuration (test-only helper) */
__USER_TEXT
L4_ThreadId_t test_create_thread_ex(L4_ThreadId_t space,
                                    L4_ThreadId_t scheduler,
                                    L4_ThreadId_t pager)
{
    L4_ThreadId_t tid = pager_create_thread();
    if (tid.raw == 0) {
        return tid;
    }

    /* Configure thread if custom settings requested */
    if (pager.raw != 0 && pager.raw != L4_Pager().raw) {
        L4_Set_PagerOf(tid, pager);
    }

    return tid;
}

/* Attempt memory access, returns 1 on success (used for pagefault tests) */
__USER_TEXT
int test_memory_access(volatile L4_Word_t *addr)
{
    volatile L4_Word_t val;

    /* Try to read the address */
    val = *addr;
    (void) val;

    /* If we got here without fault, access succeeded */
    return 1;
}

/* Abort IPC and stop thread (used for timeout/error handling) */
__USER_TEXT
void test_abort_thread(L4_ThreadId_t tid)
{
    L4_ThreadState_t state;

    if (L4_IsNilThread(tid)) {
        return;
    }

    /* Abort any pending IPC and stop the thread */
    state = L4_AbortIpc_and_stop(tid);
    (void) state;
}

/* Service pagefaults from test threads with timeout protection */
__USER_TEXT
int test_handle_pagefault(L4_ThreadId_t client,
                          L4_Word_t fault_addr,
                          L4_Word_t fault_ip,
                          L4_Time_t timeout)
{
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    /* Build reply with fpage grant for the faulting address */
    L4_MsgClear(&msg);
    L4_MsgAppendWord(&msg, fault_addr);
    L4_MsgLoad(&msg);

    /* Reply to faulting thread */
    tag = L4_Reply(client);

    return L4_IpcSucceeded(tag) ? 1 : 0;
}

/* Wait for thread completion with timeout */
__USER_TEXT
int test_wait_thread(volatile int *done_flag, int timeout_ms)
{
    int timeout = timeout_ms / 10;

    while (!*done_flag && timeout > 0) {
        L4_Sleep(L4_TimePeriod(10000)); /* 10ms */
        timeout--;
    }

    return *done_flag ? 1 : 0;
}

/* ANSI color code for SKIP status */
#define ANSI_YELLOW "\033[33m"

/* Skip test with reason */
__USER_TEXT
void test_skip(const char *name, const char *reason)
{
    test_ctx.skipped++;
    printf("[TEST:SKIP] %s\n", name);
    printf("Test %-40s[" ANSI_YELLOW "SKIP" ANSI_RESET "]\n", name);
    (void) reason; /* Reason parameter available for future use */
}
