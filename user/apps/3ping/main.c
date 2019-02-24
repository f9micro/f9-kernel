/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/link.h>
#include <user_runtime.h>
#include <gpioer.h>
#include <l4/ipc.h>
#include <l4/utcb.h>
#include <l4/pager.h>
#include <l4/thread.h>
#include <l4io.h>

#define STACK_SIZE 512

enum { PING_THREAD, PONG_THREAD, PUNG_THREAD };

static L4_ThreadId_t threads[3] __USER_DATA;

#define LABEL 0x1


#if 0
#define __L4_NUM_MRS    16
typedef unsigned long           L4_Word_t;
/*
 * Message objects
 */
typedef union {
        L4_Word_t raw[__L4_NUM_MRS];
        L4_Word_t msg[__L4_NUM_MRS];
                L4_MsgTag_t tag;
} L4_Msg_t;
#endif

__USER_TEXT
void *ping_thread(void *arg)
{
	L4_MsgTag_t tag;
	L4_Msg_t msg;
	L4_Word_t count = 0;

    printf("start ping_thread()\n");

	while (1) {
		tag = L4_Receive_Timeout(threads[PUNG_THREAD],
		                         L4_TimePeriod(1000 * 1000));
		L4_MsgStore(tag, &msg);
		count = L4_MsgWord(&msg, 0);

		if (!L4_IpcSucceeded(tag)) {
			printf("%p: recv ipc fails\n", L4_MyGlobalId());
			printf("%p: ErrorCode = 0x%x\n", L4_MyGlobalId(), L4_ErrorCode());
		}
		/* FIXME: workaround solution to avoid scheduler starvation */
		L4_Sleep(L4_TimePeriod(500 * 1000));

		printf("\nping_thread(%d)\t", count);
		L4_MsgClear(&msg);
		L4_Set_MsgLabel(&msg, LABEL);
		L4_MsgAppendWord(&msg, ++count);
		L4_MsgLoad(&msg);

		tag = L4_Send_Timeout(threads[PONG_THREAD],
		                      L4_TimePeriod(1000 * 1000));

		if (!L4_IpcSucceeded(tag)) {
			printf("%p: send ipc fails\n", L4_MyGlobalId());
			printf("%p: ErrorCode = 0x%x\n", L4_MyGlobalId(), L4_ErrorCode());
		}
		/* FIXME: workaround solution to avoid scheduler starvation */
		L4_Sleep(L4_TimePeriod(500 * 1000));
	}
}

__USER_TEXT
void *pong_thread(void *arg)
{
	L4_MsgTag_t tag;
	L4_Msg_t msg;
	L4_Word_t label;
	L4_Word_t count;
	L4_Word_t u;

    printf("start pong_thread()\n");

	while (1) {
		tag = L4_Receive_Timeout(threads[PING_THREAD],
		                         L4_TimePeriod(1000 * 1000));
		L4_MsgStore(tag, &msg);
		label = L4_Label(tag);
		u = L4_UntypedWords(tag);
		count = L4_MsgWord(&msg, 0);

		if (!L4_IpcSucceeded(tag)) {
			printf("%p: recv ipc fails\n", L4_MyGlobalId());
			printf("%p: ErrorCode = 0x%x\n", L4_MyGlobalId(), L4_ErrorCode());
		}
		printf("pong_thread %d : %d : %d\t", label, u, count);

		L4_MsgClear(&msg);
		L4_Set_MsgLabel(&msg, LABEL);
		L4_MsgAppendWord(&msg, ++count);
		L4_MsgLoad(&msg);

		tag = L4_Send_Timeout(threads[PUNG_THREAD],
		                      L4_TimePeriod(1000 * 1000));

		if (!L4_IpcSucceeded(tag)) {
			printf("%p: send ipc fails\n", L4_MyGlobalId());
			printf("%p: ErrorCode = 0x%x\n", L4_MyGlobalId(), L4_ErrorCode());
		}
		/* FIXME: workaround solution to avoid scheduler starvation */
		L4_Sleep(L4_TimePeriod(500 * 1000));
	}
}

__USER_TEXT
void *pung_thread(void *arg)
{
	L4_MsgTag_t tag;
	L4_Msg_t msg;
	L4_Word_t label;
	L4_Word_t count;
	L4_Word_t u;

    printf("start pung_thread()\n");

	while (1) {
		tag = L4_Receive_Timeout(threads[PONG_THREAD],
		                         L4_TimePeriod(1000 * 1000));
		L4_MsgStore(tag, &msg);
		label = L4_Label(tag);
		u = L4_UntypedWords(tag);
		count = L4_MsgWord(&msg, 0);

		if (!L4_IpcSucceeded(tag)) {
			printf("%p: recv ipc fails\n", L4_MyGlobalId());
			printf("%p: ErrorCode = 0x%x\n", L4_MyGlobalId(), L4_ErrorCode());
		}
		printf("pung_thread %d : %d : %d\n", label, u, count);

		L4_MsgClear(&msg);
		L4_Set_MsgLabel(&msg, LABEL);
		L4_MsgAppendWord(&msg, ++count);
		L4_MsgLoad(&msg);

		tag = L4_Send_Timeout(threads[PING_THREAD],
		                      L4_TimePeriod(1000 * 1000));

		if (!L4_IpcSucceeded(tag)) {
			printf("%p: send ipc fails\n", L4_MyGlobalId());
			printf("%p: ErrorCode = 0x%x\n", L4_MyGlobalId(), L4_ErrorCode());
		}
		/* FIXME: workaround solution to avoid scheduler starvation */
		L4_Sleep(L4_TimePeriod(500 * 1000));
	}
}

__USER_TEXT
static void *main(void *user)
{
	threads[PUNG_THREAD] = pager_create_thread();
	threads[PONG_THREAD] = pager_create_thread();
	threads[PING_THREAD] = pager_create_thread();

	pager_start_thread(threads[PUNG_THREAD], pung_thread, NULL);
	pager_start_thread(threads[PONG_THREAD], pong_thread, NULL);
	pager_start_thread(threads[PING_THREAD], ping_thread, NULL);

	// Prime the pump:
	{
		L4_MsgTag_t tag;
		L4_Msg_t msg;
		L4_Word_t count = 0;

		printf("\nPrime the pump %d\n", count);

		L4_MsgClear(&msg);
		L4_Set_MsgLabel(&msg, LABEL);
		L4_MsgAppendWord(&msg, count);
		L4_MsgLoad(&msg);

		tag = L4_Send_Timeout(threads[PONG_THREAD],
		                      L4_TimePeriod(1000 * 1000));

		if (!L4_IpcSucceeded(tag)) {
			printf("%p: send ipc fails\n", L4_MyGlobalId());
			printf("%p: ErrorCode = 0x%x\n", L4_MyGlobalId(), L4_ErrorCode());
		}
		else {
			printf("\nPump is primed %d\n", count);
		}
	}

	printf("\nEXITING main()\n");
	return 0;
}

// DECLARE_FPAGE(0x0, (number_of_threads * 2 * UTCB_SIZE) + (number_of_threads * 2 * STACK_SIZE))
DECLARE_USER(
	0,
	3ping,
	main,
	DECLARE_FPAGE(0x0, 6 * UTCB_SIZE + 6 * STACK_SIZE)
	DECLARE_FPAGE(0x0, 512)
);
