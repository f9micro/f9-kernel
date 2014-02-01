/* Copyright (c) 2003-2004 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __L4IO_H__
#define __L4IO_H__

#include <l4/types.h>

#include "stdarg.h"

#ifdef __cplusplus
extern "C" {
#endif

int vsnprintf(char *str, L4_Size_t size, const char *fmt, va_list ap);
int snprintf(char *str, L4_Size_t size, const char *fmt, ...);
int printf(const char *fmt, ...);
void putc(int c);
int getc(void);


#ifdef __cplusplus
}
#endif


#endif /* !__L4IO_H__ */
