/* Copyright (c) 2001, 2002, 2003 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __L4_PLATFORM_TYPES_H__
#define __L4_PLATFORM_TYPES_H__

/* FIXME: specify ARM Cortex-M in filename */
#define L4_32BIT
#define L4_LITTLE_ENDIAN

typedef unsigned int __attribute__((__mode__(__DI__))) L4_Word64_t;
typedef unsigned int		L4_Word32_t;
typedef unsigned short		L4_Word16_t;
typedef unsigned char		L4_Word8_t;

typedef unsigned long		L4_Word_t;

typedef signed int __attribute__((__mode__(__DI__))) L4_SignedWord64_t;
typedef signed int		L4_SignedWord32_t;
typedef signed short		L4_SignedWord16_t;
typedef signed char		L4_SignedWord8_t;

typedef signed long		L4_SignedWord_t;

typedef unsigned int		L4_Size_t;
typedef L4_Word64_t		L4_Paddr_t;

#endif /* !__L4_PLATFORM_TYPES_H__ */
