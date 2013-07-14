/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef LIB_STDIO_H_
#define LIB_SDTIO_H_

#include <types.h>
#include <lib/stdarg.h>

void __l4_putchar(uint8_t chr);
uint8_t __l4_getchar();
void __l4_puts(char *str);
void __l4_printf(char *fmt, ...);
void __l4_vprintf(char *fmt, va_list va);

#endif /* LIB_SDTIO_H_ */
