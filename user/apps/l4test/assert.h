/* Copyright (c) 2003 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __ASSERT_H__
#define __ASSERT_H__

extern void __assert(const char *, const char *, int);
#define assert(EX)  ((EX)?((void)0):__assert(#EX , __FILE__, __LINE__))

#endif /* !__ASSERT_H__ */
