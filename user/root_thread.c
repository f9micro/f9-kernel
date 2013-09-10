/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/thread.h>
#include <l4/kip.h>
#include <l4/utcb.h>
#include <l4/ipc.h>
#include <types.h>

enum { PING_THREAD, PONG_THREAD };

static l4_thread_t threads[2] __USER_BSS;

void __USER_TEXT __ping_thread(void *kip_ptr, void *utcb_ptr)
{
	uint32_t msg[8] = { 0 };

	while (1)
		L4_Ipc(threads[PONG_THREAD], L4_NILTHREAD, 0, msg);
}

void __USER_TEXT __pong_thread(void *kip_ptr, void *utcb_ptr)
{
	uint32_t msg[8] = { 0 };

	while (1)
		L4_Ipc(L4_NILTHREAD, threads[PING_THREAD], 0, msg);
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

#define STACK_SIZE 256

void __USER_TEXT __root_thread(kip_t *kip_ptr, utcb_t *utcb_ptr)
{
	l4_thread_t myself = utcb_ptr->t_globalid;
	char *free_mem = (char *) get_free_base(kip_ptr);

	uint32_t msg[8] = { 0 };

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
			0, myself, utcbs[PING_THREAD]);
	L4_ThreadControl(threads[PONG_THREAD], myself,
			0, myself, utcbs[PONG_THREAD]);

	L4_Map(myself, (memptr_t) stacks[PING_THREAD], STACK_SIZE);
	L4_Map(myself, (memptr_t) stacks[PONG_THREAD], STACK_SIZE);

	L4_Start(threads[PING_THREAD], ping_thread,
			stacks[PING_THREAD] + STACK_SIZE);
	L4_Start(threads[PONG_THREAD], pong_thread,
			stacks[PONG_THREAD] + STACK_SIZE);

	while (1)
		L4_Ipc(L4_NILTHREAD, L4_NILTHREAD, 0, msg);
}

DECLARE_THREAD(root_thread, __root_thread);
