/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     bench/pingpong/powerpc.h
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
#define HAVE_READ_CYCLES
#define HAVE_READ_INSTRS
#define HAVE_ARCH_IPC
#define HAVE_ARCH_LIPC
#define HAVE_ARCH_SPECIFIC

#undef ARCH_PMC_SETUP

L4_INLINE L4_Word64_t read_cycles (void)
{
    L4_Word_t ret;
    /* We assume that the kernel configured pmc1 for cycle counting. */
    asm volatile ("mfspr %0, %1" : "=r" (ret) : "i" (937 /* upmc1 */));
    return (L4_Word64_t) ret;
}

L4_INLINE L4_Word_t read_instrs (void)
{
    L4_Word_t ret;
    /* We assume that the kernel configured pmc2 for cycle counting. */
    asm volatile ("mfspr %0, %1" : "=r" (ret) : "i" (938 /* upmc2 */));
    return ret;
}

L4_INLINE L4_Word_t pingpong_ipc (L4_ThreadId_t dest, L4_Word_t untyped)
{
    register L4_Word_t tag asm("r14") = untyped;
    register L4_Word_t to asm("r15") = dest.raw;
    register L4_Word_t from asm("r16") = dest.raw;
    register L4_Word_t timeouts asm("r17") = 0;

    asm volatile (
	    "mtctr %4 ;"
	    "bctrl ;"
	    : /* outputs */
	      "+r" (to), "+r" (tag)
	    : /* inputs */
	      "r" (from), "r" (timeouts), "r" (__L4_Ipc)
	    : /* clobbers */
              "r0", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11",
      	      "r12", "r13",
	      "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25",
	      "r26", "r27", "r28", "r29", "lr", "ctr",
	      "memory", __L4_PPC_CLOBBER_CR_REGS
	    );

    return tag;
}

L4_INLINE L4_Word_t pingpong_lipc (L4_ThreadId_t dest, L4_Word_t untyped)
{
    register L4_Word_t tag asm("r14") = untyped;
    register L4_Word_t to asm("r15") = dest.raw;
    register L4_Word_t from asm("r16") = dest.raw;
    register L4_Word_t timeouts asm("r17") = 0;

    asm volatile (
	    "mtctr %4 ;"
	    "bctrl ;"
	    : /* outputs */
	      "+r" (to), "+r" (tag)
	    : /* inputs */
	      "r" (from), "r" (timeouts), "r" (__L4_Lipc)
	    : /* clobbers */
              "r0", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11",
      	      "r12", "r13",
	      "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25",
	      "r26", "r27", "r28", "r29", "lr", "ctr",
	      "memory", __L4_PPC_CLOBBER_CR_REGS
	    );

    return tag;
}

void arch_specific (void)
{
    L4_Word_t cycles1, cycles2, instrs1, instrs2;
    L4_Clock_t usec1, usec2;
    int tot = 1000;

    usec1 = L4_SystemClock();
    cycles1 = read_cycles();
    instrs1 = read_instrs();

    asm volatile ( 
	    "mr %%r3, %0 ;"
	    "mtctr %1 ;"

	    "0: mtlr %%r3 ; "
	    "blrl ;"
	    "bdnz 0b ;"
	    :
	    : "r" (kip->ArchSyscall3), "r" (tot)
	    : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
	    __L4_PPC_CLOBBER_REGS, "memory", __L4_PPC_CLOBBER_CR_REGS 
	    );

    usec2 = L4_SystemClock();
    cycles2 = read_cycles();
    instrs2 = read_instrs();

    printf( "\nNULL system call: %.2f cycles, %.2f usec, %.2f instrs\n",
	    ((double) (cycles2-cycles1))/tot,
	    ((double) (usec2.raw-usec1.raw))/tot,
	    ((double) (instrs2-instrs1))/tot );
}

