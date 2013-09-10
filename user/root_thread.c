/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/thread.h>
#include <l4/kip.h>
#include <l4/utcb.h>
#include <l4/ipc.h>
#include <types.h>

#define L4_THREAD_NUM(n, b) \
	((L4_ThreadId_t)((b + n) << 14))

enum { PING_THREAD, PONG_THREAD };

L4_Word_t *__L4_UtcbPtr __USER_BSS;

static L4_ThreadId_t threads[2] __USER_BSS;

int __USER_TEXT L4_Map(L4_ThreadId_t where, memptr_t base, size_t size)
{
	L4_Msg_t msg;
	L4_Fpage_t fpage = L4_Fpage(base, size);
	L4_GrantItem_t map = L4_GrantItem(fpage, base);

	L4_MsgClear(&msg);
	L4_MsgAppendGrantItem(&msg, map);

	L4_MsgLoad(&msg);

	L4_Send(where);
	return 0;
}

void __USER_TEXT __ping_thread(void *kip_ptr, void *utcb_ptr)
{
	L4_Msg_t msg;

	L4_MsgClear(&msg);
	L4_MsgLoad(&msg);

	while (1)
		L4_Send(threads[PONG_THREAD]);
}

void __USER_TEXT __pong_thread(void *kip_ptr, void *utcb_ptr)
{
	L4_MsgTag_t msgtag;
	L4_Msg_t msg;

	while (1) {
		msgtag = L4_Receive(threads[PING_THREAD]);
		L4_MsgStore(msgtag, &msg);
	}
}

DECLARE_THREAD(ping_thread, __ping_thread);
DECLARE_THREAD(pong_thread, __pong_thread);

memptr_t __USER_TEXT get_free_base(kip_t *kip_ptr)
{
	kip_mem_desc_t *desc = ((void *) kip_ptr) +
			kip_ptr->memory_info.s.memory_desc_ptr;
	int n = kip_ptr->memory_info.s.n;
	int i = 0;

	for (i = 0; i < n; ++i) {
		if ((desc[i].size & 0x3F) == 4)
			return desc[i].base & 0xFFFFFFC0;
	}

	return 0;
}

void __USER_TEXT start_thread(L4_ThreadId_t t, L4_Word_t sp, L4_Word_t ip)
{
	L4_Msg_t msg;

	L4_MsgClear(&msg);
	L4_MsgAppendWord(&msg, ip);
	L4_MsgAppendWord(&msg, sp);
	L4_MsgLoad(&msg);

	L4_Send(t);
}

#define STACK_SIZE 256

void __USER_TEXT __root_thread(kip_t *kip_ptr, utcb_t *utcb_ptr)
{
	L4_ThreadId_t myself = {.raw = utcb_ptr->t_globalid};
	char *free_mem = (char *) get_free_base(kip_ptr);

	/* Allocate utcbs and stacks in Free memory region */
	char *utcbs[2] = {
		free_mem, free_mem + UTCB_SIZE
	};
	char *stacks[2] = {
		free_mem + 2 * UTCB_SIZE,
		free_mem + 2 * UTCB_SIZE + STACK_SIZE
	};

	threads[PING_THREAD] = L4_THREAD_NUM(PING_THREAD,
			kip_ptr->thread_info.s.user_base);	/* Ping */
	threads[PONG_THREAD] = L4_THREAD_NUM(PONG_THREAD,
			kip_ptr->thread_info.s.user_base);	/* Pong */

	L4_ThreadControl(threads[PING_THREAD], myself,
			(L4_ThreadId_t){.raw = 0}, myself, utcbs[PING_THREAD]);
	L4_ThreadControl(threads[PONG_THREAD], myself,
			(L4_ThreadId_t){.raw = 0}, myself, utcbs[PONG_THREAD]);

	L4_Map(myself, (memptr_t) stacks[PING_THREAD], STACK_SIZE);
	L4_Map(myself, (memptr_t) stacks[PONG_THREAD], STACK_SIZE);

	start_thread(threads[PING_THREAD], (L4_Word32_t)ping_thread,
			(L4_Word32_t)stacks[PING_THREAD] + STACK_SIZE);
	start_thread(threads[PONG_THREAD], (L4_Word32_t)pong_thread,
			(L4_Word32_t)stacks[PONG_THREAD] + STACK_SIZE);

	while (1)
		L4_Sleep(L4_Never);
}

DECLARE_THREAD(root_thread, __root_thread);
