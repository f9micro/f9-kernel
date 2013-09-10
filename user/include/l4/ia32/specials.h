/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2005-2007, 2010,  Karlsruhe University
 *                
 * File path:     l4/ia32/specials.h
 * Description:   x86 specific functions and defines
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
 * $Id: specials.h,v 1.12 2006/11/17 16:48:17 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__IA32__SPECIALS_H__
#define __L4__IA32__SPECIALS_H__

/*
 * Architecture specific helper functions.
 */

L4_INLINE void __L4_Inc_Atomic (L4_Word_t *w)
{
    __asm__ __volatile__(
        "/* l4_inc_atomic()	*/\n"
        "lock; add $1, %0" 
        : "=m"(w));
}


L4_INLINE int __L4_Msb (L4_Word_t w) __attribute__ ((const));

L4_INLINE int __L4_Msb (L4_Word_t w)
{
    int bitnum;

    __asm__ (
	"/* l4_msb()		*/			\n"
	"	bsr	%1, %0				\n"

	: /* outputs */
	"=r" (bitnum)
	: /* inputs */
	"rm" (w)
	);

    return bitnum;
}

L4_INLINE int __L4_Lsb (L4_Word_t w) __attribute__ ((const));

L4_INLINE int __L4_Lsb (L4_Word_t w)
{
    L4_Word_t bitnum;

    __asm__ (
	"/* l4_lsb()		*/			\n"
	"bsf	%1, %0					\n"
	: /* outputs */
	"=r" (bitnum)
	: /* inputs */
	"rm" (w)
	);

    return bitnum;
}

L4_INLINE L4_Word64_t __L4_Rdtsc ()
{
    L4_Word_t eax, edx;

    __asm__ __volatile__ (
	"/* l4_rdtsc()		*/			\n"
        "rdtsc"
        : "=a"(eax), "=d"(edx));

    return (((L4_Word64_t)edx) << 32) | (L4_Word64_t)eax;
}

L4_INLINE L4_Word64_t __L4_Rdpmc (const L4_Word_t ctrsel)
{
    L4_Word_t eax, edx;

    __asm__ __volatile__ (
	"/* l4_rdpmc()		*/			\n"
        "rdpmc"
        : "=a"(eax), "=d"(edx)
        : "c"(ctrsel));
    
    return (((L4_Word64_t)edx) << 32) | (L4_Word64_t)eax;
}

/*
 * Control parameter for SpaceControl system call.
 */

#define L4_LargeSpace		0

L4_INLINE L4_Word_t L4_SmallSpace (L4_Word_t location, L4_Word_t size)
{
    location >>= 1;
    size >>= 2;
    return ((location & ~(size - 1)) | size) & 0xff;
}




/*
 * Cacheability hints for string items.
 */

#define L4_AllocateNewCacheLines	(L4_CacheAllocationHint_t) { raw: 1 })
#define L4_DoNotAllocateNewCacheLines	(L4_CacheAllocationHint_t) { raw: 2 })
#define L4_AllocateOnlyNewL1CacheLines	(L4_CacheAllocationHint_t) { raw: 3 })




/*
 * Memory attributes for MemoryControl system call.
 */

#define L4_WriteBackMemory		1
#define L4_WriteThroughMemory		2
#define L4_UncacheableMemory		4
#define L4_WriteCombiningMemory		5
#define L4_WriteProtectedMemory		8



#endif /* !__L4__IA32__SPECIALS_H__ */
