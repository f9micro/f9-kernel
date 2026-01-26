/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* ARM Cortex-M syscall implementations */

#include <l4/types.h>
#include <platform/link.h>
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
    L4_ThreadId_t result = {0};
    /* FIXME: unimplemented */
    return result;
}

__USER_TEXT
L4_Word_t L4_ThreadControl(L4_ThreadId_t dest,
                           L4_ThreadId_t SpaceSpecifier,
                           L4_ThreadId_t Scheduler,
                           L4_ThreadId_t Pager,
                           void *UtcbLocation)
{
    register L4_Word_t r0 __asm__("r0") = dest.raw;
    register L4_Word_t r1 __asm__("r1") = SpaceSpecifier.raw;
    register L4_Word_t r2 __asm__("r2") = Scheduler.raw;
    register L4_Word_t r3 __asm__("r3") = Pager.raw;
    register L4_Word_t r4 __asm__("r4") = (L4_Word_t) UtcbLocation;

    __asm__ __volatile__("svc %[syscall_num]\n"
                         : "+r"(r0)
                         : "r"(r1), "r"(r2), "r"(r3),
                           "r"(r4), [syscall_num] "i"(SYS_THREAD_CONTROL)
                         : "memory");

    return r0;
}

__USER_TEXT
L4_Clock_t L4_SystemClock(void)
{
    L4_Clock_t result = {0};
    /* FIXME: unimplemented */
    return result;
}

__USER_TEXT
void L4_ThreadSwitch(L4_ThreadId_t dest) {}

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
    register L4_Word_t r0 __asm__("r0") = to.raw;
    register L4_Word_t r1 __asm__("r1") = FromSpecifier.raw;
    register L4_Word_t r2 __asm__("r2") = Timeouts;

    __asm__ __volatile__("svc %[syscall_num]\n"
                         : "+r"(r0)
                         : "r"(r1), "r"(r2), [syscall_num] "i"(SYS_IPC)
                         : "memory");

    result.raw = __L4_MR0;

    if (from)
        from->raw = r0;

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
void L4_Unmap(L4_Word_t control) {}

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
L4_Word_t L4_MemoryControl(L4_Word_t control, const L4_Word_t *attributes)
{
    L4_Word_t result = 0;
    return result;
}
