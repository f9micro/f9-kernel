/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Copyright (c) 2002-2003, 2005-2007, 2010 Karlsruhe University.
 * 	All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __L4_PLATFORM_SPECIALS_H__
#define __L4_PLATFORM_SPECIALS_H__

L4_INLINE int __L4_Msb(L4_Word_t w) __attribute__((const));

L4_INLINE int __L4_Msb(L4_Word_t w)
{
	int zeros;
	__asm__ __volatile__(
	    "clz %0, %1\n"
	    : /* outputs */ "=r"(zeros)
	    : /* inputs */ "r"(w)
	);
	return 31 - zeros;
}

L4_INLINE int __L4_Lsb(L4_Word_t w) __attribute__((const));

L4_INLINE int __L4_Lsb(L4_Word_t w)
{
	L4_Word_t bitnum;
	__asm__ __volatile__(
	    "bsf %1, %0\n"
	    : /* outputs */ "=r"(bitnum)
	    : /* inputs */ "rm"(w)
	);
	return bitnum;
}

#endif	/* __L4_PLATFORM_SPECIALS_H__ */

#if 0	/* FIXME: IA32 specific implementation */

#ifndef __L4__IA32__SPECIALS_H__
#define __L4__IA32__SPECIALS_H__

/*
 * Architecture specific helper functions.
 */

L4_INLINE void __L4_Inc_Atomic(L4_Word_t *w)
{
	__asm__ __volatile__(
	    "/* l4_inc_atomic()	*/\n"
	    "lock; add $1, %0"
	    : "=m"(w));
}


L4_INLINE L4_Word64_t __L4_Rdtsc(void)
{
	L4_Word_t eax, edx;

	__asm__ __volatile__(
	    "/* l4_rdtsc()		*/			\n"
	    "rdtsc"
	    : "=a"(eax), "=d"(edx));

	return (((L4_Word64_t)edx) << 32) | (L4_Word64_t)eax;
}

L4_INLINE L4_Word64_t __L4_Rdpmc(const L4_Word_t ctrsel)
{
	L4_Word_t eax, edx;

	__asm__ __volatile__(
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

L4_INLINE L4_Word_t L4_SmallSpace(L4_Word_t location, L4_Word_t size)
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

#endif	/* IA32 specific implementation */
