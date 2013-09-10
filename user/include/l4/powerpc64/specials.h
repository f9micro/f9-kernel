/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	l4/powerpc/specials.h
 * Description:	PowerPC specific functions and defines.
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
 * $Id: specials.h,v 1.2 2003/09/24 19:06:26 skoglund Exp $
 *
 ***************************************************************************/
#ifndef __L4__POWERPC__SPECIALS_H__
#define __L4__POWERPC__SPECIALS_H__

L4_INLINE int __L4_Msb( L4_Word_t w ) __attribute__ ((const));

L4_INLINE int __L4_Msb( L4_Word_t w )
{
    int zeros;

    asm volatile ("cntlzd %0, %1" : "=r" (zeros) : "r" (w) );

    return 63-zeros;
}

#if defined(__cplusplus)
L4_INLINE L4_Fpage_t L4_Fpage (L4_Fpage_t f)
{
        L4_Fpage_t out;
	    out.raw = f.raw;
	        return out;
}
#endif

#endif	/* __L4__POWERPC__SPECIALS_H__ */
