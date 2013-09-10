/*********************************************************************
 *                
 * Copyright (C) 2001, 2002, 2003,  Karlsruhe University
 *                
 * File path:     l4/misc.h
 * Description:   Miscelaneous interfaces
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
 * $Id: misc.h,v 1.4 2003/11/17 06:21:55 cvansch Exp $
 *                
 ********************************************************************/
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

L4_INLINE L4_Word_t L4_Set_PageAttribute (L4_Fpage_t f, L4_Word_t attribute)
{
    L4_Word_t attributes[4];
    attributes[0] = attribute;

    L4_Set_Rights (&f, 0);			/* Set a to 0 */
    L4_LoadMR (0, f.raw);
    return L4_MemoryControl (0, attributes);
}

L4_INLINE L4_Word_t L4_Set_PageAttributes (L4_Word_t n, L4_Fpage_t * f,
				      const L4_Word_t * attributes)
{
    L4_LoadMRs (0, n, (L4_Word_t *) f);
    return L4_MemoryControl (n - 1, attributes);
}

#endif /* !__L4__MISC_H__ */
