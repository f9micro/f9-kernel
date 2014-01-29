/* Copyright (c) 2002, 2003, 2009 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __L4_PLATFORM_KDEBUG_H__
#define __L4_PLATFORM_KDEBUG_H__

#define L4_KDB_Enter(str...)			\
do {						\
} while (0)
#endif	/* __L4_PLATFORM_KDEBUG_H__ */


#if 0	/* FIXME: IA32 specific implementation */

#ifndef __L4__X86__KDEBUG_H__
#define __L4__X86__KDEBUG_H__

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

#endif	/* IA32 specific implementation */
