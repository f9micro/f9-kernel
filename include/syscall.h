/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SYSCALL_H_
#define SYSCALL_H_

typedef enum {
	SYS_KERNEL_INTERFACE,		/* Not used, KIP is mapped */
	SYS_EXCHANGE_REGISTERS,
	SYS_THREAD_CONTROL,
	SYS_SYSTEM_CLOCK,
	SYS_THREAD_SWITCH,
	SYS_SCHEDULE,
	SYS_IPC,
	SYS_LIPC,
	SYS_UNMAP,
	SYS_SPACE_CONTROL,
	SYS_PROCESSOR_CONTROL,
	SYS_MEMORY_CONTROL,
} syscall_t;

void svc_handler(void);
void syscall_init(void);
void syscall_handler(void);

#endif /* SYSCALL_H_ */
