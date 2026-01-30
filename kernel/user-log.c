/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <debug.h>
#include <memory.h>
#include <thread.h>
#include <user-log.h>

void user_log(tcb_t *from)
{
    char *format = (char *) from->ctx.regs[1];
    va_list *va = (va_list *) from->ctx.regs[2];

    /* CRITICAL: Validate user pointers before dereferencing!
     * User can pass arbitrary pointers that could cause kernel fault.
     * Check that pointers are in valid mapped memory.
     */
    memptr_t format_addr = (memptr_t) format;
    memptr_t va_addr = (memptr_t) va;

    /* Check format pointer is in valid executable memory (code section)
     * Format strings must be in UTEXT (MP_UX) or KTEXT (MP_KX), not arbitrary
     * kernel data. This prevents malicious users from using kernel data
     * structures as format strings to leak kernel information.
     */
    int format_pool = mempool_search(format_addr, 1);
    if (format_pool < 0) {
        dbg_printf(DL_KDB, "[user_log] Invalid format pointer: %p\n", format);
        return;
    }

    mempool_t *pool = mempool_getbyid(format_pool);
    if (!pool || !(pool->flags & (MP_UX | MP_KX))) {
        dbg_printf(DL_KDB,
                   "[user_log] Format pointer not in executable memory: %p\n",
                   format);
        return;
    }

    /* Check va_list pointer is in valid user-accessible memory (stack/data) */
    int va_pool = mempool_search(va_addr, sizeof(va_list));
    if (va_pool < 0) {
        dbg_printf(DL_KDB, "[user_log] Invalid va_list pointer: %p\n", va);
        return;
    }

    pool = mempool_getbyid(va_pool);
    if (!pool || !(pool->flags & (MP_UR | MP_UW))) {
        dbg_printf(DL_KDB,
                   "[user_log] va_list pointer not user-accessible: %p\n", va);
        return;
    }

    /* Pointers validated, safe to dereference */
    dbg_vprintf(DL_KDB, format, *va);
}
