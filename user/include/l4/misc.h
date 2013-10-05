/* Copyright (c) 2001, 2002, 2003 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __L4__MISC_H__
#define __L4__MISC_H__

#include <l4/types.h>
#include __L4_INC_ARCH(syscalls.h)

/*
 * Memory attributes
 */
#define L4_DefaultMemory	0

/*
 * Derived functions
 */
L4_INLINE L4_Word_t L4_Set_PageAttribute(L4_Fpage_t f, L4_Word_t attribute)
{
	L4_Word_t attributes[4];
	attributes[0] = attribute;

	L4_Set_Rights(&f, 0);			/* Set a to 0 */
	L4_LoadMR(0, f.raw);
	return L4_MemoryControl(0, attributes);
}

L4_INLINE L4_Word_t L4_Set_PageAttributes(
		L4_Word_t n, L4_Fpage_t *f,
		const L4_Word_t *attributes)
{
	L4_LoadMRs (0, n, (L4_Word_t *) f);
	return L4_MemoryControl (n - 1, attributes);
}

#endif /* !__L4__MISC_H__ */
