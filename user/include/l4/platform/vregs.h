/*********************************************************************
 *                
 * Copyright (C) 2001-2003, 2005,  Karlsruhe University
 *                
 * File path:     l4/ia32/vregs.h
 * Description:   x86 virtual registers
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: vregs.h,v 1.13 2005/12/21 17:28:08 stoess Exp $
 *                
 ********************************************************************/
#ifndef __L4__X86__VREGS_H__
#define __L4__X86__VREGS_H__

#include <l4/utcb.h>

register L4_Word32_t __L4_MR0 asm ("r4");
register L4_Word32_t __L4_MR1 asm ("r5");
register L4_Word32_t __L4_MR2 asm ("r6");
register L4_Word32_t __L4_MR3 asm ("r7");
register L4_Word32_t __L4_MR4 asm ("r8");
register L4_Word32_t __L4_MR5 asm ("r9");
register L4_Word32_t __L4_MR6 asm ("r10");
register L4_Word32_t __L4_MR7 asm ("r11");

/*
 * All virtual registers on x86 is located in a user Level Thread
 * Control Block (UTCB).
 */

L4_INLINE utcb_t * __L4_Utcb (void)
{
    extern void *current_utcb;
    return current_utcb;
}



/*
 * Location of TCRs within UTCB.
 */

#define __L4_TCR_THREAD_WORD_0			(-4)
#define __L4_TCR_THREAD_WORD_1			(-5)
#define __L4_TCR_VIRTUAL_ACTUAL_SENDER		(-6)
#define __L4_TCR_INTENDED_RECEIVER		(-7)
#define __L4_TCR_XFER_TIMEOUT			(-8)
#define __L4_TCR_ERROR_CODE			(-9)
#define __L4_TCR_PREEMPT_FLAGS			(-10)
#define __L4_TCR_COP_FLAGS			(-10)
#define __L4_TCR_EXCEPTION_HANDLER		(-11)
#define __L4_TCR_PAGER				(-12)
#define __L4_TCR_USER_DEFINED_HANDLE		(-13)
#define __L4_TCR_PROCESSOR_NO			(-14)
#define __L4_TCR_MY_GLOBAL_ID			(-15)



/*
 * Thread Control Registers.
 */

L4_INLINE L4_Word_t __L4_TCR_MyGlobalId (void)
{
    return __L4_Utcb ()->t_globalid;
}

L4_INLINE L4_Word_t __L4_TCR_MyLocalId (void)
{
    return (L4_Word_t) __L4_Utcb ();
}

L4_INLINE L4_Word_t __L4_TCR_ProcessorNo (void)
{
    return (__L4_Utcb ())->processor_no;
}

L4_INLINE L4_Word_t __L4_TCR_UserDefinedHandle (void)
{
    return __L4_Utcb()->user_defined_handle;
}

L4_INLINE void __L4_TCR_Set_UserDefinedHandle (L4_Word_t w)
{
    __L4_Utcb()->user_defined_handle = w;
}

L4_INLINE L4_Word_t __L4_TCR_Pager (void)
{
    return __L4_Utcb()->t_pager;
}

L4_INLINE void __L4_TCR_Set_Pager (L4_Word_t w)
{
    __L4_Utcb()->t_pager = w;
}

L4_INLINE L4_Word_t __L4_TCR_ExceptionHandler (void)
{
    return __L4_Utcb()->exception_handler;
}

L4_INLINE void __L4_TCR_Set_ExceptionHandler (L4_Word_t w)
{
    __L4_Utcb()->exception_handler = w;
}

L4_INLINE L4_Word_t __L4_TCR_ErrorCode (void)
{
    return __L4_Utcb()->error_code;
}

L4_INLINE L4_Word_t __L4_TCR_XferTimeout (void)
{
    return __L4_Utcb()->xfer_timeouts;
}

L4_INLINE void __L4_TCR_Set_XferTimeout (L4_Word_t w)
{
    __L4_Utcb()->xfer_timeouts = w;
}

L4_INLINE L4_Word_t __L4_TCR_IntendedReceiver(void)
{
    return __L4_Utcb()->intended_receiver;
}

L4_INLINE L4_Word_t __L4_TCR_ActualSender (void)
{
    return __L4_Utcb()->sender;
}

L4_INLINE void __L4_TCR_Set_VirtualSender (L4_Word_t w)
{
    __L4_Utcb()->sender = w;
}



/*
 * Message Registers.
 */

L4_INLINE void L4_StoreMR (int i, L4_Word_t * w)
{
	switch (i) {
		case 0: *w = __L4_MR0; break;
		case 1: *w = __L4_MR1; break;
		case 2: *w = __L4_MR2; break;
		case 3: *w = __L4_MR3; break;
		case 4: *w = __L4_MR4; break;
		case 5: *w = __L4_MR5; break;
		case 6: *w = __L4_MR6; break;
		case 7: *w = __L4_MR7; break;
		default:
			*w = __L4_Utcb()->mr[i - 8];
	}
}

L4_INLINE void L4_LoadMR (int i, L4_Word_t w)
{
	switch (i) {
		case 0: __L4_MR0 = w; break;
		case 1: __L4_MR1 = w; break;
		case 2: __L4_MR2 = w; break;
		case 3: __L4_MR3 = w; break;
		case 4: __L4_MR4 = w; break;
		case 5: __L4_MR5 = w; break;
		case 6: __L4_MR6 = w; break;
		case 7: __L4_MR7 = w; break;
		default:
			__L4_Utcb()->mr[i - 8] = w;
	}
}

L4_INLINE void L4_StoreMRs (int i, int k, L4_Word_t * w)
{
	switch (i) {
		case 0: *w++ = __L4_MR0; if (--k <= 0) break;
		case 1: *w++ = __L4_MR1; if (--k <= 0) break;
		case 2: *w++ = __L4_MR2; if (--k <= 0) break;
		case 3: *w++ = __L4_MR3; if (--k <= 0) break;
		case 4: *w++ = __L4_MR4; if (--k <= 0) break;
		case 5: *w++ = __L4_MR5; if (--k <= 0) break;
		case 6: *w++ = __L4_MR6; if (--k <= 0) break;
		case 7: *w++ = __L4_MR7; if (--k <= 0) break;
		default:
		{
			uint32_t *mr = __L4_Utcb()->mr + i - 8;
			while (k-- > 0)
				*w++ = *mr++;
		}
	}
}

L4_INLINE void L4_LoadMRs (int i, int k, L4_Word_t * w)
{
	switch (i) {
		case 0: __L4_MR0 = *w++; if (--k <= 0) break;
		case 1: __L4_MR1 = *w++; if (--k <= 0) break;
		case 2: __L4_MR2 = *w++; if (--k <= 0) break;
		case 3: __L4_MR3 = *w++; if (--k <= 0) break;
		case 4: __L4_MR4 = *w++; if (--k <= 0) break;
		case 5: __L4_MR5 = *w++; if (--k <= 0) break;
		case 6: __L4_MR6 = *w++; if (--k <= 0) break;
		case 7: __L4_MR7 = *w++; if (--k <= 0) break;
		default:
		{
			uint32_t *mr = __L4_Utcb()->mr + i - 8;
			while (k-- > 0)
				*mr++ = *w++;
		}
	}
}



/*
 * Buffer Registers.
 */

L4_INLINE void L4_StoreBR (int i, L4_Word_t * w)
{
    *w = __L4_Utcb()->br[i];
}

L4_INLINE void L4_LoadBR (int i, L4_Word_t w)
{
    __L4_Utcb()->br[i] = w;
}

L4_INLINE void L4_StoreBRs (int i, int k, L4_Word_t * w)
{
    L4_Word_t * br = __L4_Utcb()->br + i;

    while (k-- > 0)
	*w++ = *br++;
}

L4_INLINE void L4_LoadBRs (int i, int k, const L4_Word_t * w)
{
    L4_Word_t * br = __L4_Utcb()->br + i;

    while (k-- > 0)
	*br++ = *w++;
}


#endif /* !__L4__X86__VREGS_H__ */
