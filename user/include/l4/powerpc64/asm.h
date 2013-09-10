/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     arch/powerpc64/asm.h
 * Created:       23/07/2002 17:48:20 by Simon Winwood (sjw)
 * Description:   Assembler macros etc. 
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
 * $Id: asm.h,v 1.3 2003/10/17 01:48:55 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __L4__POWERPC64__ASM_H__
#define __L4__POWERPC64__ASM_H__

#define BEGIN_PROC(name)			\
    .global name;				\
    .align 3;					\
    .func name;					\
name:

#define END_PROC(name)				\
    .endfunc

/*
 * LD_ADDR ( reg, symbol )
 *   loads the address of symbol into reg
 */
#define	LD_ADDR(reg, symbol)		\
    lis	    reg, symbol##@highest;	\
    ori	    reg, reg, symbol##@higher;	\
    rldicr  reg, reg, 32,31;		\
    oris    reg, reg, symbol##@h;	\
    ori	    reg, reg, symbol##@l

#define LD_CONST(reg, value)			\
    lis	    reg,(((value)>>48)&0xFFFF);		\
    ori     reg,reg,(((value)>>32)&0xFFFF);	\
    rldicr  reg,reg,32,31;			\
    oris    reg,reg,(((value)>>16)&0xFFFF);	\
    ori     reg,reg,((value)&0xFFFF);

#define LD_LABEL(reg, label)		\
    lis     reg,(label)@highest;        \
    ori     reg,reg,(label)@higher;     \
    rldicr  reg,reg,32,31;              \
    oris    reg,reg,(label)@h;          \
    ori     reg,reg,(label)@l;
/*
 * Register names
 */
#define	sp	%r1
#define	toc	%r2
#define	utcb	%r13

#endif /* __L4__POWERPC64__ASM_H__ */
