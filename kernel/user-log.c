/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <debug.h>
#include <thread.h>
#include <user-log.h>

void user_log(tcb_t *from)
{
    char *format = (char *) from->ctx.regs[1];
    va_list *va = (va_list *) from->ctx.regs[2];

    /* Debug: validate pointers before use */
    if (!format || (uint32_t) format < 0x08000000 ||
        (uint32_t) format > 0x20020000) {
        dbg_printf(DL_KDB, "[ULOG: bad fmt %p]\n", format);
        return;
    }

    dbg_vprintf(DL_KDB, format, *va);
}
