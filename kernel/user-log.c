/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <user-log.h>
#include <debug.h>
#include <thread.h>

void user_log(tcb_t *from)
{
	char *format = (char *)from->ctx.regs[1];
	va_list *va = (va_list*)from->ctx.regs[2];
	dbg_vprintf(DL_KDB, format, *va);
}
