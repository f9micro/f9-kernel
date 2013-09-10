/*********************************************************************
 *                
 * Copyright (C) 2001-2004, 2006, 2009,  Karlsruhe University
 *                
 * File path:     l4/amd64/syscalls.h
 * Description:   amd64 syscall implementations
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
 * $Id: syscalls.h,v 1.14 2006/10/19 22:57:40 ud3 Exp $ 
 *                
 ********************************************************************/


#ifndef __L4__AMD64__SYSCALLS_H__
#define __L4__AMD64__SYSCALLS_H__

#include __L4_INC_ARCH(vregs.h)
#include __L4_INC_ARCH(specials.h)
#include <l4/message.h>
#include __L4_INC_ARCH(kdebug.h)


typedef struct {
    L4_Word_t rax;
    L4_Word_t rdx;
} __L4_Return_t;


L4_INLINE void * L4_KernelInterface (L4_Word_t *ApiVersion,
				     L4_Word_t *ApiFlags,
				     L4_Word_t *KernelId)
{
    void * base_address;

    __asm__ __volatile__ (
	"/* L4_KernelInterface() */ 			\n"
	"	lock; nop				\n"

	: /* outputs */
	"=a" (base_address),
	"=c" (*ApiVersion),
	"=d" (*ApiFlags),
	"=S" (*KernelId)

	/* no inputs */
	/* no clobbers */
	);

    return base_address;
}
typedef void (*__L4_Nop_t)(void);

typedef L4_Word_t (*__L4_ExchangeRegisters_t)( L4_Word_t, L4_Word_t, L4_Word_t,
	L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t );

extern __L4_ExchangeRegisters_t __L4_ExchangeRegisters;

L4_INLINE L4_ThreadId_t L4_ExchangeRegisters (L4_ThreadId_t dest,
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
    L4_ThreadId_t       result;
    
    L4_Word_t       dummy;
    struct {
	L4_Word_t       r8;
	L4_Word_t       r9;
	L4_Word_t       r10;
    } args;

    args.r8 = ip;
    args.r9 = flags;
    args.r10 = UserDefHandle;
    
    __asm__ __volatile__ (
	"/* L4_ExchangeRegisters */		\n"
	"movq (%[args]), %%r8			\n"
	"movq 8(%[args]), %%r9			\n"
	"movq 16(%[args]), %%r10		\n"
	"pushq %%rbp				\n"
	"pushq %[args]				\n"
	"callq *__L4_ExchangeRegisters		\n"
	"popq %[args]				\n"
	"popq %%rbp				\n"
	"movq %%r8,   (%[args])	 		\n"
	"movq %%r9,   8(%[args])	        \n"
	"movq %%r10, 16(%[args])		\n"
	: /* outputs */
	"=a" (result),				/* %0 RAX */
	"=c" (dummy),				/* %1 RCX <- R8, R9, R10 */	
	"=d" (*old_sp),				/* %2 RDX */
	"=S" (*old_control),			/* %3 RSI */
	"=D" (*old_pager)			/* %4 RDI */
	: /* inputs */
	[dest]		"0" (dest),		/* %5  RAX */
	[args]		"1" (&args),		/* %6  RCX -> R8, R9, R10 */
	[sp]		"2" (sp),		/* %7  RDX */
	[control]	"3" (control),		/* %8  RSI */	
	[pager]		"4" (pager)		/* %9  RDI */
	: /* clobbers */
	"memory", "rbx", "r8", "r9", "r10",  "r11", "r12", "r13", "r14", "r15"
	);
    
  *old_ip = args.r8;
  *old_flags = args.r9;    
  *old_UserDefhandle = args.r10;
  
  return result;
}

typedef L4_Word_t (*__L4_ThreadControl_t)( L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_ThreadControl_t __L4_ThreadControl;

L4_INLINE L4_Word_t L4_ThreadControl (L4_ThreadId_t dest,
				      L4_ThreadId_t SpaceSpecifier,
				      L4_ThreadId_t Scheduler,
				      L4_ThreadId_t Pager,
				      void * UtcbLocation)
{

    L4_Word_t result = 0;
    L4_Word_t dummy = 0;
    
    __asm__ __volatile__ (
	"/* L4_ThreadControl */			\n"
	"movq %%rax, %%r8			\n"
	"movq %%rcx, %%r9			\n"
	"pushq %%rbp				\n"
	"callq *__L4_ThreadControl		\n"
	"popq %%rbp				\n"
	: /* outputs */
	"=a" (result),				/* %0 RAX */
	"=c" (dummy),				/* %1 RCX */	
	"=d" (dummy),				/* %2 RDX */
	"=S" (dummy),				/* %3 RSI */
	"=D" (dummy)				/* %4 RDI */
	: /* inputs */
	[spacespec]	"0" (SpaceSpecifier),	/* %5  RAX <- R8 */
	[utcblocation]	"1" (UtcbLocation),	/* %6  RCX <- R9 */
	[scheduler]	"2" (Scheduler),	/* %7  RDX */
	[pager]		"3" (Pager),		/* %8  RSI */	
	[dest]		"4" (dest)		/* %9  RDI */
	: /* clobbers */
	  "memory", "rbx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
	);
  
  return result;
}



typedef L4_Clock_t (*__L4_SystemClock_t)( void );
extern __L4_SystemClock_t __L4_SystemClock;

L4_INLINE  L4_Clock_t L4_SystemClock (void)
{
    L4_Clock_t ret;
    
    __asm__ __volatile__ (
	"/* L4_SystemClock */			\n"
	"pushq %%rbp				\n"
	"callq *__L4_SystemClock		\n"
	"popq %%rbp				\n"
	: /* outputs */
	"=a" (ret.raw)				/* %0 RAX */
	: /* inputs */
	[dest]		"0" (ret.raw)		/* %1 RAX */
	: /* clobbers */
	  "memory", "rbx", "rcx",  "rdx", "rsi", "rdi", 
	"r8", "r9", "r10",  "r11", "r12", "r13", "r14", "r15"
	);

    return ret;
}

typedef L4_Word_t (*__L4_ThreadSwitch_t)( L4_Word_t );
extern __L4_ThreadSwitch_t __L4_ThreadSwitch;

L4_INLINE void L4_ThreadSwitch (L4_ThreadId_t dest)
{
    L4_Word_t dummy;
    __asm__ __volatile__ (
	"/* L4_ThreadSwitch */			\n"
	"pushq %%rbp				\n"
	"callq *__L4_ThreadSwitch		\n"
	"popq %%rbp				\n"
	: /* outputs */
	"=D" (dummy)				/* %0 RDI */
	: /* inputs */
	[dest]		"0" (dest)		/* %1  RDI */
	: /* clobbers */
	"memory", "rax", "rbx", "rcx",  "rdx", "rsi", 
	"r8", "r9", "r10",  "r11", "r12", "r13", "r14", "r15"
	);

}



typedef __L4_Return_t (*__L4_Schedule_t)( L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_Schedule_t __L4_Schedule;

L4_INLINE L4_Word_t  L4_Schedule (L4_ThreadId_t dest,
				  L4_Word_t TimeControl,
				  L4_Word_t ProcessorControl,
				  L4_Word_t PrioControl,
				  L4_Word_t PreemptionControl,
				  L4_Word_t * old_TimeControl)
{
    L4_Word_t result = 0;
    L4_Word_t dummy = 0;
    
    __asm__ __volatile__ (
	"/* L4_Schedule */			\n"
	"movq %%rax, %%r8			\n"
	"movq %%rcx, %%r9			\n"
	"pushq %%rbp				\n"
	"callq *__L4_Schedule			\n"
	"popq %%rbp				\n"
	: /* outputs */
	"=a" (result),				/* %0 RAX -> R8 */
	"=c" (dummy),				/* %1 RCX -> R9*/	
	"=d" (*old_TimeControl),		/* %2 RDX */
	"=S" (dummy),				/* %3 RSI */
	"=D" (dummy)				/* %4 RDI */
	: /* inputs */
	[procontrol]	"0" (ProcessorControl),	/* %5  RAX <- R8 */
	[precontrol]	"1" (PreemptionControl),/* %6  RCX <- R9 */
	[tcontrol]	"2" (TimeControl),	/* %7  RDX */
	[PrioControl]	"3" (PrioControl),	/* %8  RSI */	
	[dest]		"4" (dest)		/* %9  RDI */
	: /* clobbers */
	"memory", "rbx","r8", "r9", "r10",  "r11", "r12", "r13", "r14", "r15"
	);
  
  return result;
}


typedef L4_Word_t (*__L4_Ipc_t)( L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_Ipc_t __L4_Ipc;

L4_INLINE L4_MsgTag_t L4_Ipc (L4_ThreadId_t to,
			      L4_ThreadId_t FromSpecifier,
			      L4_Word_t Timeouts,
			      L4_ThreadId_t * from)
{

    L4_ThreadId_t result;
    L4_Word_t * utcb = __L4_X86_Utcb ();
    
    L4_Word_t dummy;

    
    __asm__ __volatile__ (
	"/* L4_Ipc() */				\n"
	"movq	%[timeouts], %%r8		\n"
	"movq	%%rcx, %%r9			\n"
	"movq	 8(%[utcb]), %%rax		\n" 
	"movq	16(%[utcb]), %%rbx		\n" 
	"movq	24(%[utcb]), %%r10		\n" 
	"movq	32(%[utcb]), %%r12		\n" 
	"movq	40(%[utcb]), %%r13		\n" 
	"movq	48(%[utcb]), %%r14		\n" 
	"movq	56(%[utcb]), %%r15		\n" 
	"pushq	%%rbp				\n"
	"callq	*__L4_Ipc			\n"
	"popq	%%rbp				\n"
	"movq	%%r10, 24(%[utcb]) 		\n" 
	"movq	%%r12, 32(%[utcb]) 		\n" 
	"movq	%%r13, 40(%[utcb]) 		\n" 
	"movq	%%r14, 48(%[utcb]) 		\n" 
	"movq	%%r15, 56(%[utcb]) 		\n" 
	"movq	%%r9, %%rcx			\n"
	: /* outputs */
	"=a" (utcb[1]),				/* %0 RAX */
	"=b" (utcb[2]),				/* %1 RBX */			
	"=c" (utcb[0]),				/* %2 RCX <- R09*/	
	"=d" (dummy),				/* %3 RDX */
	"=S" (result)				/* %4 RSI */
	
	: /* inputs */
	[to]		"4" (to),		/* %5  RSI */
	[utcb]		"D" (utcb),		/* %6  RDI */
	[from]		"3" (FromSpecifier),	/* %7  RDX */
	[utcb0]		"2" (utcb[0]),		/* %8  RCX -> R09 */
	[timeouts]	"r" (Timeouts)		/* %9  REG -> R08 */
	: /* clobbers */
	  "memory", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
	);

    if (! L4_IsNilThread (FromSpecifier)) {
	*from = result;
    }
    
    return * (L4_MsgTag_t *) (&utcb[0]);
   
}

typedef L4_Word_t (*__L4_Lipc_t)( L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_Lipc_t __L4_Lipc;

L4_INLINE L4_MsgTag_t L4_Lipc (L4_ThreadId_t to,
			       L4_ThreadId_t FromSpecifier,
			       L4_Word_t Timeouts,
			       L4_ThreadId_t * from)
{
    L4_ThreadId_t result;
    L4_Word_t * utcb = __L4_X86_Utcb ();
    L4_Word_t dummy;

    
    __asm__ __volatile__ (
	"/* L4_Ipc() */				\n"
	"movq	%[timeouts], %%r8		\n"
	"movq	%[utcb0], %%r9			\n"
	"movq	 8(%[utcb]), %%rax		\n" 
	"movq	16(%[utcb]), %%rbx		\n" 
	"movq	24(%[utcb]), %%r10		\n" 
	"movq	32(%[utcb]), %%r12		\n" 
	"movq	40(%[utcb]), %%r13		\n" 
	"movq	48(%[utcb]), %%r14		\n" 
	"movq	56(%[utcb]), %%r15		\n" 
	"pushq	%%rbp				\n"
	"callq	*__L4_Lipc			\n"
	"popq	%%rbp				\n"
	"movq	%%r9, %[utcb0]			\n"
	"movq	%%r10, 24(%[utcb]) 		\n" 
	"movq	%%r12, 32(%[utcb]) 		\n" 
	"movq	%%r13, 40(%[utcb]) 		\n" 
	"movq	%%r14, 48(%[utcb]) 		\n" 
	"movq	%%r15, 56(%[utcb]) 		\n" 
	: /* outputs */
	"=a" (utcb[1]),				/* %0 RAX */
	"=b" (utcb[2]),				/* %1 RBX */			
	"=c" (utcb[0]),				/* %2 RCX <- R09*/	
	"=d" (dummy),				/* %3 RDX */
	"=S" (result)				/* %4 RSI */
	
	: /* inputs */
	[to]		"4" (to),		/* %5  RSI */
	[utcb]		"D" (utcb),		/* %6  RDI */
	[from]		"3" (FromSpecifier),	/* %7  RDX */
	[utcb0]		"2" (utcb[0]),		/* %8  RCX -> R09 */
	[timeouts]	"r" (Timeouts)		/* %9  REG -> R08 */
	: /* clobbers */
	  "memory", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
	);

    if (! L4_IsNilThread (FromSpecifier)) {
	*from = result;
    }
    
    return * (L4_MsgTag_t *) (&utcb[0]);

   
}


typedef void (*__L4_Unmap_t)( L4_Word_t );
extern __L4_Unmap_t __L4_Unmap;

L4_INLINE void L4_Unmap (L4_Word_t control)
{   
    
    L4_Word_t * utcb = __L4_X86_Utcb ();
    L4_Word_t dummy;
    
    __asm__ __volatile__ (
	"/* L4_Unmap() */			\n"
	"movq	%[utcb0], %%r9			\n"
	"movq	24(%[utcb]), %%r10		\n" 
	"movq	32(%[utcb]), %%r12		\n" 
	"movq	40(%[utcb]), %%r13		\n" 
	"movq	48(%[utcb]), %%r14		\n" 
	"movq	56(%[utcb]), %%r15		\n" 
	"pushq	%%rbp				\n"
	"callq	*__L4_Unmap			\n"
	"popq	%%rbp				\n"
	"movq	%%r10, 24(%[utcb]) 		\n" 
	"movq	%%r12, 32(%[utcb]) 		\n" 
	"movq	%%r13, 40(%[utcb]) 		\n" 
	"movq	%%r14, 48(%[utcb]) 		\n" 
	"movq	%%r15, 56(%[utcb]) 		\n" 
	"movq	%%r9, %[utcb0]			\n"
	: /* outputs */
	  "=a" (utcb[1]),				/* %0 RAX */
	  "=b" (utcb[2]),				/* %1 RBX */			
	  "=c" (utcb[0]),				/* %2 RCX <- R09*/	
	  "=d" (dummy),	        			/* %3 RDX */
	  "=D" (utcb)	        			/* %4 RDI */
	  
	: /* inputs */
	  [utcb1]	"0" (utcb[1]),			/* %5 RAX */
	  [utcb2]	"1" (utcb[2]),			/* %6 RBX */
	  [utcb0]	"2" (utcb[0]),			/* %7 RCX -> R09 */
	  [control]	"3" (control),        		/* %8 RDX */
	  [utcb]	"4" (utcb)	       		/* %9 RDI */
	: /* clobbers */
	  "memory", "rsi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
	);

}

typedef __L4_Return_t (*__L4_SpaceControl_t)( L4_Word_t, L4_Word_t, L4_Word_t,
	L4_Word_t, L4_Word_t );
extern __L4_SpaceControl_t __L4_SpaceControl;

L4_INLINE L4_Word_t L4_SpaceControl (L4_ThreadId_t SpaceSpecifier,
				     L4_Word_t control,
				     L4_Fpage_t KernelInterfacePageArea,
				     L4_Fpage_t UtcbArea,
				     L4_ThreadId_t redirector,
				     L4_Word_t *old_control)
{
    L4_Word_t result = 0;
    L4_Word_t dummy = 0;
    
    __asm__ __volatile__ (
	"/* L4_SpaceControl */			\n"
	"movq %%rax, %%r8			\n"
	"movq %%rcx, %%r9			\n"
	"pushq %%rbp				\n"
	"callq *__L4_SpaceControl		\n"
	"popq %%rbp				\n"
	: /* outputs */
	"=a" (result),					/* %0 RAX */
	"=c" (dummy), 	  	  		  	/* %1 RCX */	
	"=d" (*old_control),  	  		  	/* %2 RDX */
	"=S" (dummy), 	  	  		  	/* %3 RSI */
	"=D" (dummy)  	  	  		  	/* %4 RDI */
	: /* inputs */
	[utcbarea]	"0" (UtcbArea),			/* %5  RAX -> R8 */
	[redirector]	"1" (redirector),		/* %6  RCX -> R9 */
	[kiparea]	"2" (KernelInterfacePageArea),	/* %7  RDX */
	[control]	"3" (control),			/* %8  RSI */	
	[spacespec]	"4" (SpaceSpecifier)		/* %9  RDI */
	: /* clobbers */
	"memory", "rbx", "r8", "r9", "r10",  "r11", "r12", "r13", "r14", "r15"
	);
  
  return result;
}

typedef L4_Word_t (*__L4_ProcessorControl_t)( L4_Word_t,  L4_Word_t,
	L4_Word_t, L4_Word_t );
extern __L4_ProcessorControl_t __L4_ProcessorControl;


L4_INLINE L4_Word_t L4_ProcessorControl (L4_Word_t ProcessorNo,
					 L4_Word_t InternalFrequency,
					 L4_Word_t ExternalFrequency,
					 L4_Word_t voltage)
{
    L4_Word_t result;
    L4_Word_t dummy;
    __asm__ __volatile__ (
	"/* L4_ProcessorControl */		\n"
	"movq %%rax, %%r8			\n"
	"pushq %%rbp				\n"
	"callq *__L4_ProcessorControl		\n"
	"popq %%rbp				\n"
	: /* outputs */
	"=a" (result),				/* %0 RAX  */
	"=d" (dummy),				/* %1 RDX */
	"=S" (dummy),				/* %2 RSI */
	"=D" (dummy)				/* %3 RDI */
	: /* inputs */
	[voltage]	"0" (voltage),		/* %4  RAX -> R8 */
	[exfreq]	"1" (ExternalFrequency),/* %5  RDX */
	[intfreq]	"2" (InternalFrequency),/* %6  RSI */	
	[procno]	"3" (ProcessorNo)	/* %7  RDI */
	: /* clobbers */
	"memory", "rcx", "rbx", "r8", "r9", "r10",  "r11", "r12", "r13", "r14", "r15"
	);
  
  return result;
}

typedef L4_Word_t (*__L4_MemoryControl_t)( L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_MemoryControl_t __L4_MemoryControl;

L4_INLINE L4_Word_t L4_MemoryControl (L4_Word_t control,
				 const L4_Word_t * attributes)
{
    L4_Word_t result;
    L4_Word_t dummy;
    L4_Word_t * utcb = __L4_X86_Utcb ();

    __asm__ __volatile__ (
	"/* L4_MemoryControl */			\n"
	"movq   (%[attributes]), %%rcx		\n"
	"movq  8(%[attributes]), %%rsi		\n"
	"movq 16(%[attributes]), %%r8		\n"
	"movq 24(%[attributes]), %%r11		\n"
	"movq	%[utcb0], %%r9			\n"
	"movq	16(%[utcb]), %%rbx		\n" 
	"movq	24(%[utcb]), %%r10		\n" 
	"movq	32(%[utcb]), %%r12		\n" 
	"movq	40(%[utcb]), %%r13		\n" 
	"movq	48(%[utcb]), %%r14		\n" 
	"movq	56(%[utcb]), %%r15		\n" 
	"pushq	%%rbp				\n"
	"callq	*__L4_MemoryControl		\n"
	"popq	%%rbp				\n"
	
	: /* outputs */
	  "=a" (utcb[1]),				/* %0 RAX */
	  "=b" (dummy),					/* %1 RBX */	
	  "=c" (utcb[0]),				/* %2 RCX <- R09*/	
	  "=d" (result),	        		/* %3 RDX */
	  "=D" (utcb)	        			/* %4 RDI */
	  
	: /* inputs */
	  [utcb1]	"0" (utcb[1]),			/* %5 RAX */
	  [attributes]	"1" (attributes),		/* %6 RBX -> RCX, RSI, R8, R11 */
	  [utcb0]	"2" (utcb[0]),			/* %7 RCX -> R09 */
	  [control]	"3" (control),        		/* %8 RDX */
	  [utcb]	"4" (utcb)	       		/* %9 RDI */
	: /* clobbers */

	  "memory", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
	);

    return result;
}



#endif /* !__L4__AMD64__SYSCALLS_H__ */
