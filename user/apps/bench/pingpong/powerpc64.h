/*********************************************************************
 *                
 * Copyright (C) 2004, 2008,  Karlsruhe University
 *                
 * File path:     bench/pingpong/powerpc64.h
 * Description:   PowerPC64 specific pingpong functions
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
 * $Id: powerpc64.h,v 1.1 2004/05/10 15:54:14 skoglund Exp $
 *                
 ********************************************************************/
#define HAVE_READ_CYCLES
#define HAVE_ARCH_IPC

#define UTCB_ADDRESS	(0x0fe00000ul)

L4_INLINE L4_Word64_t read_cycles (void)
{
    L4_Word_t ret;

    asm volatile ("mftb %0;" : "=r" (ret)); 

    return (L4_Word64_t) (ret * 8);
}

L4_INLINE L4_Word_t pingpong_ipc (L4_ThreadId_t dest, L4_Word_t untyped)
{
    register L4_Word_t r3 asm("r3") = dest.raw;
    register L4_Word_t r4 asm("r4") = dest.raw;
    register L4_Word_t r5 asm("r5") = 0;
    register L4_Word_t tag asm("r14") = untyped;

    asm volatile (
	"li	0, -32000;	"
	"sc;			"

	: /* outputs */
	 "+r" (r3), "+r" (r4), "+r" (r5), "+r" (tag)
	: /* inputs */
	: /* clobbers */
	 "r0", "ctr", "memory"
    );

    /* Trash the rest. This allows the compiler to choose which
     * inputs to use in the asm code above
     */
    asm volatile (
	"" :::
	 "r0", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
	 "r15", "r16", "r17", "r18", "r19", "r20", "r21",
	 "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29",
	 __L4_PPC64_CLOBBER_CR_REGS
    );

    return tag;		/* mr0 */
}

