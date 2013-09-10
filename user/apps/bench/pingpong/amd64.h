/*********************************************************************
 *                
 * Copyright (C) 2004-2008, 2010,  Karlsruhe University
 *                
 * File path:     bench/pingpong/amd64.h
 * Description:   AMD64 specific pingpong functions
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
 * $Id: amd64.h,v 1.7 2006/10/21 04:09:35 reichelt Exp $
 *                
 ********************************************************************/
#include <l4/arch.h>

#include L4_COMPAT_H_LOCATION


#define HAVE_ARCH_SPECIFIC
#define HAVE_ARCH_IPC
#define HAVE_HANDLE_ARCH_PAGEFAULT
#define HAVE_READ_CYCLES

#define L4_REQUEST_MASK		( ~((~0UL) >> ((sizeof (L4_Word_t) * 8) - 20)))

#define L4_IO_PAGEFAULT		(-8UL << 20)
#define L4_IO_PORT_START	(0)
#define L4_IO_PORT_END		(1<<16)


L4_INLINE L4_Word64_t read_cycles(void)
{
    L4_Word32_t eax, edx;

    __asm__ __volatile__ (
            "rdtsc"
            : "=a"(eax), "=d"(edx));

    return (((L4_Word64_t)edx) << 32) | (L4_Word64_t)eax;
}


L4_Fpage_t handle_arch_pagefault (L4_MsgTag_t tag, L4_Word_t faddr, L4_Word_t fip, L4_Word_t log2size)
{
    // If pagefault is an IO-Page, return that IO Fpage 
    L4_Fpage_t fp;
    fp.raw = faddr;
    if ((tag.raw & L4_REQUEST_MASK) == L4_IO_PAGEFAULT && L4_IsIoFpage(fp))
    {
	return fp;
    }
	
	
    return L4_FpageLog2 (faddr, log2size);
}


extern L4_ThreadId32_t ping_tid_32;
extern void pong_32_thread (void);

void arch_specific (void)
{
#if 0
    L4_Word64_t cycles1, cycles2;
    L4_Clock_t usec1, usec2;
    L4_Word_t tot = 1000, i;

    i = tot;
    
    usec1 = L4_SystemClock();
    cycles1 = read_cycles();

    while (i--)
	L4_Nop();
    
    usec2 = L4_SystemClock();
    cycles2 = read_cycles();

    
    printf( "\nNULL system call: %ld cycles1, %ld usec\n",
	    ((cycles2-cycles1)/tot),((usec2-usec1).raw)/tot);
#endif

    L4_Word_t control;
    L4_Msg_t msg;

    INTER_AS=1; MIGRATE=0; SMALL_AS=0; LIPC=0;

    ping_tid_32 = L4_ThreadId32 (ping_tid);

    printf("\nIPC between 32-bit and 64-bit threads:\n\n");

    L4_ThreadControl (ping_tid, ping_tid, roottid,
		      L4_nilthread, UTCB(0));
    L4_ThreadControl (pong_tid, pong_tid, roottid, 
		      L4_nilthread, UTCB(1));
    L4_SpaceControl (ping_tid, 0, kip_area, utcb_area, L4_nilthread,
		     &control);
    L4_SpaceControl (pong_tid, 1ULL << 63, kip_area, utcb_area, L4_nilthread,
		     &control);
    if (control & (1ULL << 63))
    {
	L4_ThreadControl (ping_tid, ping_tid, roottid, pager_tid, 
			  NOUTCB);
	L4_ThreadControl (pong_tid, pong_tid, roottid, pager_tid, 
			  NOUTCB);

	L4_Clear (&msg);
	L4_Append (&msg, START_ADDR (ping_thread));
	L4_Append (&msg, START_ADDR (pong_32_thread));
	L4_Load (&msg);
	L4_Send (pager_tid);

	L4_Receive (ping_tid);
    }
    else
	printf("Compatibility Mode support not compiled in.\n");

    L4_ThreadControl (ping_tid, L4_nilthread, L4_nilthread, 
		      L4_nilthread, NOUTCB);
    L4_ThreadControl (pong_tid, L4_nilthread, L4_nilthread, 
		      L4_nilthread, NOUTCB);
}

L4_INLINE L4_Word_t pingpong_ipc (L4_ThreadId_t dest, L4_Word_t untyped)
{
    L4_Word_t dummy;
    __asm__ __volatile__ (
	"/* pingpong_arch_ipc() */\n"
	"movq   %%rax, %%r9	\n"
	"xorq	%%r8, %%r8	\n"
	"pushq	%%rbp		\n"
	"call	*__L4_Ipc	\n"	/* jump via KIP */
	"popq	%%rbp		\n"
	"movq   %%r9, %%rax	\n"
	: "=S" (dummy), 
	  "=a" (untyped),
	  "=d" (dest)
	: "a" (untyped), 
	  "d" (dest), 
	  "S" (dest), 
	  "D" (__L4_X86_Utcb())
	: "rbx", "rcx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");

    return untyped;
}

