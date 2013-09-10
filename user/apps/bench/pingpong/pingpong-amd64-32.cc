/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     bench/pingpong/pingpong-amd64-32.cc
 * Description:   32-bit pong thread for AMD64
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
 * $Id: pingpong-amd64-32.cc,v 1.1 2006/10/21 04:06:30 reichelt Exp $
 *                
 ********************************************************************/
#define __L4_ARCH__ ia32

#include <config.h>
#include <l4/kip.h>
#include <l4/ipc.h>

L4_ThreadId_t ping_tid_32;

void *ipc_32;

L4_INLINE L4_Word_t pingpong_ipc_32 (L4_ThreadId_t dest, L4_Word_t untyped)
{
    L4_Word_t dummy;
    __asm__ __volatile__ (
	"/* pingpong_arch_ipc() */\n"
	"pushl	%%ebp		\n"
	"xorl	%%ecx, %%ecx	\n"
	"movl	%%esi, (%%edi)	\n"
	"call	*ipc_32		\n"	/* jump via KIP */
	"popl	%%ebp		\n"
	: "=d" (dummy), "=S"(untyped)
	: "a" (dest), "d" (dest), "S"(untyped), "D"(__L4_X86_Utcb())
	: "ebx", "ecx" );
    return untyped;
}

void pong_32_thread (void)
{
    L4_Word_t untyped = 0;

    L4_KernelInterfacePage_t * kip =
	(L4_KernelInterfacePage_t *) L4_KernelInterface ();
    ipc_32 = ((char *) kip) + kip->Ipc;

    for (;;)
	untyped = pingpong_ipc_32 (ping_tid_32, untyped);
}
