/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <types.h>
#include <lib/stdio.h>
#ifdef CONFIG_STDIO_USE_DBGPORT
#include <platform/debug_device.h>
#endif

/* FIXME: we can move it to be inline function in header file. */
#ifdef CONFIG_STDIO_USE_DBGPORT
uint8_t __l4_getchar(void)
{
	return (dbg_getchar());
}

void __l4_putchar(uint8_t chr)
{
	dbg_putchar(chr);
}
#else
/* CONFIG_STDIO_NODEV */
uint8_t __l4_getchar(void)
{
	return (0);
}

void __l4_putchar(uint8_t chr)
{
	(void) chr;
}
#endif
