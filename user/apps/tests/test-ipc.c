/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/ipc.h>
#include <l4/pager.h>
#include <l4/thread.h>
#include <l4io.h>

#include "tests.h"

/* Shared state for IPC test */
__USER_BSS static L4_ThreadId_t sender_tid;
__USER_BSS static L4_ThreadId_t receiver_tid;
__USER_BSS static volatile L4_Word_t received_value;
__USER_BSS static volatile L4_Word_t received_label;
__USER_BSS static volatile L4_Word_t received_from;
__USER_BSS static volatile int receiver_ready;
__USER_BSS static volatile int receiver_done;

/* Magic values for IPC verification */
#define IPC_MAGIC_VALUE 0xCAFEBABE
#define IPC_MAGIC_LABEL 0x1234

/*
 * Receiver thread: waits for message from sender.
 * Validates sender TID and message label.
 */
__USER_TEXT
static void *receiver_thread(void *arg)
{
    L4_MsgTag_t tag;
    L4_Msg_t msg;
    L4_ThreadId_t from;

    /* Signal ready before blocking on receive */
    receiver_ready = 1;

    /* Wait for message from sender (with timeout to avoid blocking forever) */
    tag = L4_Wait_Timeout(L4_TimePeriod(500000), &from);

    if (L4_IpcSucceeded(tag)) {
        L4_MsgStore(tag, &msg);
        received_value = L4_MsgWord(&msg, 0);
        received_label = L4_Label(tag);
        received_from = from.raw;
    } else {
        received_value = 0;
        received_label = 0;
        received_from = 0;
    }

    receiver_done = 1;
    return NULL;
}

/*
 * Sender thread: sends message to receiver.
 * Uses handshake instead of fixed delay.
 */
__USER_TEXT
static void *sender_thread(void *arg)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    int timeout;

    /* Wait for receiver to be ready (handshake) */
    timeout = 100;
    while (!receiver_ready && timeout > 0) {
        L4_Sleep(L4_TimePeriod(1000)); /* 1ms */
        timeout--;
    }

    if (!receiver_ready) {
        printf("Sender: receiver not ready\n");
        return NULL;
    }

    /* Send magic value to receiver */
    L4_MsgClear(&msg);
    L4_Set_Label(&msg.tag, IPC_MAGIC_LABEL);
    L4_MsgAppendWord(&msg, IPC_MAGIC_VALUE);
    L4_MsgLoad(&msg);

    /* Use timeout to avoid blocking forever */
    tag = L4_Send_Timeout(receiver_tid, L4_TimePeriod(100000));

    if (!L4_IpcSucceeded(tag)) {
        printf("IPC send failed\n");
    }

    return NULL;
}

/*
 * Basic IPC test: sender sends a message, receiver receives it.
 * Validates: sender TID, message label, message content.
 */
__USER_TEXT
void test_ipc_basic(void)
{
    int timeout;
    int ok;

    TEST_RUN("ipc_basic");

    /* Initialize shared state */
    received_value = 0;
    received_label = 0;
    received_from = 0;
    receiver_ready = 0;
    receiver_done = 0;

    /* Create receiver thread first (so it's ready to receive) */
    receiver_tid = pager_create_thread();
    if (receiver_tid.raw == 0) {
        printf("Failed to create receiver thread\n");
        TEST_FAIL("ipc_basic");
        return;
    }

    /* Create sender thread */
    sender_tid = pager_create_thread();
    if (sender_tid.raw == 0) {
        printf("Failed to create sender thread\n");
        TEST_FAIL("ipc_basic");
        return;
    }

    /* Start receiver first, then sender */
    pager_start_thread(receiver_tid, receiver_thread, NULL);
    pager_start_thread(sender_tid, sender_thread, NULL);

    /* Wait for receiver to complete (simple polling with timeout) */
    timeout = 100; /* ~1 second with 10ms sleeps */
    while (!receiver_done && timeout > 0) {
        L4_Sleep(L4_TimePeriod(10000)); /* 10ms */
        timeout--;
    }

    /* Verify: value, label, and sender TID */
    ok = receiver_done && received_value == IPC_MAGIC_VALUE &&
         received_label == IPC_MAGIC_LABEL && received_from == sender_tid.raw;

    if (ok) {
        TEST_PASS("ipc_basic");
    } else {
        printf("IPC failed: val=0x%lx label=0x%lx from=0x%lx\n",
               (unsigned long) received_value, (unsigned long) received_label,
               (unsigned long) received_from);
        printf("  expected: val=0x%lx label=0x%lx from=0x%lx\n",
               (unsigned long) IPC_MAGIC_VALUE, (unsigned long) IPC_MAGIC_LABEL,
               (unsigned long) sender_tid.raw);
        TEST_FAIL("ipc_basic");
    }
}

/* Multi-word IPC test state */
#define IPC_MULTI_LABEL 0x5678
__USER_BSS static volatile L4_Word_t multi_word_count;
__USER_BSS static volatile L4_Word_t multi_words[4];
__USER_BSS static volatile L4_Word_t multi_label;
__USER_BSS static volatile L4_Word_t multi_from;
__USER_BSS static volatile int multi_receiver_ready;
__USER_BSS static volatile int multi_receiver_done;
__USER_BSS static L4_ThreadId_t multi_sender_tid;
__USER_BSS static L4_ThreadId_t multi_receiver_tid;

/*
 * Receiver for multi-word test.
 * Validates sender TID and message label.
 */
__USER_TEXT
static void *multi_receiver_thread(void *arg)
{
    L4_MsgTag_t tag;
    L4_Msg_t msg;
    L4_ThreadId_t from;
    L4_Word_t i;

    /* Signal ready before blocking */
    multi_receiver_ready = 1;

    /* Use timeout to avoid blocking forever */
    tag = L4_Wait_Timeout(L4_TimePeriod(500000), &from);

    if (L4_IpcSucceeded(tag)) {
        L4_MsgStore(tag, &msg);
        multi_word_count = L4_UntypedWords(tag);
        multi_label = L4_Label(tag);
        multi_from = from.raw;
        for (i = 0; i < multi_word_count && i < 4; i++) {
            multi_words[i] = L4_MsgWord(&msg, i);
        }
    } else {
        multi_word_count = 0;
        multi_label = 0;
        multi_from = 0;
    }

    multi_receiver_done = 1;
    return NULL;
}

/*
 * Sender for multi-word test.
 * Uses handshake instead of fixed delay.
 */
__USER_TEXT
static void *multi_sender_thread(void *arg)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    int timeout;

    /* Wait for receiver to be ready (handshake) */
    timeout = 100;
    while (!multi_receiver_ready && timeout > 0) {
        L4_Sleep(L4_TimePeriod(1000)); /* 1ms */
        timeout--;
    }

    if (!multi_receiver_ready) {
        /* Receiver not ready, abort */
        return NULL;
    }

    /* Send 4 words */
    L4_MsgClear(&msg);
    L4_Set_Label(&msg.tag, IPC_MULTI_LABEL);
    L4_MsgAppendWord(&msg, 0x11111111);
    L4_MsgAppendWord(&msg, 0x22222222);
    L4_MsgAppendWord(&msg, 0x33333333);
    L4_MsgAppendWord(&msg, 0x44444444);
    L4_MsgLoad(&msg);

    /* Use timeout to avoid blocking forever */
    tag = L4_Send_Timeout(multi_receiver_tid, L4_TimePeriod(100000));
    (void) tag;

    return NULL;
}

/*
 * Test: IPC with multiple words.
 * Validates: word count, all values, label, sender TID.
 */
__USER_TEXT
void test_ipc_multiword(void)
{
    L4_ThreadId_t mw_receiver, mw_sender;
    int timeout;
    int ok;

    TEST_RUN("ipc_multiword");

    multi_word_count = 0;
    multi_words[0] = multi_words[1] = 0;
    multi_words[2] = multi_words[3] = 0;
    multi_label = 0;
    multi_from = 0;
    multi_receiver_ready = 0;
    multi_receiver_done = 0;

    /* Create threads */
    mw_receiver = pager_create_thread();
    if (mw_receiver.raw == 0) {
        printf("Failed to create multi-word receiver\n");
        TEST_FAIL("ipc_multiword");
        return;
    }

    mw_sender = pager_create_thread();
    if (mw_sender.raw == 0) {
        printf("Failed to create multi-word sender\n");
        TEST_FAIL("ipc_multiword");
        return;
    }

    /* Store TIDs for threads to use (separate from basic IPC test) */
    multi_receiver_tid = mw_receiver;
    multi_sender_tid = mw_sender;

    pager_start_thread(mw_receiver, multi_receiver_thread, NULL);
    pager_start_thread(mw_sender, multi_sender_thread, NULL);

    timeout = 50;
    while (!multi_receiver_done && timeout > 0) {
        L4_Sleep(L4_TimePeriod(10000));
        timeout--;
    }

    /* Verify all 4 words, label, and sender TID */
    ok = multi_receiver_done && multi_word_count >= 4 &&
         multi_words[0] == 0x11111111 && multi_words[1] == 0x22222222 &&
         multi_words[2] == 0x33333333 && multi_words[3] == 0x44444444 &&
         multi_label == IPC_MULTI_LABEL && multi_from == mw_sender.raw;

    if (ok) {
        TEST_PASS("ipc_multiword");
    } else {
        printf("Multi-word IPC failed: count=%lu label=0x%lx\n",
               (unsigned long) multi_word_count, (unsigned long) multi_label);
        TEST_FAIL("ipc_multiword");
    }
}
