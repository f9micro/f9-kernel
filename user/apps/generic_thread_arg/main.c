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

enum { PING_THREAD = 0, PONG_THREAD, PUNG_THREAD, BOSS_THREAD, THREAD_COUNT};
static L4_ThreadId_t threads[THREAD_COUNT] __USER_DATA;

#define LABEL 0x1

typedef struct thread_initialization_s {
	L4_Word_t this_thread;
	L4_Word_t prev_thread_id;
	L4_Word_t next_thread_id;
} thread_initialization_t;

static thread_initialization_t thread_initialization[THREAD_COUNT] __USER_DATA;


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

	L4_Word_t my_thread_id = 0;
	L4_Word_t prev_thread_id = 0;
	L4_Word_t next_thread_id = 0;

	printf("\nping_thread %p\n", L4_MyGlobalId());

	thread_initialization_t *thread_initialization_p = (thread_initialization_t *)arg;
	my_thread_id = thread_initialization_p->this_thread;
	prev_thread_id = thread_initialization_p->prev_thread_id;
	next_thread_id = thread_initialization_p->next_thread_id;

	printf("\nping_thread %p %d is initialized : prev_thread_id %d, next_thread_id %d\n", L4_MyGlobalId(), my_thread_id, prev_thread_id, next_thread_id);
	
	/* WARNING !!! Cannot do this! Apparently, a message cannot be sent to a thread that is not already waiting for it. */
	// L4_Sleep(L4_TimePeriod(500 * 1000));

	while (1) {
		tag = L4_Receive_Timeout(threads[prev_thread_id], L4_TimePeriod(1000 * 1000));

		if (!L4_IpcSucceeded(tag)) {
			printf("\nping_thread - %p, %d: recv ipc fails\n", L4_MyGlobalId(), my_thread_id);
			printf("ping_thread - %p: ErrorCode = 0x%x %s\n", L4_MyGlobalId(), L4_ErrorCode(), L4_ErrorCode_String(L4_ErrorCode()));
			// If there is no message for us then back to the top of this loop to wait again for a message.
			continue;
		}
		else {
			// Cannot call L4_MsgStore(tag, &msg) or L4_MsgWord(&msg, 0) 
			// in the event of an IPC receive timeout condition.
			L4_MsgStore(tag, &msg);
			count = L4_MsgWord(&msg, 0);
			prev_thread_id = L4_MsgWord(&msg, 1);
		}

		/* FIXME: workaround solution to avoid scheduler starvation */
		L4_Sleep(L4_TimePeriod(500 * 1000));

		printf("\nThread # %d received count %d from thread %d, next %d\n", my_thread_id, count, prev_thread_id, next_thread_id);

		L4_MsgClear(&msg);
		L4_Set_MsgLabel(&msg, LABEL);
		L4_MsgAppendWord(&msg, ++count);
		L4_MsgAppendWord(&msg, my_thread_id); // prev_thread_id for next thread in the sequence.
		L4_MsgAppendWord(&msg, next_thread_id);
		L4_MsgLoad(&msg);

		tag = L4_Send_Timeout(threads[next_thread_id],
		                      L4_TimePeriod(1000 * 1000));

		if (!L4_IpcSucceeded(tag)) {
			printf("\nping_thread - %p: send ipc fails\n", L4_MyGlobalId());
			printf("ping_thread - %p: ErrorCode = 0x%x %s\n", L4_MyGlobalId(), L4_ErrorCode(), L4_ErrorCode_String(L4_ErrorCode()));
		}
		/* FIXME: workaround solution to avoid scheduler starvation */
		L4_Sleep(L4_TimePeriod(500 * 1000));
	}
}

__USER_TEXT
void *boss_thread(void *arg)
{
	L4_MsgTag_t tag;
	L4_Msg_t msg;

	L4_Word_t this_thread;

	printf("\nboss_thread() %p is running\n", L4_MyGlobalId());

	// Initialize each thread state.
	L4_Word_t prev_thread_id = BOSS_THREAD;
	L4_Word_t next_thread_id = PONG_THREAD;
	for (this_thread = PING_THREAD; this_thread < BOSS_THREAD; this_thread++) {

		printf("\nInitialize thread state %d %d %d\n", prev_thread_id, this_thread, next_thread_id);
		thread_initialization[this_thread].this_thread = this_thread;
		thread_initialization[this_thread].prev_thread_id = prev_thread_id;
		thread_initialization[this_thread].next_thread_id = next_thread_id;

		printf("\nCreate thread %d\n", this_thread);
		threads[this_thread] = pager_create_thread();
		printf("\nStart thread %d\n", this_thread);
		pager_start_thread(threads[this_thread], ping_thread, &thread_initialization[this_thread]);

		prev_thread_id = ((++prev_thread_id >= BOSS_THREAD) ? PING_THREAD : prev_thread_id);
		next_thread_id = ((++next_thread_id == BOSS_THREAD) ? PING_THREAD : next_thread_id);
	}

	// Prime the pump - send a message to kick-off the round-robin pinging among the threads.
	{
		L4_Word_t count = 0;

		printf("\nPrime the pump %d\n", count);

		L4_MsgClear(&msg);
		L4_Set_MsgLabel(&msg, LABEL);
		L4_MsgAppendWord(&msg, count);
		L4_MsgAppendWord(&msg, PUNG_THREAD);
		L4_MsgLoad(&msg);

		tag = L4_Send_Timeout(threads[PING_THREAD], L4_TimePeriod(1000 * 1000));

		if (!L4_IpcSucceeded(tag)) {
			printf("\nboss_thread() %p: send ipc fails\n", L4_MyGlobalId());
			printf("boss_thread() %p: ErrorCode = 0x%x %s\n", L4_MyGlobalId(), L4_ErrorCode(), L4_ErrorCode_String(L4_ErrorCode()));
		}
		else {
			printf("\nPump is primed %d\n", count);
		}
	}

	printf("\nEXITING boss_thread()\n");
	return 0;
}

__USER_TEXT
static void *main(void *user)
{
	printf("\nENTERING main()\n");

	threads[BOSS_THREAD] = pager_create_thread();
	pager_start_thread(threads[BOSS_THREAD], boss_thread, NULL);

	printf("\nEXITING main()\n");
	return 0;
}

// DECLARE_FPAGE(0x0, (number_of_threads * 2 * UTCB_SIZE) + (number_of_threads * 2 * STACK_SIZE))
DECLARE_USER(
	0,
	generic_thread,
	main,
	DECLARE_FPAGE(0x0, 8 * UTCB_SIZE + 8 * STACK_SIZE)
	DECLARE_FPAGE(0x0, 512)
);
