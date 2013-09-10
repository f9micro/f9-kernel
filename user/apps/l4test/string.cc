/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	piggybacker/common/string.cc
 * Description:	
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
 * $Id: string.cc,v 1.1 2003/10/29 23:11:58 cvansch Exp $
 *
 ***************************************************************************/

#include "string.h"

int
strlen( const char *str )
{
	int len = 0;
	while(*str != '\0')
		str++, len++;

	return len;
}

int strcmp( const char *str1, const char *str2 )
{
    while( *str1 && *str2 ) {
	if( *str1 < *str2 )
	    return -1;
	if( *str1 > *str2 )
	    return 1;
	str1++;
	str2++;
    }
    if( *str2 )
	return -1;
    if( *str1 )
	return 1;
    return 0;
}

int strcmp_of( const char *str_of, const char *search )
{
    while( *str_of && *search )
    {
	if ((*str_of == '@') && (*search == '/'))
	{
	    while ( *str_of && (*str_of != '/'))
		str_of++;

	    if ( !*str_of )
		return -1;
	}
	if( *str_of < *search )
	    return -1;
	if( *str_of > *search )
	    return 1;
	str_of++;
	search++;
    }

    if( *search )
	return -1;

    if( *str_of == '@' )
	while ( *str_of && (*str_of != '/') )
	    str_of++;

    if( *str_of )
	return 1;

    return 0;
}

