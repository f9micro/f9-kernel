/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/thread.h>
#include <l4/kip.h>
#include <l4/utcb.h>
#include <l4/ipc.h>
#include <types.h>
#include <app.h>

extern app_struct user_app_start[];
extern app_struct user_app_end[];

int __USER_TEXT L4_Map(L4_ThreadId_t where, memptr_t base, size_t size)
{
	L4_Msg_t msg;
	L4_Word_t page[2] = {
		(base & 0xFFFFFFC0) | 0xA,
		size & 0xFFFFFFC0
	};

	L4_MsgPut(&msg, 0, 0, NULL, 2, page);
	L4_MsgLoad(&msg);

	L4_Send(where);
	return 0;
}

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

void __USER_TEXT map_user_sections(kip_t *kip_ptr, L4_ThreadId_t tid)
{
	kip_mem_desc_t *desc = ((void *) kip_ptr) +
			kip_ptr->memory_info.s.memory_desc_ptr;
	int n = kip_ptr->memory_info.s.n;
	int i = 0;

	for (i = 0; i < n; ++i) {
		uint32_t tag = desc[i].size & 0x3F;
		if (tag == 2 || tag == 3) {
			L4_Map(tid, desc[i].base, desc[i].size);
		}
	}
}

static void __USER_TEXT start_thread(L4_ThreadId_t t, L4_Word_t ip,
		L4_Word_t sp, L4_Word_t stack_size)
{
	L4_Msg_t msg;

	L4_MsgClear(&msg);
	L4_MsgAppendWord(&msg, ip);
	L4_MsgAppendWord(&msg, sp);
	L4_MsgAppendWord(&msg, stack_size);
	L4_MsgLoad(&msg);

	L4_Send(t);
}

#define STACK_SIZE 256

void __USER_TEXT __root_thread(kip_t *kip_ptr, utcb_t *utcb_ptr)
{
	L4_ThreadId_t myself = {.raw = utcb_ptr->t_globalid};
	char *free_mem = (char *) get_free_base(kip_ptr);

	for (app_struct *ptr = user_app_start; ptr != user_app_end; ++ptr) {
		L4_ThreadId_t tid;
		L4_Word_t stack;
		app_fpage_t *fpage = ptr->fpages;

		tid = L4_GlobalId(ptr->tid + kip_ptr->thread_info.s.user_base, 2);

		/* create thread */
		L4_ThreadControl(tid, tid, L4_nilthread, myself, free_mem);
		free_mem += UTCB_SIZE;

		/* map user_text, user_data and user_bss */
		map_user_sections(kip_ptr, tid);

		/* map thread stack */
		L4_Map(tid, (L4_Word_t)free_mem, STACK_SIZE);
		free_mem += STACK_SIZE;
		stack = (L4_Word_t)free_mem;

		/* map fpages */
		while (fpage->base || fpage->size) {
			if (fpage->base) {
				L4_Map(tid, fpage->base, fpage->size);
			}
			else {
				L4_Map(tid, (L4_Word_t)free_mem, fpage->size);
				fpage->base = (L4_Word_t)free_mem;
			}

			free_mem += fpage->size;
			fpage++;
		}

		/* start thread */
		start_thread(tid, (L4_Word_t)ptr->entry, stack, STACK_SIZE);
	}

	while (1)
		L4_Sleep(L4_Never);
}

DECLARE_THREAD(root_thread, __root_thread);
