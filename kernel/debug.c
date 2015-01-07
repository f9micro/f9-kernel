/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <lib/stdarg.h>
#include <lib/stdio.h>
#include <debug.h>

#ifdef CONFIG_DEBUG

dbg_layer_t dbg_layer;

void dbg_printf(dbg_layer_t layer, char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	dbg_vprintf(layer, fmt, va);
	va_end(va);
}

void dbg_vprintf(dbg_layer_t layer, char *fmt, va_list va)
{
	if (layer != DL_EMERG && !(dbg_layer & layer))
		return;

	__l4_vprintf(fmt, va);
}

#endif	/* CONFIG_DEBUG */
