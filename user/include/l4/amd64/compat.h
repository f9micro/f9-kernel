/*********************************************************************
 *                
 * Copyright (C) 2006-2007,  Karlsruhe University
 *                
 * File path:     l4/amd64/compat.h
 * Description:   L4 types for inter-architecture compatibility
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
 * $Id: compat.h,v 1.1 2006/10/21 03:56:20 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __L4__AMD64__COMPAT_H__
#define __L4__AMD64__COMPAT_H__

#include <l4/types.h>


/*
 * Thread IDs
 */

typedef union {
    L4_Word32_t	raw;
    struct {
	L4_BITFIELD2(L4_Word32_t,
		version : 14,
		thread_no : 18);
    } X;
} L4_GthreadId32_t;

typedef union {
    L4_Word32_t	raw;
    struct {
	L4_BITFIELD2(L4_Word32_t,
		__zeros : 6,
		local_id : 26);
    } X;
} L4_LthreadId32_t;

typedef union {
    L4_Word32_t	raw;
    L4_GthreadId32_t	global;
    L4_LthreadId32_t	local;
} L4_ThreadId32_t;


#define L4_nilthread32	    ((L4_ThreadId32_t) { raw : 0UL})
#define L4_anythread32	    ((L4_ThreadId32_t) { raw : (L4_Word32_t) ~0UL})
#define L4_anylocalthread32 ((L4_ThreadId32_t) { local : { X : {L4_SHUFFLE2(0, ((1UL<<(26))-1)) }}})

L4_INLINE L4_ThreadId32_t L4_GlobalId32 (L4_Word32_t threadno, L4_Word32_t version)
{
    L4_ThreadId32_t t;
    t.global.X.thread_no = threadno;
    t.global.X.version = version;
    return t;
}

L4_INLINE L4_Word32_t L4_Version32 (L4_ThreadId32_t t)
{
    return t.global.X.version;
}

L4_INLINE L4_Word32_t L4_ThreadNo32 (L4_ThreadId32_t t)
{
    return t.global.X.thread_no;
}

L4_INLINE L4_Bool_t L4_IsThreadEqual32 (const L4_ThreadId32_t l,
					const L4_ThreadId32_t r)
{
    return l.raw == r.raw;
}

L4_INLINE L4_Bool_t L4_IsThreadNotEqual32 (const L4_ThreadId32_t l,
					   const L4_ThreadId32_t r)
{
    return l.raw != r.raw;
}

#if defined(__cplusplus)
static inline L4_Bool_t operator == (const L4_ThreadId32_t & l,
				     const L4_ThreadId32_t & r)
{
    return l.raw == r.raw;
}

static inline L4_Bool_t operator != (const L4_ThreadId32_t & l,
				     const L4_ThreadId32_t & r)
{
    return l.raw != r.raw;
}
#endif /* __cplusplus */

L4_INLINE L4_Bool_t L4_IsNilThread32 (L4_ThreadId32_t t)
{
    return t.raw == 0;
}

L4_INLINE L4_Bool_t L4_IsLocalId32 (L4_ThreadId32_t t)
{
    return t.local.X.__zeros == 0;
}

L4_INLINE L4_Bool_t L4_IsGlobalId32 (L4_ThreadId32_t t)
{
    return t.local.X.__zeros != 0;
}


L4_INLINE L4_ThreadId32_t L4_ThreadId32 (L4_ThreadId_t id)
{
    if (L4_IsThreadEqual(id, L4_anythread)) {
	return L4_anythread32;
    } else if (L4_IsLocalId(id)) {
	if (L4_IsThreadEqual(id, L4_anylocalthread)) {
	    return L4_anylocalthread32;
	} else {
	    L4_ThreadId32_t t;
	    t.raw = id.raw;
	    return t;
	}
    } else {
	return L4_GlobalId32(L4_ThreadNo(id), L4_Version(id));
    }
}

L4_INLINE L4_ThreadId_t L4_ThreadIdFrom32 (L4_ThreadId32_t id)
{
    if (L4_IsThreadEqual32(id, L4_anythread32)) {
	return L4_anythread;
    } else if (L4_IsLocalId32(id)) {
	if (L4_IsThreadEqual32(id, L4_anylocalthread32)) {
	    return L4_anylocalthread;
	} else {
	    L4_ThreadId_t t;
	    t.raw = id.raw;
	    return t;
	}
    } else {
	return L4_GlobalId(L4_ThreadNo32(id), L4_Version32(id));
    }
}

#if defined(__cplusplus)
L4_INLINE L4_ThreadId_t L4_ThreadId (L4_ThreadId32_t id)
{
    return L4_ThreadIdFrom32(id);
}
#endif /* __cplusplus */

#endif /* !__L4__AMD64__COMPAT_H__ */
