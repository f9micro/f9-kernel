/* Copyright (c) 2002-2003 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __PIGGYBACKER__INCLUDE__STRING_H__
#define __PIGGYBACKER__INCLUDE__STRING_H__

#include <l4/types.h>

extern void hex( L4_Word_t num, char str[] );
extern int strlen( const char *src );
extern void strcpy( char *dst, const char *src );
extern int strcmp( const char *str1, const char *str2 );
extern int strcmp_of( const char *str_of, const char *search );

#endif	/* __PIGGYBACKER__INCLUDE__STRING_H__ */
