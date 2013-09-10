/*********************************************************************
 *                
 * Copyright (C) 2001-2004,  Karlsruhe University
 *                
 * File path:     l4/space.h
 * Description:   Interfaces for handling address spaces/mappings
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
 * $Id: space.h,v 1.12 2004/03/12 17:50:56 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__SPACE_H__
#define __L4__SPACE_H__

#include <l4/types.h>
#include __L4_INC_ARCH(syscalls.h)

/*
 * Derived functions
 */

L4_INLINE L4_Fpage_t L4_UnmapFpage (L4_Fpage_t f)
{
    L4_LoadMR (0, f.raw);
    L4_Unmap ((L4_Word_t)0);
    L4_StoreMR (0, &f.raw);
    return f;
}

L4_INLINE void L4_UnmapFpages (L4_Word_t n, L4_Fpage_t * fpages)
{
    L4_LoadMRs (0, n, (L4_Word_t *) fpages);
    L4_Unmap (n-1);
    L4_StoreMRs (0, n, (L4_Word_t *) fpages);
}

#if defined(__cplusplus)
L4_INLINE L4_Fpage_t L4_Unmap (L4_Fpage_t f)
{
    return L4_UnmapFpage (f);
}

L4_INLINE void L4_Unmap (L4_Word_t n, L4_Fpage_t * fpages)
{
    L4_UnmapFpages (n, fpages);
}
#endif

L4_INLINE L4_Fpage_t L4_Flush (L4_Fpage_t f)
{
    L4_LoadMR (0, f.raw);
    L4_Unmap (0x40);
    L4_StoreMR (0, &f.raw);
    return f;
}

L4_INLINE void L4_FlushFpages (L4_Word_t n, L4_Fpage_t * fpages)
{
    L4_LoadMRs (0, n, (L4_Word_t *) fpages);
    L4_Unmap (0x40 + n-1);
    L4_StoreMRs (0, n, (L4_Word_t *) fpages);
}

#if defined(__cplusplus)
L4_INLINE void L4_Flush (L4_Word_t n, L4_Fpage_t * f)
{
    L4_FlushFpages (n, f);
}
#endif

L4_INLINE L4_Fpage_t L4_GetStatus (L4_Fpage_t f)
{
    L4_LoadMR (0, f.raw & ~0x0f);
    L4_Unmap ((L4_Word_t) 0);
    L4_StoreMR (0, &f.raw);
    return f;
}

L4_INLINE L4_Bool_t L4_WasWritten (L4_Fpage_t f)
{
    return (f.raw & L4_Writable) != 0;
}

L4_INLINE L4_Bool_t L4_WasReferenced (L4_Fpage_t f)
{
    return (f.raw & L4_Readable) != 0;
}

L4_INLINE L4_Bool_t L4_WaseXecuted (L4_Fpage_t f)
{
    return (f.raw & L4_eXecutable) != 0;
}



#endif /* !__L4__SPACE_H__ */
