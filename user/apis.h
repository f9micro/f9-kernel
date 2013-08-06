/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/thread.h>
#include <l4/ipc.h>
#include <types.h>

#ifndef USER_APIS_H
#define USER_APIS_H

#define L4_THREAD_NUM(n, b) \
	((b + n) << 14)

volatile int __USER_TEXT L4_ThreadControl(
		l4_thread_t dest, l4_thread_t SpaceSpecifier,
		l4_thread_t scheduler,
		l4_thread_t pager, void *UtcbLocation)
{
	 uint32_t result;

	 __asm__ __volatile__ ("ldr r4, %1\n"
	                       "svc #2\n"
	                       "str r0, %[output]\n"
	                       : [output] "=m" (result)
	                       : "m" (UtcbLocation));

	 return result;
}

volatile int __USER_TEXT L4_Ipc(
		l4_thread_t to, l4_thread_t from,
		uint32_t timeouts, uint32_t *high_mrs)
{
	uint32_t result;

	 __asm__ __volatile__ ("ldr r12, %1\n"
	                       "push {r4-r11}\n"
	                       "ldm r12, {r4-r11}\n"
	                       "svc #6\n"
	                       "pop {r4-r11}\n"
	                       "str r0, %[output]\n"
	                       : [output] "=m" (result)
	                       : "m" (high_mrs));

	 return result;
}

int __USER_TEXT L4_Start(l4_thread_t who, void *pc, void *sp)
{
	uint32_t msg[8] = {
		2,
		(uint32_t) pc,
		(uint32_t) sp,
	};

	L4_Ipc(who, L4_NILTHREAD, 0, msg);
	return 0;
}

int __USER_TEXT L4_Map(l4_thread_t where, memptr_t base, size_t size)
{
	union {
		struct {
			ipc_msg_tag_t tag;
			ipc_typed_item map1;
			ipc_typed_item map2;
		};
		uint32_t raw[8];
	} msg = {
		.tag = {
			.raw = 0,
			.s.n_typed = 2,
		},
		.map1 = {
			.map.header = IPC_TI_MAP_GRANT,
			.map.base = base >> 4,
		},
		.map2 = {
			.map.header = IPC_TI_MAP_GRANT,
			.map.base = size >> 4,
		},
	};

	L4_Ipc(where, L4_NILTHREAD, 0, msg.raw);
	return 0;
}

#endif	/* USER_APIS_H */
