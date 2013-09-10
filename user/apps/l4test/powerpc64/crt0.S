/****************************************************************************
 *
 * Copyright (C) 2002-2003, University of New South Wales
 *
 * File path:	user/serv/sigma0/crt0-powerpc64.S
 * Description:	PowerPC64 startup code for L4 Test
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
 * $Id: crt0.S,v 1.5 2005/01/21 06:02:22 cvansch Exp $
 *
 ***************************************************************************/

#include <l4/powerpc64/asm.h>


	.section ".text"
	.align	2
	.globl	_start
_start:
	/* Install our stack. */
	LD_ADDR(%r1, _sigma0_stack_top)
	addi	%r1, %r1, -64

	/* Load the TOC. */
	LD_ADDR(%r10, __L4_Init)
	ld	%r2, 8(%r10)

	/* Zero bss */
	LD_ADDR(%r10, __bss_start)
	LD_ADDR(%r11, _end)
	subi	%r10, %r10, 4
	subi	%r11, %r11, 4

	li	%r12, 0
1:	cmp	0, %r10, %r11
	beq	2f
	stwu	%r12, 4(%r10)
	b	1b
2:

	bl	.__L4_Init

	/* Jump into C code. */
	bl	.main

	.long	0x00000000
1:	b	1b	/* Spin. */

/****************************************************************************/

	.section ".bss"
	.globl _sigma0_stack_bottom
	.globl _sigma0_stack_top

#define SIGMA0_STACK_SIZE	(4096*4)
_sigma0_stack_bottom:
.lcomm	_sigma0_stack, SIGMA0_STACK_SIZE, 16
_sigma0_stack_top:

/****************************************************************************/

	.section ".text"
	.align	2
	.globl	_exreg_target
_exreg_target:
	/* stack[0] = (L4_Word_t) fn_ptr; */

	ld	%r3, 0(%r1)	/* Load the function address */

	ld	%r0, 0(%r3)
	ld	%r2, 8(%r3)	/* Load the TOC */
	mtlr	%r0
	blrl
	nop

	.long	0x00000000
1:	b	1b	/* Spin. */
