/* Copyright (c) 2001-2003, 2005 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* ARM virtual registers - all located in UTCB */

#ifndef __L4_PLATFORM_VREGS_H__
#define __L4_PLATFORM_VREGS_H__

#include <l4/utcb.h>

#define __L4_NUM_MRS 16
#define __L4_NUM_BRS 8

L4_INLINE utcb_t *__L4_Utcb(void)
{
    extern void *current_utcb;
    return current_utcb;
}

/* Thread Control Registers */

L4_INLINE L4_Word_t __L4_TCR_MyGlobalId(void)
{
    return __L4_Utcb()->t_globalid;
}

L4_INLINE L4_Word_t __L4_TCR_MyLocalId(void)
{
    return (L4_Word_t) __L4_Utcb();
}

L4_INLINE L4_Word_t __L4_TCR_ProcessorNo(void)
{
    return (__L4_Utcb())->processor_no;
}

L4_INLINE L4_Word_t __L4_TCR_UserDefinedHandle(void)
{
    return __L4_Utcb()->user_defined_handle;
}

L4_INLINE void __L4_TCR_Set_UserDefinedHandle(L4_Word_t w)
{
    __L4_Utcb()->user_defined_handle = w;
}

L4_INLINE L4_Word_t __L4_TCR_Pager(void)
{
    return __L4_Utcb()->t_pager;
}

L4_INLINE void __L4_TCR_Set_Pager(L4_Word_t w)
{
    __L4_Utcb()->t_pager = w;
}

L4_INLINE L4_Word_t __L4_TCR_ExceptionHandler(void)
{
    return __L4_Utcb()->exception_handler;
}

L4_INLINE void __L4_TCR_Set_ExceptionHandler(L4_Word_t w)
{
    __L4_Utcb()->exception_handler = w;
}

L4_INLINE L4_Word_t __L4_TCR_ErrorCode(void)
{
    return __L4_Utcb()->error_code;
}

L4_INLINE L4_Word_t __L4_TCR_XferTimeout(void)
{
    return __L4_Utcb()->xfer_timeouts;
}

L4_INLINE void __L4_TCR_Set_XferTimeout(L4_Word_t w)
{
    __L4_Utcb()->xfer_timeouts = w;
}

L4_INLINE L4_Word_t __L4_TCR_IntendedReceiver(void)
{
    return __L4_Utcb()->intended_receiver;
}

L4_INLINE L4_Word_t __L4_TCR_ActualSender(void)
{
    return __L4_Utcb()->sender;
}

L4_INLINE void __L4_TCR_Set_VirtualSender(L4_Word_t w)
{
    __L4_Utcb()->sender = w;
}

/* Message Registers: MR0-MR7 in UTCB->mr_low[], MR8-MR15 in UTCB->mr[] */

L4_INLINE L4_Word32_t L4_NumMRs(void)
{
    return __L4_NUM_MRS;
}

L4_INLINE void L4_StoreMR(int i, L4_Word_t *w)
{
    if (i >= 0 && i < 8)
        *w = __L4_Utcb()->mr_low[i];
    else if (i >= 8 && i < __L4_NUM_MRS)
        *w = __L4_Utcb()->mr[i - 8];
    else
        *w = 0;
}

L4_INLINE void L4_LoadMR(int i, L4_Word_t w)
{
    if (i >= 0 && i < 8)
        __L4_Utcb()->mr_low[i] = w;
    else if (i >= 8 && i < __L4_NUM_MRS)
        __L4_Utcb()->mr[i - 8] = w;
}

L4_INLINE void L4_StoreMRs(int i, int k, L4_Word_t *w)
{
    utcb_t *utcb;

    if (i < 0 || k <= 0 || i + k > __L4_NUM_MRS)
        return;

    utcb = __L4_Utcb();

    while (k > 0 && i < 8) {
        *w++ = utcb->mr_low[i++];
        k--;
    }
    while (k > 0 && i < __L4_NUM_MRS) {
        *w++ = utcb->mr[i - 8];
        i++;
        k--;
    }
}

L4_INLINE void L4_LoadMRs(int i, int k, L4_Word_t *w)
{
    utcb_t *utcb;

    if (i < 0 || k <= 0 || i + k > __L4_NUM_MRS)
        return;

    utcb = __L4_Utcb();

    while (k > 0 && i < 8) {
        utcb->mr_low[i++] = *w++;
        k--;
    }
    while (k > 0 && i < __L4_NUM_MRS) {
        utcb->mr[i - 8] = *w++;
        i++;
        k--;
    }
}

/* Buffer Registers */

L4_INLINE L4_Word32_t L4_NumBRs(void)
{
    return __L4_NUM_BRS;
}

L4_INLINE void L4_StoreBR(int i, L4_Word_t *w)
{
    if (i >= 0 && i < __L4_NUM_BRS)
        *w = __L4_Utcb()->br[i];
}

L4_INLINE void L4_LoadBR(int i, L4_Word_t w)
{
    if (i >= 0 && i < __L4_NUM_BRS)
        __L4_Utcb()->br[i] = w;
}

L4_INLINE void L4_StoreBRs(int i, int k, L4_Word_t *w)
{
    L4_Word_t *br;

    if (i < 0 || k <= 0 || i + k > __L4_NUM_BRS)
        return;

    br = __L4_Utcb()->br + i;
    while (k-- > 0)
        *w++ = *br++;
}

L4_INLINE void L4_LoadBRs(int i, int k, const L4_Word_t *w)
{
    L4_Word_t *br;

    if (i < 0 || k <= 0 || i + k > __L4_NUM_BRS)
        return;

    br = __L4_Utcb()->br + i;
    while (k-- > 0)
        *br++ = *w++;
}

#endif /* !__L4_PLATFORM_VREGS_H__ */
