/****************************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:	l4/powerpc64/syscalls.h
 * Description:	PowerPC64 system call implementations.
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
 * $Id: syscalls.h,v 1.7 2004/08/23 18:43:16 skoglund Exp $
 *
 ***************************************************************************/
#ifndef __L4__POWERPC64__SYSCALLS_H__
#define __L4__POWERPC64__SYSCALLS_H__

#include <l4/types.h>
#include <l4/message.h>

#define __L4_PPC64_CLOBBER_REGS						\
		"r0",  "r11", "r12", "r14", "r15", "r16", "r17",	\
		"r18", "r19", "r20", "r21", "r22", "r23", "r24",	\
		"r25", "r26", "r27", "r28", "r29", "lr", "ctr"
#if (__GNUC__ >= 3)
#define __L4_PPC64_CLOBBER_CR_REGS					\
		"cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6",	\
		"cr7", "xer"
#else
#define __L4_PPC64_CLOBBER_CR_REGS					\
		"cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7"
#endif

L4_INLINE void * L4_KernelInterface(
	L4_Word_t *ApiVersion,
	L4_Word_t *ApiFlags,
	L4_Word_t *KernelId
	)
{
    register void * base_address asm("r3");
    register L4_Word_t api_version asm("r4");
    register L4_Word_t api_flags asm("r5");
    register L4_Word_t kernel_id asm("r6");

    __asm__ __volatile__ (
	"tlbia ;"
	: /* ouputs */
	 "=r" (base_address), "=r" (api_version),
	 "=r" (api_flags), "=r" (kernel_id)
	: /* inputs */
    );

    if( ApiVersion ) *ApiVersion = api_version;
    if( ApiFlags ) *ApiFlags = api_flags;
    if( KernelId ) *KernelId = kernel_id;

    return base_address;
}

typedef L4_Word_t (*__L4_ExchangeRegisters_t)( L4_Word_t, L4_Word_t, L4_Word_t,
	L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_ExchangeRegisters_t __L4_ExchangeRegisters;

L4_INLINE L4_ThreadId_t L4_ExchangeRegisters(
	L4_ThreadId_t dest,
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
	L4_Word_t *old_UserDefHandle,
	L4_ThreadId_t *old_pager
	)
{
    register L4_Word_t r3 asm("r3") = dest.raw;
    register L4_Word_t r4 asm("r4") = control;
    register L4_Word_t r5 asm("r5") = sp;
    register L4_Word_t r6 asm("r6") = ip;
    register L4_Word_t r7 asm("r7") = flags;
    register L4_Word_t r8 asm("r8") = UserDefHandle;
    register L4_Word_t r9 asm("r9") = pager.raw;
    register L4_Word_t r10 asm("r10") = 0; // -- is_local ??

    __asm__ __volatile__ (
	"mtctr  %[sys];"
	"bctrl ;"
	: /* outputs */
	 "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6),
	 "+r" (r7), "+r" (r8), "+r" (r9), "+r" (r10)
	: /* inputs */
	 [sys] "r" (__L4_ExchangeRegisters)
	: /* clobbers */
	 __L4_PPC64_CLOBBER_REGS, "memory", __L4_PPC64_CLOBBER_CR_REGS
    );

    *old_control = r4;
    *old_sp = r5;
    *old_ip = r6;
    *old_flags = r7;
    *old_UserDefHandle = r8;
    old_pager->raw = r9;

    return (L4_ThreadId_t) {raw: r3};
}

typedef L4_Word_t (*__L4_ThreadControl_t)( L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_ThreadControl_t __L4_ThreadControl;

L4_INLINE L4_Word_t L4_ThreadControl(
	L4_ThreadId_t dest,
	L4_ThreadId_t SpaceSpecifier,
	L4_ThreadId_t Scheduler,
	L4_ThreadId_t Pager,
	void * UtcbLocation)
{
    register L4_Word_t r3 asm("r3") = dest.raw;
    register L4_Word_t r4 asm("r4") = SpaceSpecifier.raw;
    register L4_Word_t r5 asm("r5") = Scheduler.raw;
    register L4_Word_t r6 asm("r6") = Pager.raw;
    register void *    r7 asm("r7") = UtcbLocation;

    __asm__ __volatile__ (
	"mtctr  %[sys];"
	"bctrl;"
	: /* outputs */
	 "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6), "+r" (r7)
	: /* inputs */
	 [sys] "r" (__L4_ThreadControl)
	: /* clobbers */
	 "r8", "r9", "r10",
	 __L4_PPC64_CLOBBER_REGS, "memory", __L4_PPC64_CLOBBER_CR_REGS
    );

    return r3;
}

typedef L4_Clock_t (*__L4_SystemClock_t)( void );
extern __L4_SystemClock_t __L4_SystemClock;

L4_INLINE L4_Clock_t L4_SystemClock( void )
{
    register L4_Clock_t r3 asm("r3");

    __asm__ __volatile__ (
	"mtctr  %[sys];"
	"bctrl;"
	: /* outputs */
	 "=r" (r3.raw)
	: /* inputs */
	 [sys] "r" (__L4_SystemClock)
	: /* clobbers */
	 "r4", "r5", "r6", "r7", "r8", "r9", "r10",
	 __L4_PPC64_CLOBBER_REGS, "memory", __L4_PPC64_CLOBBER_CR_REGS
    );

    return ( r3 );
}

typedef L4_Word_t (*__L4_ThreadSwitch_t)( L4_Word_t );
extern __L4_ThreadSwitch_t __L4_ThreadSwitch;

L4_INLINE void L4_ThreadSwitch( L4_ThreadId_t dest )
{
    register L4_Word_t r3 asm("r3") = dest.raw;
    
    __asm__ __volatile__ (
	"mtctr  %[sys];"
	"bctrl ;"
	: /* outputs */
	 "+r" (r3)
	: /* inputs */
	 [sys] "r" (__L4_ThreadSwitch)
	: /* clobbers */
	 "r4", "r5", "r6", "r7", "r8", "r9", "r10",
	 __L4_PPC64_CLOBBER_REGS, "memory", __L4_PPC64_CLOBBER_CR_REGS
    );
}

typedef L4_Word_t (*__L4_Schedule_t)( L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_Schedule_t __L4_Schedule;

L4_INLINE L4_Word_t  L4_Schedule(
	L4_ThreadId_t dest,
	L4_Word_t TimeControl,
	L4_Word_t ProcessorControl,
	L4_Word_t PrioControl,
	L4_Word_t PreemptionControl,
	L4_Word_t * old_TimeControl
	)
{
    register L4_Word_t r3 asm("r3") = dest.raw;
    register L4_Word_t r4 asm("r4") = TimeControl;
    register L4_Word_t r5 asm("r5") = ProcessorControl;
    register L4_Word_t r6 asm("r6") = PrioControl;
    register L4_Word_t r7 asm("r7") = PreemptionControl;
    
    __asm__ __volatile__ (
	"mtctr  %[sys];"
	"bctrl;"
	: /* outputs */
	 "+r" (r3), "+r" (r4)
	: /* inputs */
	 [sys] "r" (__L4_Schedule), "r" (r5), "r" (r6), "r" (r7)
	: /* clobbers */
	 "r8", "r9", "r10",
	 __L4_PPC64_CLOBBER_REGS, "memory", __L4_PPC64_CLOBBER_CR_REGS
    );

    *old_TimeControl = r4;
    return r3;
}

typedef L4_Word_t (*__L4_Ipc_t)( L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_Ipc_t __L4_Ipc;

L4_INLINE L4_MsgTag_t L4_Ipc(
	L4_ThreadId_t to,
	L4_ThreadId_t FromSpecifier,
	L4_Word_t Timeouts,
	L4_ThreadId_t *from
	)
{
    register L4_Word_t r3 asm("r3") = to.raw;
    register L4_Word_t r4 asm("r4") = FromSpecifier.raw;
    register L4_Word_t r5 asm("r5") = Timeouts;

    register L4_Word_t mr0 asm ("r14");
    register L4_Word_t mr1 asm ("r15");
    register L4_Word_t mr2 asm ("r16");
    register L4_Word_t mr3 asm ("r17");
    register L4_Word_t mr4 asm ("r18");
    register L4_Word_t mr5 asm ("r19");
    register L4_Word_t mr6 asm ("r20");
    register L4_Word_t mr7 asm ("r21");
    register L4_Word_t mr8 asm ("r22");
    register L4_Word_t mr9 asm ("r23");

    // Only load MRs if send phase is included
    if (! L4_IsNilThread (to))
    {
	mr0 = (__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 0];
	mr1 = (__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 1];
	mr2 = (__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 2];
	mr3 = (__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 3];
	mr4 = (__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 4];
	mr5 = (__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 5];
	mr6 = (__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 6];
	mr7 = (__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 7];
	mr8 = (__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 8];
	mr9 = (__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 9];

	asm volatile (
	    "" ::
	     "r" (mr0), "r" (mr1), "r" (mr2), "r" (mr3), "r" (mr4),
	     "r" (mr5), "r" (mr6), "r" (mr7), "r" (mr8), "r" (mr9)
	);
    }

    asm volatile (
	"mtctr	%[sys];"
	"bctrl;"

	: /* outputs */
	 "+r" (r3), "+r" (r4), "+r" (r5),
	 "=r" (mr0), "=r" (mr1), "=r" (mr2), "=r" (mr3), "=r" (mr4),
	 "=r" (mr5), "=r" (mr6), "=r" (mr7), "=r" (mr8), "=r" (mr9)
	: /* inputs */
	 [sys] "r" (__L4_Ipc)
	: /* clobbers */
	 "r0", "ctr", "lr", "memory"
    );

    /* Trash the rest. This allows the compiler to choose which
     * inputs to use in the asm code above
     */
    asm volatile (
	"" :::
	 "r0", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
	 "r24", "r25", "r26", "r27", "r28", "r29",
	 __L4_PPC64_CLOBBER_CR_REGS
    );

    if( !L4_IsNilThread(FromSpecifier) ) {
	from->raw = r3;		    /* Result is in r3 */

	(__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 0] = mr0;
	(__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 1] = mr1;
	(__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 2] = mr2;
	(__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 3] = mr3;
	(__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 4] = mr4;
	(__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 5] = mr5;
	(__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 6] = mr6;
	(__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 7] = mr7;
	(__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 8] = mr8;
	(__L4_PPC64_Utcb())[__L4_TCR_MR_OFFSET + 9] = mr9;
    }

    return (L4_MsgTag_t){ raw: mr0 };	/* mr0 */
}

typedef L4_Word_t (*__L4_Lipc_t)( L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_Lipc_t __L4_Lipc;

L4_INLINE L4_MsgTag_t L4_Lipc(
	L4_ThreadId_t to,
	L4_ThreadId_t FromSpecifier,
	L4_Word_t Timeouts,
	L4_ThreadId_t *from
	)
{
    return L4_Ipc( to, FromSpecifier, Timeouts, from );
}

typedef void (*__L4_Unmap_t)( L4_Word_t );
extern __L4_Unmap_t __L4_Unmap;

L4_INLINE void L4_Unmap( L4_Word_t control )
{
    register L4_Word_t r3 asm("r3") = control;
 
    asm volatile (
	"mtctr %[sys];"
	"bctrl ;"
	: /* outputs */
	 "+r" (r3)
	: /* inputs */
	 [sys] "r" (__L4_Unmap)
	: /* clobbers */
	 "r4", "r5", "r6", "r7", "r8", "r9", "r10",
	 __L4_PPC64_CLOBBER_REGS, "memory", __L4_PPC64_CLOBBER_CR_REGS
    );
}

typedef void (*__L4_SpaceControl_t)( L4_Word_t, L4_Word_t, L4_Word_t,
	L4_Word_t, L4_Word_t );
extern __L4_SpaceControl_t __L4_SpaceControl;

L4_INLINE L4_Word_t L4_SpaceControl(
	L4_ThreadId_t SpaceSpecifier,
	L4_Word_t control,
	L4_Fpage_t KernelInterfacePageArea,
	L4_Fpage_t UtcbArea,
	L4_ThreadId_t redirector,
	L4_Word_t *old_control
	)
{
    register L4_Word_t r3 asm("r3") = SpaceSpecifier.raw;
    register L4_Word_t r4 asm("r4") = control;
    register L4_Word_t r5 asm("r5") = KernelInterfacePageArea.raw;
    register L4_Word_t r6 asm("r6") = UtcbArea.raw;
    register L4_Word_t r7 asm("r7") = redirector.raw;

    __asm__ __volatile__ (
	"mtctr  %[sys];"
	"bctrl ;"
	: /* outputs */
	 "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6), "+r" (r7)
	: /* inputs */
	 [sys] "r" (__L4_SpaceControl)
	: /* clobbers */
	 "r8", "r9", "r10",
	 __L4_PPC64_CLOBBER_REGS, "memory", __L4_PPC64_CLOBBER_CR_REGS
    );

    *old_control = r4;
    return r3;
}

typedef L4_Word_t (*__L4_ProcessorControl_t)( L4_Word_t,  L4_Word_t,
	L4_Word_t, L4_Word_t );
extern __L4_ProcessorControl_t __L4_ProcessorControl;

L4_INLINE L4_Word_t L4_ProcessorControl(
	L4_Word_t ProcessorNo,
	L4_Word_t InternalFrequency,
	L4_Word_t ExternalFrequency,
	L4_Word_t voltage
	)
{
    register L4_Word_t r3 asm("r3") = ProcessorNo;
    register L4_Word_t r4 asm("r4") = InternalFrequency;
    register L4_Word_t r5 asm("r5") = ExternalFrequency;
    register L4_Word_t r6 asm("r6") = voltage;

    __asm__ __volatile__ (
	    "mtctr  %[sys];"
	    "bctrl ;"
	    : /* outputs */
	      "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6)
	    : /* inputs */
	      [sys] "r" (__L4_ProcessorControl)
	    : /* clobbers */
	      "r7", "r8", "r9", "r10",
	      __L4_PPC64_CLOBBER_REGS, "memory", __L4_PPC64_CLOBBER_CR_REGS
	    );

    return r3;
}

typedef L4_Word_t (*__L4_MemoryControl_t)( L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_MemoryControl_t __L4_MemoryControl;

L4_INLINE L4_Word_t L4_MemoryControl(
	L4_Word_t control,
	const L4_Word_t attributes[4]
	)
{
    register L4_Word_t r3 asm("r3") = control;
    register L4_Word_t r4 asm("r4") = attributes[0];
    register L4_Word_t r5 asm("r5") = attributes[1];
    register L4_Word_t r6 asm("r6") = attributes[2];
    register L4_Word_t r7 asm("r7") = attributes[3];

    __asm__ __volatile__ (
	"mtctr  %[sys];"
	"bctrl ;"
	: /* outputs */
	 "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6), "+r" (r7)
	: /* inputs */
	 [sys] "r" (__L4_MemoryControl)
	: /* clobbers */
	 "r8", "r9", "r10",
	 __L4_PPC64_CLOBBER_REGS, "memory", __L4_PPC64_CLOBBER_CR_REGS
    );

    return r3;
}

typedef L4_Word_t (*__L4_RtasCall_t)( L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t* );
extern __L4_RtasCall_t __L4_RtasCall;

L4_INLINE L4_Word_t L4_RtasCall(
	L4_Word_t token,
	L4_Word_t nargs,
	L4_Word_t nret,
	L4_Word_t *ptr
	)
{
    if (!__L4_RtasCall)
	return -1ul;

    register L4_Word_t r3 asm("r3") = token;
    register L4_Word_t r4 asm("r4") = nargs;
    register L4_Word_t r5 asm("r5") = nret;
    register L4_Word_t r6 asm("r6") = (L4_Word_t)ptr;

    __asm__ __volatile__ (
	"mtctr  %[sys];"
	"bctrl ;"
	: /* outputs */
	 "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6)
	: /* inputs */
	 [sys] "r" (__L4_RtasCall)
	: /* clobbers */
	 "r7", "r8", "r9", "r10",
	 __L4_PPC64_CLOBBER_REGS, "memory", __L4_PPC64_CLOBBER_CR_REGS
    );

    return r3;
}

#endif	/* __L4__POWERPC64__SYSCALLS_H__ */
