/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	l4/powerpc/vregs.h
 * Description:	PowerPC virtual registers.
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
 * $Id: vregs.h,v 1.8 2003/10/27 16:21:33 joshua Exp $
 *
 ***************************************************************************/
#ifndef __L4__POWERPC__VREGS_H__
#define __L4__POWERPC__VREGS_H__

L4_INLINE L4_Word_t *__L4_PPC_Utcb( void ) __attribute__ ((const));

L4_INLINE L4_Word_t *__L4_PPC_Utcb( void )
{
#if 0
    extern L4_Word_t *_l4_utcb;
    return _l4_utcb;
#else
    L4_Word_t *utcb;

    __asm__ __volatile__ ( "mr %0, %%r2" : "=r" (utcb) );

    return utcb;
#endif
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

L4_INLINE L4_Word_t __L4_TCR_MyGlobalId( void )
{
    return (__L4_PPC_Utcb())[ __L4_TCR_MY_GLOBAL_ID ];
}

L4_INLINE L4_Word_t __L4_TCR_MyLocalId( void )
{
    return (L4_Word_t) __L4_PPC_Utcb();
}

L4_INLINE L4_Word_t __L4_TCR_ProcessorNo( void )
{
    return (__L4_PPC_Utcb())[ __L4_TCR_PROCESSOR_NO ];
}

L4_INLINE L4_Word_t __L4_TCR_UserDefinedHandle( void )
{
    return (__L4_PPC_Utcb())[ __L4_TCR_USER_DEFINED_HANDLE ];
}

L4_INLINE void __L4_TCR_Set_UserDefinedHandle( L4_Word_t w )
{
    (__L4_PPC_Utcb())[ __L4_TCR_USER_DEFINED_HANDLE ] = w;
}

L4_INLINE L4_Word_t __L4_TCR_Pager( void )
{
    return (__L4_PPC_Utcb())[ __L4_TCR_PAGER ];
}

L4_INLINE void __L4_TCR_Set_Pager( L4_Word_t w )
{
    (__L4_PPC_Utcb())[ __L4_TCR_PAGER ] = w;
}

L4_INLINE L4_Word_t __L4_TCR_ExceptionHandler( void )
{
    return (__L4_PPC_Utcb())[ __L4_TCR_EXCEPTION_HANDLER ];
}

L4_INLINE void __L4_TCR_Set_ExceptionHandler( L4_Word_t w )
{
    (__L4_PPC_Utcb())[ __L4_TCR_EXCEPTION_HANDLER ] = w;
}

L4_INLINE L4_Word_t __L4_TCR_ErrorCode( void )
{
    return (__L4_PPC_Utcb())[ __L4_TCR_ERROR_CODE ];
}

L4_INLINE L4_Word_t __L4_TCR_XferTimeout( void )
{
    return (__L4_PPC_Utcb())[ __L4_TCR_XFER_TIMEOUT ];
}

L4_INLINE void __L4_TCR_Set_XferTimeout( L4_Word_t w )
{
    (__L4_PPC_Utcb())[ __L4_TCR_XFER_TIMEOUT ] = w;
}

L4_INLINE L4_Word_t __L4_TCR_IntendedReceiver( void )
{
    return (__L4_PPC_Utcb())[ __L4_TCR_INTENDED_RECEIVER ];
}

L4_INLINE L4_Word_t __L4_TCR_ActualSender( void )
{
    return (__L4_PPC_Utcb())[ __L4_TCR_VIRTUAL_ACTUAL_SENDER ];
}

L4_INLINE void __L4_TCR_Set_VirtualSender( L4_Word_t w )
{
    (__L4_PPC_Utcb())[ __L4_TCR_VIRTUAL_ACTUAL_SENDER ] = w;
}

L4_INLINE L4_Word_t __L4_TCR_ThreadWord (L4_Word_t n)
{
    return (__L4_PPC_Utcb ())[__L4_TCR_THREAD_WORD_0 - (int) n];
}

L4_INLINE void __L4_TCR_Set_ThreadWord (L4_Word_t n, L4_Word_t w)
{
    (__L4_PPC_Utcb ())[__L4_TCR_THREAD_WORD_0 - (int) n] = w;
}

L4_INLINE void L4_Set_CopFlag( L4_Word_t n )
{
    (__L4_PPC_Utcb())[ __L4_TCR_COP_FLAGS ] |= (1 << (8 + n));
}

L4_INLINE void L4_Clr_CopFlag( L4_Word_t n )
{
    (__L4_PPC_Utcb())[ __L4_TCR_COP_FLAGS ] &= ~(1 << (8 + n));
}

L4_INLINE L4_Bool_t L4_EnablePreemptionFaultException( void )
{
    L4_Bool_t old = ((__L4_PPC_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] >> 5) & 1;
    (__L4_PPC_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] |= (1 << 5);
    return old;
}

L4_INLINE L4_Bool_t L4_DisablePreemptionFaultException( void )
{
    L4_Bool_t old = ((__L4_PPC_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] >> 5) & 1;
    (__L4_PPC_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] &= ~(1 << 5);
    return old;
}

L4_INLINE L4_Bool_t L4_EnablePreemption( void )
{
    L4_Bool_t old = ((__L4_PPC_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] >> 6) & 1;
    (__L4_PPC_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] |= (1 << 6);
    return old;
}

L4_INLINE L4_Bool_t L4_DisablePreemption( void )
{
    L4_Bool_t old = ((__L4_PPC_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] >> 6) & 1;
    (__L4_PPC_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] &= ~(1 << 6);
    return old;
}

L4_INLINE L4_Bool_t L4_PreemptionPending( void )
{
    L4_Bool_t retval = (((__L4_PPC_Utcb())[ __L4_TCR_PREEMPT_FLAGS ]) >> 7) & 1;
    return retval;
}



/*
 * Message Registers.
 */

L4_INLINE void L4_StoreMR( int i, L4_Word_t * w )
{
    *w = (__L4_PPC_Utcb())[ i ];
}

L4_INLINE void L4_LoadMR( int i, L4_Word_t w )
{
    (__L4_PPC_Utcb())[ i ] = w;
}

L4_INLINE void L4_StoreMRs( int i, int k, L4_Word_t * w )
{
    L4_Word_t * mr = __L4_PPC_Utcb() + i;

    while( k-- > 0 )
	*w++ = *mr++;
}

L4_INLINE void L4_LoadMRs( int i, int k, L4_Word_t * w )
{
    L4_Word_t * mr = __L4_PPC_Utcb() + i;

    while( k-- > 0 )
	*mr++ = *w++;
}



/*
 * Buffer Registers.
 */

L4_INLINE void L4_StoreBR( int i, L4_Word_t * w )
{
    *w = (__L4_PPC_Utcb())[ -16 - i ];
}

L4_INLINE void L4_LoadBR( int i, L4_Word_t w )
{
    (__L4_PPC_Utcb())[ -16 - i ] = w;
}

L4_INLINE void L4_StoreBRs( int i, int k, L4_Word_t * w )
{
    L4_Word_t * br = (__L4_PPC_Utcb()) - 16 - i;

    while( k-- > 0 )
	*w++ = *br--;
}

L4_INLINE void L4_LoadBRs( int i, int k, const L4_Word_t * w )
{
    L4_Word_t * br = (__L4_PPC_Utcb()) - 16 - i;

    while( k-- > 0 )
	*br-- = *w++;
}


#endif	/* __L4__POWERPC__VREGS_H__ */

