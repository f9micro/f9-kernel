/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/link.h>
#include <user_runtime.h>
#include <l4/ipc.h>
#include <l4/utcb.h>
#include <l4/pager.h>
#include <l4/thread.h>
#include <l4io.h>

#define STACK_SIZE 512

enum { PING_THREAD, PONG_THREAD };

static L4_ThreadId_t threads[2] __USER_DATA;

__USER_TEXT
void *ping_thread(void *arg)
{
	L4_Msg_t msg;
	L4_MsgTag_t tag;

	L4_MsgClear(&msg);
	L4_MsgLoad(&msg);

	while (1) {
		tag = L4_Send_Timeout(threads[PONG_THREAD],
		                      L4_TimePeriod(1000 * 1000));

		if (!L4_IpcSucceeded(tag)) {
			printf("%p: send ipc fails\n", L4_MyGlobalId());
			printf("%p: ErrorCode = 0x%x\n", L4_MyGlobalId(), L4_ErrorCode());
		}
	}
}

__USER_TEXT
void *pong_thread(void *arg)
{
	L4_MsgTag_t tag;
	L4_Msg_t msg;

	while (1) {
		tag = L4_Receive_Timeout(threads[PING_THREAD],
		                         L4_TimePeriod(1000 * 1000));
		L4_MsgStore(tag, &msg);

		if (!L4_IpcSucceeded(tag)) {
			printf("%p: recv ipc fails\n", L4_MyGlobalId());
			printf("%p: ErrorCode = 0x%x\n", L4_MyGlobalId(), L4_ErrorCode());
		}
		/* FIXME: workaround solution to avoid scheduler starvation */
		L4_Sleep(L4_TimePeriod(500 * 1000));
	}
}

__USER_TEXT
static void *main(void *user)
{
	threads[PING_THREAD] = pager_create_thread();
	threads[PONG_THREAD] = pager_create_thread();
	pager_start_thread(threads[PING_THREAD], ping_thread, NULL);
	pager_start_thread(threads[PONG_THREAD], pong_thread, NULL);
	return 0;
}

DECLARE_USER(
	0,
	pingpong,
	main,
	DECLARE_FPAGE(0x0, 4 * UTCB_SIZE + 4 * STACK_SIZE)
	DECLARE_FPAGE(0x0, 512)
);
