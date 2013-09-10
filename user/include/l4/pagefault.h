/*********************************************************************
 *                
 * Copyright (C) 2001, 2002, 2003,  Karlsruhe University
 *                
 * File path:     l4/pagefault.h
 * Description:   Page-fault RPC protocol
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
 * $Id: pagefault.h,v 1.7 2003/09/24 19:06:21 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__PAGEFAULT_H__
#define __L4__PAGEFAULT_H__

#include <l4/message.h>
#include __L4_INC_ARCH(message.h)


#if defined(L4_64BIT)
# define __PLUS32	+ 32
#else
# define __PLUS32
#endif


typedef union L4_PfRequest {
    L4_Word_t	raw[3];
    struct {
#if defined(L4_BIG_ENDIAN)
	unsigned	__padding:12 __PLUS32:	/* -2 */
	unsigned	wrx:4;
	unsigned	__zero:4;
	unsigned	t:6;			/* 0 */
	unsigned	u:6;			/* 2 */
#else
	unsigned	u:6;			/* 2 */
	unsigned	t:6;			/* 0 */
	unsigned	__zero:4;
	unsigned	wrx:4;
	unsigned	__padding:12 __PLUS32:	/* -2 */
#endif
	L4_Word_t	fault_address;
	L4_Word_t	fault_ip;
    } X;
} L4_PfRequest_t;

typedef union L4_PfReply {
    L4_Word_t	raw[3];
    struct {
#if defined(L4_BIG_ENDIAN)
	unsigned	__padding:16 __PLUS32:	/* 0 */
	unsigned	__zero:4;
	unsigned	t:6;			/* 2 */
	unsigned	u:6;			/* 0 */
#else
	unsigned	u:6;			/* 0 */
	unsigned	t:6;			/* 2 */
	unsigned	__zero:4;
	unsigned	__padding:16 __PLUS32:	/* 0 */
#endif
	L4_MapItem_t	MapItem;
    } X;
} L4_PfReply_t;



#undef __PLUS32

#endif /* !__L4__PAGEFAULT_H__ */
