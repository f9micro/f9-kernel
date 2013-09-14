/*********************************************************************
 *                
 * Copyright (C) 2001-2004, 2008-2009,  Karlsruhe University
 *                
 * File path:     l4/ia32/syscalls.h
 * Description:   x86 syscall implementations
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: syscalls.h,v 1.31 2004/08/23 18:43:11 skoglund Exp $
 *                
 ********************************************************************/

#include <platform/link.h>
#include <l4/types.h>
#include __L4_INC_ARCH(syscalls.h)


__USER_TEXT void * L4_KernelInterface (L4_Word_t *ApiVersion,
				     L4_Word_t *ApiFlags,
				     L4_Word_t *KernelId)
{
    void * base_address = 0;
    return base_address;
}


__USER_TEXT L4_ThreadId_t L4_ExchangeRegisters (L4_ThreadId_t dest,
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


__USER_TEXT L4_Word_t L4_ThreadControl (L4_ThreadId_t dest,
				      L4_ThreadId_t SpaceSpecifier,
				      L4_ThreadId_t Scheduler,
				      L4_ThreadId_t Pager,
				      void * UtcbLocation)
{
    L4_Word_t result;

	 __asm__ __volatile__ ("ldr r4, %1\n"
	                       "svc #2\n"
	                       "str r0, %[output]\n"
	                       : [output] "=m" (result)
	                       : "m" (UtcbLocation));

	 return result;
}


__USER_TEXT L4_Clock_t L4_SystemClock (void)
{
    L4_Clock_t result;
    return result;
}


__USER_TEXT void L4_ThreadSwitch (L4_ThreadId_t dest)
{
}


__USER_TEXT L4_Word_t  L4_Schedule (L4_ThreadId_t dest,
				  L4_Word_t TimeControl,
				  L4_Word_t ProcessorControl,
				  L4_Word_t PrioControl,
				  L4_Word_t PreemptionControl,
				  L4_Word_t * old_TimeControl)
{
    L4_Word_t result = 0;
    return result;
}

__USER_TEXT L4_MsgTag_t L4_Ipc (L4_ThreadId_t to,
			      L4_ThreadId_t FromSpecifier,
			      L4_Word_t Timeouts,
			      L4_ThreadId_t * from)
{
    L4_MsgTag_t result;

	 __asm__ __volatile__ ("svc #6\n");

	result.raw = __L4_MR0;

	 return result;
}


__USER_TEXT L4_MsgTag_t L4_Lipc (L4_ThreadId_t to,
			       L4_ThreadId_t FromSpecifier,
			       L4_Word_t Timeouts,
			       L4_ThreadId_t * from)
{
	return L4_Ipc(to, FromSpecifier, Timeouts, from);
}

__USER_TEXT void L4_Unmap (L4_Word_t control)
{
}


__USER_TEXT L4_Word_t L4_SpaceControl (L4_ThreadId_t SpaceSpecifier,
				     L4_Word_t control,
				     L4_Fpage_t KernelInterfacePageArea,
				     L4_Fpage_t UtcbArea,
				     L4_ThreadId_t redirector,
				     L4_Word_t *old_control)
{
    L4_Word_t result = 0;
    return result;
}


__USER_TEXT L4_Word_t L4_ProcessorControl (L4_Word_t ProcessorNo,
					 L4_Word_t InternalFrequency,
					 L4_Word_t ExternalFrequency,
					 L4_Word_t voltage)
{
    L4_Word_t result = 0;
    return result;
}


__USER_TEXT L4_Word_t L4_MemoryControl (L4_Word_t control,
				 const L4_Word_t * attributes)
{
	L4_Word_t result = 0;
    return result;
}

