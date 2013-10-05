/* Copyright (c) 2001, 2002, 2003 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
    L4_Word_t raw[3];
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
    L4_Word_t raw[3];
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
	L4_MapItem_t MapItem;
    } X;
} L4_PfReply_t;

#undef __PLUS32

#endif /* !__L4__PAGEFAULT_H__ */
