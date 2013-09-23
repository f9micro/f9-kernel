/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/link.h>
#include <app.h>
#include <l4/ipc.h>
#include <l4/utcb.h>

#define STACK_SIZE 256

enum { PING_THREAD, PONG_THREAD };

static L4_ThreadId_t threads[2] __USER_DATA;

static L4_Word_t last_thread __USER_DATA;
static L4_Word_t free_mem __USER_DATA;

void __USER_TEXT ping_thread(void)
{
	L4_Msg_t msg;

	L4_MsgClear(&msg);
	L4_MsgLoad(&msg);

	while (1)
		L4_Send(threads[PONG_THREAD]);
}

void __USER_TEXT pong_thread(void)
{
	L4_MsgTag_t msgtag;
	L4_Msg_t msg;

	while (1) {
		msgtag = L4_Receive(threads[PING_THREAD]);
		L4_MsgStore(msgtag, &msg);
	}
}

static void __USER_TEXT start_thread(L4_ThreadId_t t, L4_Word_t ip, L4_Word_t sp)
{
	L4_Msg_t msg;

	L4_MsgClear(&msg);
	L4_MsgAppendWord(&msg, ip);
	L4_MsgAppendWord(&msg, sp);
	L4_MsgLoad(&msg);

	L4_Send(t);
}

static L4_ThreadId_t __USER_TEXT create_thread(app_struct *app, void (*func)(void))
{
	L4_ThreadId_t myself = L4_MyGlobalId();
	L4_ThreadId_t child;

	child.raw = myself.raw + (++last_thread << 14);

	L4_ThreadControl(child, myself, L4_nilthread, myself, (void*)free_mem);
	free_mem += UTCB_SIZE + STACK_SIZE;

	start_thread(child, (L4_Word_t)func, free_mem);

	return child;
}

static void __USER_TEXT main(app_struct *app)
{
	free_mem = app->fpages[0].base;

	threads[PING_THREAD] = create_thread(app, ping_thread);
	threads[PONG_THREAD] = create_thread(app, pong_thread);
}

DECLARE_APP(
	0,
	pingpong,
	main,
	DECLARE_FPAGE(0x0, 2 * UTCB_SIZE + 2 *STACK_SIZE)
);
