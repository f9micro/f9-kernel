/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	piggybacker/common/1275tree.cc
 * Description:	Builds a position independent copy of the Open Firmware
 * 		device tree.
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
 * $Id: 1275tree.cc,v 1.1 2003/10/29 23:11:17 cvansch Exp $
 *
 ***************************************************************************/

#include <l4/types.h>
//#include <piggybacker/string.h>
//#include <piggybacker/ieee1275.h>
//#include <piggybacker/1275tree.h>
//#include <piggybacker/io.h>
#include "1275tree.h"
#include "string.h"

#define NAME_BUFSIZ	256

/****************************************************************************/

device_t *device_find( device_t *list, const char *name )
{
    while( list->handle )
    {
	if( !strcmp_of(list->name, name) )
	    return list;
	list = device_next( list );
    }
    return (device_t *)0;
}

device_t *device_find_handle( device_t *list, L4_Word32_t handle )
{
    while( list->handle )
    {
	if( list->handle == handle )
	    return list;
	list = device_next( list );
    }
    return (device_t *)0;
}

item_t *item_find( device_t *dev, const char *name )
{
    L4_Word_t i;
    item_t *item_name, *item_data;

    i = 0;
    item_name = item_first( dev );
    item_data = item_next( item_name );

    while( i < dev->prop_count )
    {
	if( !strcmp(item_name->data, name) )
	    return item_data;
	item_name = item_next( item_data );
	item_data = item_next( item_name );
	i++;
    }

    return (item_t *)0;
}

/****************************************************************************/

