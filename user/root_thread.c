/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/thread.h>
#include <l4/kip.h>
#include <l4/utcb.h>
#include <l4/ipc.h>
#include <l4io.h>
#include <types.h>
#include <user_runtime.h>

extern user_struct user_runtime_start[];
extern user_struct user_runtime_end[];

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
	L4_MsgAppendWord(&msg, 0);
	L4_MsgAppendWord(&msg, 0);

	L4_MsgLoad(&msg);

	L4_Send(t);
}

#define STACK_SIZE 0x200

/* Align to minimum fpage size (256 bytes) to prevent overlap issues.
 * Safe version that returns max aligned address on overflow.
 */
#define FPAGE_ALIGN_SIZE 256
#define FPAGE_ALIGN_MASK (~(FPAGE_ALIGN_SIZE - 1))
static inline L4_Word_t fpage_align_safe(L4_Word_t addr)
{
	L4_Word_t aligned = (addr + FPAGE_ALIGN_SIZE - 1) & FPAGE_ALIGN_MASK;
	/* Check for overflow: if aligned < addr, we wrapped around */
	if (aligned < addr)
		return FPAGE_ALIGN_MASK;  /* Return max aligned address */
	return aligned;
}
#define FPAGE_ALIGN(addr) fpage_align_safe((L4_Word_t)(addr))

void __USER_TEXT __root_thread(kip_t *kip_ptr, utcb_t *utcb_ptr)
{
	L4_ThreadId_t myself = {.raw = utcb_ptr->t_globalid};
	char *free_mem = (char *) get_free_base(kip_ptr);

	/* Validate free_mem base - 0 means no free memory found */
	if (!free_mem) {
		/* No free memory available, halt */
		while (1)
			L4_Sleep(L4_Never);
	}

	for (user_struct *ptr = user_runtime_start; ptr != user_runtime_end; ++ptr) {
		L4_ThreadId_t tid;
		L4_Word_t stack;
		user_fpage_t *fpage = ptr->fpages;

		tid = L4_GlobalId(ptr->tid + kip_ptr->thread_info.s.user_base, 2);

		/* Align UTCB base BEFORE ThreadControl to prevent kernel's
		 * mempool_align_base() from rounding down into previous allocation
		 */
		free_mem = (char *) FPAGE_ALIGN(free_mem);

		/* create thread */
		L4_ThreadControl(tid, tid, L4_nilthread, myself, free_mem);
		free_mem += UTCB_SIZE;

		/* map user_text, user_data and user_bss */
		map_user_sections(kip_ptr, tid);

		/* map thread stack - align first to prevent overlap */
		free_mem = (char *) FPAGE_ALIGN(free_mem);
		L4_Map(tid, (L4_Word_t)free_mem, STACK_SIZE);
		free_mem += STACK_SIZE;
		stack = (L4_Word_t)free_mem;

		/* map fpages */
		while (fpage->base || fpage->size) {
			if (fpage->base) {
				L4_Map(tid, fpage->base, fpage->size);
			} else {
				/* Align dynamic allocations to the fpage's own size.
				 * This ensures the fpage can be a single MPU region.
				 * fpage->size MUST be a non-zero power of 2.
				 */
				if (!fpage->size || (fpage->size & (fpage->size - 1))) {
					printf("ERROR: invalid fpage size %p\n",
					       (void *)fpage->size);
					fpage++;
					continue;
				}
				L4_Word_t align_mask = ~(fpage->size - 1);
				free_mem = (char *)(((L4_Word_t)free_mem + fpage->size - 1) & align_mask);
				L4_Map(tid, (L4_Word_t)free_mem, fpage->size);
				fpage->base = (L4_Word_t)free_mem;
				free_mem += fpage->size;
			}

			fpage++;
		}

		/* start thread */
		start_thread(tid, (L4_Word_t)ptr->entry, stack, STACK_SIZE);
	}

	printf("root_thread: all user threads started\n");

	while (1)
		L4_Sleep(L4_Never);
}

DECLARE_THREAD(root_thread, __root_thread);
