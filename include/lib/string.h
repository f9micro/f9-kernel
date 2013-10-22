/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef	LIB_STRING_H_
#define	LIB_STRING_H_

#include <types.h>

void *memcpy (void *__restrict, const void *__restrict, size_t);
void *memset (void *, int, size_t);
int strcmp(const char *l, const char *r);

#endif /* LIB_STRING_H_ */
