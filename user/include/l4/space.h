/* Copyright (c) 2001-2004 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __L4__SPACE_H__
#define __L4__SPACE_H__

#include <l4/types.h>
#include __L4_INC_ARCH(syscalls.h)

/*
 * Derived functions
 */
L4_INLINE L4_Fpage_t L4_UnmapFpage(L4_Fpage_t f)
{
	L4_LoadMR(0, f.raw);
	L4_Unmap((L4_Word_t)0);
	L4_StoreMR(0, &f.raw);
	return f;
}

L4_INLINE void L4_UnmapFpages(L4_Word_t n, L4_Fpage_t *fpages)
{
	L4_LoadMRs(0, n, (L4_Word_t *) fpages);
	L4_Unmap(n - 1);
	L4_StoreMRs(0, n, (L4_Word_t *) fpages);
}

#if defined(__cplusplus)
L4_INLINE L4_Fpage_t L4_Unmap(L4_Fpage_t f)
{
	return L4_UnmapFpage(f);
}

L4_INLINE void L4_Unmap(L4_Word_t n, L4_Fpage_t *fpages)
{
	L4_UnmapFpages(n, fpages);
}
#endif

L4_INLINE L4_Fpage_t L4_Flush(L4_Fpage_t f)
{
	L4_LoadMR(0, f.raw);
	L4_Unmap(0x40);
	L4_StoreMR(0, &f.raw);
	return f;
}

L4_INLINE void L4_FlushFpages(L4_Word_t n, L4_Fpage_t *fpages)
{
	L4_LoadMRs(0, n, (L4_Word_t *) fpages);
	L4_Unmap(0x40 + n - 1);
	L4_StoreMRs(0, n, (L4_Word_t *) fpages);
}

#if defined(__cplusplus)
L4_INLINE void L4_Flush(L4_Word_t n, L4_Fpage_t *f)
{
	L4_FlushFpages(n, f);
}
#endif

L4_INLINE L4_Fpage_t L4_GetStatus(L4_Fpage_t f)
{
	L4_LoadMR(0, f.raw & ~0x0f);
	L4_Unmap((L4_Word_t) 0);
	L4_StoreMR(0, &f.raw);
	return f;
}

L4_INLINE L4_Bool_t L4_WasWritten(L4_Fpage_t f)
{
	return (f.raw & L4_Writable) != 0;
}

L4_INLINE L4_Bool_t L4_WasReferenced(L4_Fpage_t f)
{
	return (f.raw & L4_Readable) != 0;
}

L4_INLINE L4_Bool_t L4_WaseXecuted(L4_Fpage_t f)
{
	return (f.raw & L4_eXecutable) != 0;
}

#endif /* !__L4__SPACE_H__ */
