/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	include/piggybacker/1275tree.h
 * Description:	Macros and data types for enabling easy access to the 
 *		position independent Open Firmware device tree.
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
 * $Id: 1275tree.h,v 1.1 2003/10/29 23:11:17 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __L4TEST__POWERPC64__1275TREE_H__
#define __L4TEST__POWERPC64__1275TREE_H__

#include <l4/types.h>

typedef struct {
    L4_Word32_t handle;
    L4_Word32_t prop_count;
    L4_Word32_t prop_size;
    L4_Word32_t len;
    char name[];
} device_t;

typedef struct {
    L4_Word32_t len;
    char data[];
} item_t;


L4_INLINE L4_Word_t wrap_up( L4_Word_t val, L4_Word_t size )
{
    if( val % size )
	val = (val + size) & ~(size-1);
    return val;
}

L4_INLINE device_t *device_first( char *spill )
{
    return (device_t *)( wrap_up( (L4_Word_t)spill, sizeof(L4_Word_t)) );
}

L4_INLINE device_t *device_next( device_t *dev )
{
    return (device_t *)wrap_up( (L4_Word_t)dev->name + dev->len + dev->prop_size, sizeof(L4_Word_t) );
}

L4_INLINE item_t *item_first( device_t *dev )
{
    return (item_t *)wrap_up( (L4_Word_t)dev->name + dev->len, sizeof(L4_Word_t) );
}

L4_INLINE item_t *item_next( item_t *item )
{
    return (item_t *)wrap_up( (L4_Word_t)item->data + item->len, sizeof(L4_Word_t) );
}

extern device_t *device_find( device_t *list, const char *name );
extern device_t *device_find_handle( device_t *list, L4_Word32_t handle );
extern item_t *item_find( device_t *dev, const char *name );

#endif	/* __L4TEST__POWERPC64__1275TREE_H__ */
