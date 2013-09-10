/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     l4/powerpc/syscalls.h
 * Description:   
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
 * $Id$
 *                
 ********************************************************************/
#ifndef __L4__POWERPC__SYSCALLS_H__
#define __L4__POWERPC__SYSCALLS_H__

#include <l4/types.h>
#include <l4/message.h>

#define __L4_PPC_CLOBBER_REGS	\
			"r0", "r11", "r12", "r13", "r14", "r15", "r16", "r17",\
			"r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25",\
			"r26", "r27", "r28", "r29", "lr", "ctr"
#if (__GNUC__ >= 3)
#define __L4_PPC_CLOBBER_CR_REGS	\
			"cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", \
			"cr7", "xer"
#else
#define __L4_PPC_CLOBBER_CR_REGS	\
			"cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7"
#endif

typedef struct {
    L4_Word_t r3;
    L4_Word_t r4;
} __L4_Return_t;

L4_INLINE void * L4_KernelInterface(
	L4_Word_t *ApiVersion,
	L4_Word_t *ApiFlags,
	L4_Word_t *KernelId
	)
{
    register void * base_address;
    register L4_Word_t api_version;
    register L4_Word_t api_flags;
    register L4_Word_t kernel_id;

    __asm__ __volatile__ (
	    "tlbia ;"
	    "mr	%0, %%r3 ;"
	    "mr	%1, %%r4 ;"
	    "mr	%2, %%r5 ;"
	    "mr	%3, %%r6 ;"
	    : /* ouputs */
	      "=r" (base_address), "=r" (api_version),
	      "=r" (api_flags), "=r" (kernel_id)
	    : /* inputs */
	    : /* clobbers */
	      "r3", "r4", "r5", "r6"
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
    register L4_Word_t r3 __asm__("r3") = dest.raw;

    struct {
	L4_Word_t control;
	L4_Word_t sp;
	L4_Word_t ip;
	L4_Word_t flags;
	L4_Word_t handle;
	L4_Word_t pager;
    } inputs;

    inputs.control = control;
    inputs.sp = sp;
    inputs.ip = ip;
    inputs.flags = flags;
    inputs.handle = UserDefHandle;
    inputs.pager = pager.raw;

    __asm__ __volatile__ (
	    "lwz %%r4, (0*4)(%2) ;"
	    "lwz %%r5, (1*4)(%2) ;"
	    "lwz %%r6, (2*4)(%2) ;"
	    "lwz %%r7, (3*4)(%2) ;"
	    "lwz %%r8, (4*4)(%2) ;"
	    "lwz %%r9, (5*4)(%2) ;"
	    "mtctr %1 ;"
	    "bctrl ;"
	    "stw %%r4, (0*4)(%2) ;"
	    "stw %%r5, (1*4)(%2) ;"
	    "stw %%r6, (2*4)(%2) ;"
	    "stw %%r7, (3*4)(%2) ;"
	    "stw %%r8, (4*4)(%2) ;"
	    "stw %%r9, (5*4)(%2) ;"
	    : /* outputs */
	      "+r" (r3)
	    : /* inputs */
	      "r" (__L4_ExchangeRegisters), "r" (&inputs)
	    : /* clobbers */
	      "r4", "r5", "r6", "r7", "r8", "r9", "r10",
	      __L4_PPC_CLOBBER_REGS, "memory", __L4_PPC_CLOBBER_CR_REGS
	    );

    *old_control = inputs.control;
    *old_sp = inputs.sp;
    *old_ip = inputs.ip;
    *old_flags = inputs.flags;
    *old_UserDefHandle = inputs.handle;
    old_pager->raw = inputs.pager;

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
    register L4_Word_t r3 __asm__("r3") = dest.raw;
    register L4_Word_t r4 __asm__("r4") = SpaceSpecifier.raw;
    register L4_Word_t r5 __asm__("r5") = Scheduler.raw;
    register L4_Word_t r6 __asm__("r6") = Pager.raw;
    register void *    r7 __asm__("r7") = UtcbLocation;

    __asm__ __volatile__ (
	    "mtctr %5 ;"
	    "bctrl ;"
	    : /* outputs */
	    "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6), "+r" (r7)
	    : /* inputs */
	    "r" (__L4_ThreadControl)
	    : /* clobbers */
	    "r8", "r9", "r10",
	      __L4_PPC_CLOBBER_REGS, "memory", __L4_PPC_CLOBBER_CR_REGS
	    );

    return r3;
}

typedef L4_Clock_t (*__L4_SystemClock_t)( void );
extern __L4_SystemClock_t __L4_SystemClock;

L4_INLINE L4_Clock_t L4_SystemClock( void )
{
    register L4_Word_t r3 __asm__("r3");
    register L4_Word_t r4 __asm__("r4");

    __asm__ __volatile__ (
	    "mtctr %2 ;"
	    "bctrl ;"
	    : /* outputs */
	      "=r" (r3), "=r" (r4)
	    : /* inputs */
	      "r" (__L4_SystemClock)
	    : /* clobbers */
	      "r5", "r6", "r7", "r8", "r9", "r10",
	      __L4_PPC_CLOBBER_REGS, "memory", __L4_PPC_CLOBBER_CR_REGS
	    );

    return  ((L4_Clock_t){ X: {r3, r4} });
}

typedef L4_Word_t (*__L4_ThreadSwitch_t)( L4_Word_t );
extern __L4_ThreadSwitch_t __L4_ThreadSwitch;

L4_INLINE void L4_ThreadSwitch( L4_ThreadId_t dest )
{
    register L4_Word_t r3 __asm__("r3") = dest.raw;
    
    __asm__ __volatile__ (
	    "mtctr %1 ;"
	    "bctrl ;"
	    : /* outputs */
	      "+r" (r3)
	    : /* inputs */
	      "r" (__L4_ThreadSwitch)
	    : /* clobbers */
	      "r4", "r5", "r6", "r7", "r8", "r9", "r10",
	      __L4_PPC_CLOBBER_REGS, "memory", __L4_PPC_CLOBBER_CR_REGS
	    );
}

typedef __L4_Return_t (*__L4_Schedule_t)( L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_Schedule_t __L4_Schedule;

L4_INLINE L4_Word_t  L4_Schedule(
	L4_ThreadId_t dest,
	L4_Word_t TimeControl,
	L4_Word_t ProcessorControl,
	L4_Word_t prio,
	L4_Word_t PreemptionControl,
	L4_Word_t * old_TimeControl
	)
{
    register L4_Word_t r3 __asm__("r3") = dest.raw;
    register L4_Word_t r4 __asm__("r4") = TimeControl;
    register L4_Word_t r5 __asm__("r5") = ProcessorControl;
    register L4_Word_t r6 __asm__("r6") = prio;
    register L4_Word_t r7 __asm__("r7") = PreemptionControl;
    
    __asm__ __volatile__ (
	    "mtctr %2 ;"
	    "bctrl ;"
	    : /* outputs */
	      "+r" (r3), "+r" (r4)
	    : /* inputs */
	      "r" (__L4_Schedule), "r" (r5), "r" (r6), "r" (r7)
	    : /* clobbers */
	      "r8", "r9", "r10",
	      __L4_PPC_CLOBBER_REGS, "memory", __L4_PPC_CLOBBER_CR_REGS
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
    register L4_Word_t r15 __asm__("r15") = to.raw;
    register L4_Word_t r16 __asm__("r16") = FromSpecifier.raw;
    register L4_Word_t r17 __asm__("r17") = Timeouts;

    __asm__ __volatile__ (
	    "lwz %%r0, 36 (%%r2) ;"
	    "lwz %%r3, 4  (%%r2) ;"
	    "lwz %%r4, 8  (%%r2) ;"
	    "lwz %%r5, 12 (%%r2) ;"
	    "lwz %%r6, 16 (%%r2) ;"
	    "lwz %%r7, 20 (%%r2) ;"
	    "lwz %%r8, 24 (%%r2) ;"
	    "lwz %%r9, 28 (%%r2) ;"
	    "lwz %%r10, 32 (%%r2) ;"
	    "lwz %%r14, 0 (%%r2) ;"

	    "mtctr %3 ;"
	    "bctrl ;"

	    "stw %%r0, 36 (%%r2) ;"
	    "stw %%r3, 4  (%%r2) ;"
	    "stw %%r4, 8  (%%r2) ;"
	    "stw %%r5, 12 (%%r2) ;"
	    "stw %%r6, 16 (%%r2) ;"
	    "stw %%r7, 20 (%%r2) ;"
	    "stw %%r8, 24 (%%r2) ;"
	    "stw %%r9, 28 (%%r2) ;"
	    "stw %%r10, 32 (%%r2) ;"
	    "stw %%r14, 0 (%%r2) ;"

	    : /* outputs */
	      "+r" (r15), "+r" (r16), "+r" (r17)
	    : /* inputs */
	      "r" (__L4_Ipc)
	    : /* clobbers */
	      "r0", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11",
	      "r12", "r13", "r14",
	      "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", 
	      "r26", "r27", "r28", "r29", "lr", "ctr",
	      "memory", __L4_PPC_CLOBBER_CR_REGS
	    );

    if( !L4_IsNilThread(FromSpecifier) && from ) {
	from->raw = r16;
    }

    return (L4_MsgTag_t){ raw: __L4_PPC_Utcb()[0] };	/* mr0 */
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
    register L4_Word_t r15 __asm__("r15") = to.raw;
    register L4_Word_t r16 __asm__("r16") = FromSpecifier.raw;
    register L4_Word_t r17 __asm__("r17") = Timeouts;

    __asm__ __volatile__ (
	    "lwz %%r0, 36 (%%r2) ;"
	    "lwz %%r3, 4  (%%r2) ;"
	    "lwz %%r4, 8  (%%r2) ;"
	    "lwz %%r5, 12 (%%r2) ;"
	    "lwz %%r6, 16 (%%r2) ;"
	    "lwz %%r7, 20 (%%r2) ;"
	    "lwz %%r8, 24 (%%r2) ;"
	    "lwz %%r9, 28 (%%r2) ;"
	    "lwz %%r10, 32 (%%r2) ;"
	    "lwz %%r14, 0 (%%r2) ;"

	    "mtctr %3 ;"
	    "bctrl ;"

	    "stw %%r0, 36 (%%r2) ;"
	    "stw %%r3, 4  (%%r2) ;"
	    "stw %%r4, 8  (%%r2) ;"
	    "stw %%r5, 12 (%%r2) ;"
	    "stw %%r6, 16 (%%r2) ;"
	    "stw %%r7, 20 (%%r2) ;"
	    "stw %%r8, 24 (%%r2) ;"
	    "stw %%r9, 28 (%%r2) ;"
	    "stw %%r10, 32 (%%r2) ;"
	    "stw %%r14, 0 (%%r2) ;"

	    : /* outputs */
	      "+r" (r15), "+r" (r16), "+r" (r17)
	    : /* inputs */
	      "r" (__L4_Lipc)
	    : /* clobbers */
	      "r0", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11",
	      "r12", "r13", "r14",
	      "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", 
	      "r26", "r27", "r28", "r29", "lr", "ctr",
	      "memory", __L4_PPC_CLOBBER_CR_REGS
	    );

    if( !L4_IsNilThread(FromSpecifier) ) {
	from->raw = r16;
    }

   return (L4_MsgTag_t){ raw: __L4_PPC_Utcb()[0] };	/* mr0 */
}

typedef void (*__L4_Unmap_t)( L4_Word_t );
extern __L4_Unmap_t __L4_Unmap;

L4_INLINE void L4_Unmap( L4_Word_t control )
{
    __asm__ __volatile__ (
	    "mr %%r15, %1 ;"
	    "mtctr %0 ;"

	    "lwz %%r0, 36 (%%r2) ;"
	    "lwz %%r3, 4  (%%r2) ;"
	    "lwz %%r4, 8  (%%r2) ;"
	    "lwz %%r5, 12 (%%r2) ;"
	    "lwz %%r6, 16 (%%r2) ;"
	    "lwz %%r7, 20 (%%r2) ;"
	    "lwz %%r8, 24 (%%r2) ;"
	    "lwz %%r9, 28 (%%r2) ;"
	    "lwz %%r10, 32 (%%r2) ;"
	    "lwz %%r14, 0 (%%r2) ;"

	    "bctrl ;"

	    "stw %%r0, 36 (%%r2) ;"
	    "stw %%r3, 4  (%%r2) ;"
	    "stw %%r4, 8  (%%r2) ;"
	    "stw %%r5, 12 (%%r2) ;"
	    "stw %%r6, 16 (%%r2) ;"
	    "stw %%r7, 20 (%%r2) ;"
	    "stw %%r8, 24 (%%r2) ;"
	    "stw %%r9, 28 (%%r2) ;"
	    "stw %%r10, 32 (%%r2) ;"
	    "stw %%r14, 0 (%%r2) ;"

	    : /* outputs */
	    : /* inputs */
	      "r" (__L4_Unmap), "r" (control)
	    : /* clobbers */
	      "r0", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
	      __L4_PPC_CLOBBER_REGS, "memory", __L4_PPC_CLOBBER_CR_REGS
	    );
}

typedef __L4_Return_t (*__L4_SpaceControl_t)( L4_Word_t, L4_Word_t, L4_Word_t,
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
    register L4_Word_t r3 __asm__("r3") = SpaceSpecifier.raw;
    register L4_Word_t r4 __asm__("r4") = control;
    register L4_Word_t r5 __asm__("r5") = KernelInterfacePageArea.raw;
    register L4_Word_t r6 __asm__("r6") = UtcbArea.raw;
    register L4_Word_t r7 __asm__("r7") = redirector.raw;

    __asm__ __volatile__ (
	    "mtctr %5 ;"
	    "bctrl ;"
	    : /* outputs */
	      "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6), "+r" (r7)
	    : /* inputs */
	      "r" (__L4_SpaceControl)
	    : /* clobbers */
	      "r8", "r9", "r10",
	      __L4_PPC_CLOBBER_REGS, "memory", __L4_PPC_CLOBBER_CR_REGS
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
    register L4_Word_t r3 __asm__("r3") = ProcessorNo;
    register L4_Word_t r4 __asm__("r4") = InternalFrequency;
    register L4_Word_t r5 __asm__("r5") = ExternalFrequency;
    register L4_Word_t r6 __asm__("r6") = voltage;

    __asm__ __volatile__ (
	    "mtctr %4 ;"
	    "bctrl ;"
	    : /* outputs */
	      "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6)
	    : /* inputs */
	      "r" (__L4_ProcessorControl)
	    : /* clobbers */
	      "r7", "r8", "r9", "r10",
	      __L4_PPC_CLOBBER_REGS, "memory", __L4_PPC_CLOBBER_CR_REGS
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
    register L4_Word_t r15 __asm__("r15") = control;
    register L4_Word_t r16 __asm__("r16") = attributes[0];
    register L4_Word_t r17 __asm__("r17") = attributes[1];
    register L4_Word_t r18 __asm__("r18") = attributes[2];
    register L4_Word_t r19 __asm__("r19") = attributes[3];
    register L4_Word_t r3 __asm__("r3");

    __asm__ __volatile__ (
	    "lwz %%r0, 36 (%%r2) ;"
	    "lwz %%r3, 4  (%%r2) ;"
	    "lwz %%r4, 8  (%%r2) ;"
	    "lwz %%r5, 12 (%%r2) ;"
	    "lwz %%r6, 16 (%%r2) ;"
	    "lwz %%r7, 20 (%%r2) ;"
	    "lwz %%r8, 24 (%%r2) ;"
	    "lwz %%r9, 28 (%%r2) ;"
	    "lwz %%r10, 32 (%%r2) ;"
	    "lwz %%r14, 0 (%%r2) ;"

	    "mtctr %1 ;"
	    "bctrl ;"
	    : /* outputs */
	      "=r" (r3)
	    : /* inputs */
	      "r" (__L4_MemoryControl), "r" (r15), "r" (r16), "r" (r17),
	      "r" (r18), "r" (r19)
	    : /* clobbers */
	      "r0", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
	      "r20", "r21", "r22", "r23", "r24", "r25",
	      "r26", "r27", "r28", "r29", "lr", "ctr",
	      "memory", __L4_PPC_CLOBBER_CR_REGS
	    );

    return r3;
}

#endif	/* __L4__POWERPC__SYSCALLS_H__ */
