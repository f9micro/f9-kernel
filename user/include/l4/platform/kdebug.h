/*********************************************************************
 *                
 * Copyright (C) 2002, 2003, 2009,  Karlsruhe University
 *                
 * File path:     l4/ia32/kdebug.h
 * Description:   L4 Kdebug interface for ia32
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
 * $Id: kdebug.h,v 1.10 2006/12/05 17:08:39 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__X86__KDEBUG_H__
#define __L4__X86__KDEBUG_H__


#define L4_KDB_Enter(str...)			\
do {						\
    __asm__ __volatile__ (			\
	"/* L4_KDB_Enter() */		\n"	\
	"	int	$3		\n"	\
	"	jmp	2f		\n"	\
	"	mov	$1f, %eax	\n"	\
	".section .rodata		\n"	\
	"1:	.ascii \"" str "\"	\n"	\
	"	.byte 0			\n"	\
	".previous			\n"	\
	"2:				\n");	\
} while (0)

#define __L4_KDB_Op(op, name)			\
L4_INLINE void L4_KDB_##name (void)		\
{						\
    __asm__ __volatile__ (			\
	"/* L4_KDB_"#name"() */		\n"	\
	"	int	$3		\n"	\
	"	cmpb	%0, %%al	\n"	\
	:					\
	: "i" (op));				\
}

#define __L4_KDB_Op_Arg(op, name, argtype)	\
L4_INLINE void L4_KDB_##name (argtype arg)	\
{						\
    __asm__ __volatile__ (			\
	"/* L4_KDB_"#name"() */		\n"	\
	"	int	$3		\n"	\
	"	cmpb	%1, %%al	\n"	\
	:					\
	: "a" (arg), "i" (op));			\
}

#define __L4_KDB_Op_Ret(op, name, rettype)	\
L4_INLINE rettype L4_KDB_##name (void)		\
{						\
    rettype ret;				\
    __asm__ __volatile__ (			\
	"/* L4_KDB_"#name"() */		\n"	\
	"	int	$3		\n"	\
	"	cmpb	%1, %%al	\n"	\
	: "=a" (ret)				\
	: "i" (op));				\
    return ret;					\
}


__L4_KDB_Op_Arg (0x00, PrintChar, char);
__L4_KDB_Op_Arg (0x01, PrintString, char *);
__L4_KDB_Op     (0x02, ClearPage);
__L4_KDB_Op     (0x03, ToggleBreakin);
__L4_KDB_Op_Ret (0x08, ReadChar, char);
__L4_KDB_Op_Ret (0x0d, ReadChar_Blocked, char);


#endif /* !__L4__X86__KDEBUG_H__ */
