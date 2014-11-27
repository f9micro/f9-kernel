/* Copyright (c) 2002, 2003, 2007, 2010 Karlsruhe University.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/ipc.h>
#include <l4/space.h>
#include <l4/thread.h>
#include <l4/arch.h>
#include <l4/kdebug.h>
#include <l4/pager.h>
#include <l4io.h>
#include <platform/cortex_m.h>

#include "arch.h"
#include "config.h"
#include "l4test.h"
#include "assert.h"

__USER_BSS L4_ThreadId_t ipc_t1;
__USER_BSS L4_ThreadId_t ipc_t2;
__USER_BSS bool ipc_ok;

__USER_BSS L4_Word_t ipc_pf_block_address = 0;
__USER_BSS L4_Word_t ipc_pf_abort_address = 0;

__USER_TEXT
L4_Word_t Word(L4_ThreadId_t t)
{
	return t.raw;
}

__USER_TEXT
const char *ipc_errorcode(L4_Word_t errcode)
{
	switch ((errcode >> 1) & 0x7) {
	case 0:
		return "ok (0)";
	case 1:
		return "timeout";
	case 2:
		return "non-existing partner";
	case 3:
		return "cancelled";
	case 4:
		return "message overflow";
	case 5:
		return "xfer timeout (current)";
	case 6:
		return "xfer timeout (partner)";
	case 7:
		return "aborted";
	}
	return "unknown";
}

__USER_TEXT
const char *ipc_errorphase(L4_Word_t errcode)
{
	return errcode & 0x1 ? "receive-phase" : "send-phase";
}


__USER_TEXT
void setup_ipc_threads(void *(*f1)(void *), void *(*f2)(void *),
                       bool rcv_same_space, bool snd_same_space,
                       bool xcpu)
{
	ipc_t1 = pager_create_thread();
	ipc_t2 = pager_create_thread();
	pager_start_thread(ipc_t1, f1, NULL);
	pager_start_thread(ipc_t2, f2, NULL);
}

/*
 ** IPC test with only untyped words
 */
__USER_TEXT
static void *simple_ipc_t1_l(void *arg)
{
	L4_Msg_t msg;
	L4_MsgTag_t tag;
	L4_Word_t i;
	L4_ThreadId_t tid;
	L4_ThreadId_t from;

	/* Correct message contents */
	ipc_ok = true;

	for (L4_Word_t n = 0; n < L4_NumMRs(); n++) {
		for (L4_Word_t k = 1; k < L4_NumMRs(); k++)
			L4_LoadMR(k, 0);
		tag = L4_Receive(ipc_t2);
		if (!L4_IpcSucceeded(tag)) {
			printf("Xfer %d words -- IPC failed %s %s\n",
			       (int) n,
			       ipc_errorcode(L4_ErrorCode()),
			       ipc_errorphase(L4_ErrorCode()));
			ipc_ok = false;
			break;
		}

		L4_MsgStore(tag, &msg);
		if (L4_Label(tag) != 0xf00f) {
			printf("Xfer %d words -- wrong label: 0x%lx != 0xf00f\n",
			       (int) n, (long) L4_Label(tag));
			ipc_ok = false;
			break;
		}

		for (i = 1; i <= n; i++) {
			L4_Word_t val = L4_MsgWord(&msg, i - 1);
			if (val != i) {
				printf("Xfer %d words -- wrong value in MR[%d]: "
				       "0x%lx != 0x%lx\n",
				       (int) n, (int) i, (long) val, (long) i);
				ipc_ok = false;
			}
			if (!ipc_ok)
				break;
		}
		if (!ipc_ok)
			break;
	}

	print_result("Send Message transfer", ipc_ok);

	/* Correct message contents */
	ipc_ok = true;
	tag = L4_Call(ipc_t2);

	for (L4_Word_t n = 0; n < L4_NumMRs(); n++) {
		L4_MsgStore(tag, &msg);
		if (L4_Label(tag) != 0xf00d) {
			L4_KDB_Enter("2 label");
			printf("Xfer %d words -- wrong label: 0x%lx != 0xf00d\n",
			       (int) n, (long) L4_Label(tag));
			ipc_ok = false;
			break;
		}
		for (i = 1; i <= n; i++) {
			L4_Word_t val = L4_MsgWord(&msg, i - 1);
			if (val != i) {
				printf("Xfer %d words -- wrong value in MR[%d]: "
				       "0x%lx != 0x%lx\n",
				       (int) n, (int) i, (long) val, (long) i);
				ipc_ok = false;
				break;
			}
			if (!ipc_ok)
				break;

		}

		if (n == L4_NumMRs() - 1)
			break;

		for (L4_Word_t k = 1; k < L4_NumMRs(); k++)
			L4_LoadMR(k, 0);
		tag = L4_ReplyWait(ipc_t2, &tid);

		if (!L4_IpcSucceeded(tag)) {
			printf("Xfer %d words -- IPC failed %s %s\n",
			       (int) n,
			       ipc_errorcode(L4_ErrorCode()),
			       ipc_errorphase(L4_ErrorCode()));
			ipc_ok = false;
			break;
		}

	}

	print_result("ReplyWait Message transfer", ipc_ok);
	/* From parameter (local) */
	tag = L4_Wait(&from);
	ipc_ok = true;

	if (from.raw != L4_LocalIdOf(ipc_t2).raw) {
		printf("Returned Id %lx != %lx (local) [%lx (global)]\n",
		       Word(from), Word(L4_LocalIdOf(ipc_t2)), Word(ipc_t2));
		ipc_ok = false;
	}
	print_result("From parameter (local)", ipc_ok);
	L4_Set_MsgTag(L4_Niltag);
	L4_Send(ipc_t2);

	return NULL;
}

__USER_TEXT
static void *simple_ipc_t2_l(void *arg)
{
	L4_Msg_t msg;
	L4_ThreadId_t dt;
	L4_MsgTag_t tag;

	/* Message contents */
	for (L4_Word_t n = 0; n < L4_NumMRs(); n++) {
		L4_MsgClear(&msg);
		L4_Set_Label(&msg.tag, 0xf00f);
		for (L4_Word_t i = 1; i <= n; i++)
			L4_MsgAppendWord(&msg, i);
		L4_MsgLoad(&msg);
		tag = L4_Send(ipc_t1);
		if (!L4_IpcSucceeded(tag)) {
			printf("Xfer %d words -- IPC failed %s %s\n",
			       (int) n,
			       ipc_errorcode(L4_ErrorCode()),
			       ipc_errorphase(L4_ErrorCode()));
			ipc_ok = false;
			break;
		}
	}

	L4_Receive(ipc_t1);

	/* Message contents */
	for (L4_Word_t n = 0; n < L4_NumMRs(); n++) {
		L4_MsgClear(&msg);
		L4_Set_Label(&msg.tag, 0xf00d);
		for (L4_Word_t i = 1; i <= n; i++)
			L4_MsgAppendWord(&msg, i);
		L4_MsgLoad(&msg);
		if (n == L4_NumMRs() - 1)
			tag = L4_Send(ipc_t1);
		else
			tag = L4_ReplyWait(ipc_t1, &dt);

		if (!L4_IpcSucceeded(tag)) {
			printf("Xfer %d words -- IPC failed %s %s\n", (int) n,
			       ipc_errorcode(L4_ErrorCode()),
			       ipc_errorphase(L4_ErrorCode()));
			ipc_ok = false;
			break;
		}

	}

	// From parameter (local)
	L4_Set_MsgTag(L4_Niltag);
	L4_Send(ipc_t1);
	L4_Receive(ipc_t1);

	return NULL;
}

__USER_TEXT
static void simple_ipc(void)
{
	printf("\nSimple IPC test (inter-as, only untyped words)\n");
	setup_ipc_threads(simple_ipc_t1_l, simple_ipc_t2_l, true, true, false);
}

__USER_TEXT
void all_ipc_tests(void)
{
	simple_ipc();
}
