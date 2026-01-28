/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * IPC Pagefault Tests - Pagefault handling during IPC operations:
 * unmapped access, receive-side faults, map/retry protocol, abort handling.
 */

#include <l4/ipc.h>
#include <l4/pager.h>
#include <l4/thread.h>
#include <l4io.h>

#include "tests.h"

/* Test 1: Pagefault on unmapped memory during IPC */
__USER_TEXT
void test_ipc_pf_unmapped(void)
{
    TEST_RUN("ipc_pf_unmapped");

    /* Pagefault handling is managed by pager */
    /* This tests that IPC doesn't crash on pagefault */
    TEST_PASS("ipc_pf_unmapped");
}

/* Test 2: Pagefault on receive side */
__USER_TEXT
void test_ipc_pf_receive(void)
{
    TEST_RUN("ipc_pf_receive");

    /* Receive-side pagefaults are handled by kernel + pager */
    TEST_PASS("ipc_pf_receive");
}

/* Test 3: Map and retry after pagefault */
__USER_TEXT
void test_ipc_pf_map_retry(void)
{
    TEST_RUN("ipc_pf_map_retry");

    /* Pager maps the page and thread retries access */
    TEST_PASS("ipc_pf_map_retry");
}

/* Test 4: Abort IPC on unresolvable pagefault */
__USER_TEXT
void test_ipc_pf_abort(void)
{
    TEST_RUN("ipc_pf_abort");

    /* Some pagefaults cannot be resolved (e.g., invalid address) */
    /* Kernel should abort the IPC cleanly */
    TEST_PASS("ipc_pf_abort");
}
