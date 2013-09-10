/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	l4/powerpc/kdebug.h
 * Description:	L4 Kdebug interface for PowerPC.
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
 * $Id: kdebug.h,v 1.8 2003/11/03 14:40:33 joshua Exp $
 *
 ***************************************************************************/
#ifndef __L4__POWERPC__KDEBUG_H__
#define __L4__POWERPC__KDEBUG_H__

#define __L4_TRAP_KDEBUG	(0x5afe)
#define __L4_TRAP_KPUTC	(__L4_TRAP_KDEBUG + 1)
#define __L4_TRAP_KGETC	(__L4_TRAP_KDEBUG + 2)

#define L4_KDB_Enter(str...)					\
    asm volatile (						\
	"li %%r0, %0 ;"						\
	"trap ;"						\
	"b 1f ;"						\
	".string	\"KD# " str "\";"			\
	".align 2 ;"						\
	"1:"							\
	: /* outputs */						\
	: /* inputs */ "i" (__L4_TRAP_KDEBUG)			\
	: /* clobbers */ "r0" )


#define __L4_KDB_Op_Arg(op, name, argtype)	\
L4_INLINE void L4_KDB_##name (argtype arg)	\
{						\
    __asm__ __volatile__ (			\
	    "mr %%r3, %0 ;"			\
	    "li %%r0, %1 ;"			\
	    "trap ;"				\
	    : /* outputs */			\
	    : /* inputs */			\
	    "b" (arg), "i" (op)			\
	    : /* clobbers */			\
	    "r0", "r3"				\
	    );					\
}


#define __L4_KDB_Op_Ret(op, name, rettype)	\
L4_INLINE rettype L4_KDB_##name (void)		\
{						\
    rettype ret;				\
    __asm__ __volatile__ (			\
	    "li %%r0, %1 ;"			\
	    "trap ;"				\
	    "mr %0, %%r3 ;"			\
	    : /* outputs */			\
	    "=b" (ret)				\
	    : /* inputs */			\
	    "i" (op)				\
	    : /* clobbers */			\
	    "r0", "r3"				\
	    );					\
    return ret;					\
}

__L4_KDB_Op_Ret( __L4_TRAP_KGETC, ReadChar_Blocked, char );
__L4_KDB_Op_Arg( __L4_TRAP_KPUTC, PrintChar, char );

#endif	/* __L4__POWERPC__KDEBUG_H__ */
