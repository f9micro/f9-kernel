/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* ARM Cortex-M syscall implementations */

#include <platform/link.h>
#include <l4/types.h>
#include <syscall.h>
#include __L4_INC_ARCH(syscalls.h)

__USER_TEXT
void *L4_KernelInterface(L4_Word_t *ApiVersion,
                         L4_Word_t *ApiFlags,
                         L4_Word_t *KernelId)
{
	return &kip_start;
}

__USER_TEXT
L4_ThreadId_t L4_ExchangeRegisters(L4_ThreadId_t dest,
                                   L4_Word_t control,
                                   L4_Word_t sp,
                                   L4_Word_t ip,
                                   L4_Word_t flags,
                                   L4_Word_t UserDefHandle,
                                   L4_ThreadId_t pager,
                                   L4_Word_t *old_control,
                                   L4_Word_t *old_sp,
                                   L4_Word_t *old_ip,
                                   L4_Word_t *old_flags,
                                   L4_Word_t *old_UserDefhandle,
                                   L4_ThreadId_t *old_pager)
{
	L4_ThreadId_t	result;
	return result;
}

__USER_TEXT
L4_Word_t L4_ThreadControl(L4_ThreadId_t dest,
                           L4_ThreadId_t SpaceSpecifier,
                           L4_ThreadId_t Scheduler,
                           L4_ThreadId_t Pager,
                           void *UtcbLocation)
{
	L4_Word_t result;

	__asm__ __volatile__(
	    "ldr r4, %1\n"
	    "svc %[syscall_num]\n"
	    "str r0, %[output]\n"
	    : [output] "=m"(result)
	    : "m"(UtcbLocation), [syscall_num] "i"(SYS_THREAD_CONTROL));

	return result;
}

__USER_TEXT
L4_Clock_t L4_SystemClock(void)
{
	L4_Clock_t result;
	return result;
}

__USER_TEXT
void L4_ThreadSwitch(L4_ThreadId_t dest)
{
}

__USER_TEXT
L4_Word_t L4_Schedule(L4_ThreadId_t dest,
                      L4_Word_t TimeControl,
                      L4_Word_t ProcessorControl,
                      L4_Word_t PrioControl,
                      L4_Word_t PreemptionControl,
                      L4_Word_t *old_TimeControl)
{
	L4_Word_t result = 0;
	return result;
}

__USER_TEXT
L4_MsgTag_t L4_Ipc(L4_ThreadId_t to,
                   L4_ThreadId_t FromSpecifier,
                   L4_Word_t Timeouts,
                   L4_ThreadId_t *from)
{
	L4_MsgTag_t result;
	L4_ThreadId_t from_ret;

	__asm__ __volatile__(
	    "svc %[syscall_num]\n"
	    "str r0, %[from]\n"
	    : [from] "=m"(from_ret)
        : [syscall_num] "i"(SYS_IPC));

	result.raw = __L4_MR0;

	if (from != NULL)
		*from = from_ret;

	return result;
}


__USER_TEXT
L4_MsgTag_t L4_Lipc(L4_ThreadId_t to,
                    L4_ThreadId_t FromSpecifier,
                    L4_Word_t Timeouts,
                    L4_ThreadId_t *from)
{
	return L4_Ipc(to, FromSpecifier, Timeouts, from);
}

__USER_TEXT
void L4_Unmap(L4_Word_t control)
{
}

__USER_TEXT
L4_Word_t L4_SpaceControl(L4_ThreadId_t SpaceSpecifier,
                          L4_Word_t control,
                          L4_Fpage_t KernelInterfacePageArea,
                          L4_Fpage_t UtcbArea,
                          L4_ThreadId_t redirector,
                          L4_Word_t *old_control)
{
	L4_Word_t result = 0;
	return result;
}

__USER_TEXT
L4_Word_t L4_ProcessorControl(L4_Word_t ProcessorNo,
                              L4_Word_t InternalFrequency,
                              L4_Word_t ExternalFrequency,
                              L4_Word_t voltage)
{
	L4_Word_t result = 0;
	return result;
}

__USER_TEXT
L4_Word_t L4_MemoryControl(L4_Word_t control,
                           const L4_Word_t *attributes)
{
	L4_Word_t result = 0;
	return result;
}
