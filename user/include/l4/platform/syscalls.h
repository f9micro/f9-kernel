/* Copyright (c) 2001-2004, 2008-2009 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __L4_PLATFORM_SYSCALLS_H__
#define __L4_PLATFORM_SYSCALLS_H__

#include <platform/link.h>
#include __L4_INC_ARCH(vregs.h)
#include __L4_INC_ARCH(specials.h)
#include <l4/message.h>

__USER_TEXT
void *L4_KernelInterface(L4_Word_t *ApiVersion,
                         L4_Word_t *ApiFlags,
                         L4_Word_t *KernelId);

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
                                   L4_ThreadId_t *old_pager);
__USER_TEXT
L4_Word_t L4_ThreadControl(L4_ThreadId_t dest,
                           L4_ThreadId_t SpaceSpecifier,
                           L4_ThreadId_t Scheduler,
                           L4_ThreadId_t Pager,
                           void * UtcbLocation);
__USER_TEXT
L4_Clock_t L4_SystemClock(void);

__USER_TEXT
void L4_ThreadSwitch(L4_ThreadId_t dest);

__USER_TEXT
L4_Word_t L4_Schedule(L4_ThreadId_t dest,
                      L4_Word_t TimeControl,
                      L4_Word_t ProcessorControl,
                      L4_Word_t PrioControl,
                      L4_Word_t PreemptionControl,
                      L4_Word_t * old_TimeControl);
__USER_TEXT
L4_MsgTag_t L4_Ipc(L4_ThreadId_t to,
                   L4_ThreadId_t FromSpecifier,
                   L4_Word_t Timeouts,
                   L4_ThreadId_t * from);
__USER_TEXT
L4_MsgTag_t L4_Lipc(L4_ThreadId_t to,
                    L4_ThreadId_t FromSpecifier,
                    L4_Word_t Timeouts,
                    L4_ThreadId_t * from);
__USER_TEXT
void L4_Unmap(L4_Word_t control);

__USER_TEXT
L4_Word_t L4_SpaceControl(L4_ThreadId_t SpaceSpecifier,
                          L4_Word_t control,
                          L4_Fpage_t KernelInterfacePageArea,
                          L4_Fpage_t UtcbArea,
                          L4_ThreadId_t redirector,
                          L4_Word_t *old_control);
__USER_TEXT
L4_Word_t L4_ProcessorControl(L4_Word_t ProcessorNo,
                              L4_Word_t InternalFrequency,
                              L4_Word_t ExternalFrequency,
                              L4_Word_t voltage);
__USER_TEXT
L4_Word_t L4_MemoryControl(L4_Word_t control,
                           const L4_Word_t * attributes);

#endif /* !__L4_PLATFORM_SYSCALLS_H__ */
