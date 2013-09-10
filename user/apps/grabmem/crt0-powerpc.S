/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	apps/l4test/powerpc/crt0.S
 * Description:	PowerPC startup code for l4test.
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
 * $Id: crt0-powerpc.S,v 1.1 2003/10/21 04:54:08 benno Exp $
 *
 ***************************************************************************/

	.section ".data"
	.align	2
	.globl	_l4_utcb
_l4_utcb:
	.long	0

	.section ".text"
	.align	2
	.globl	_start
_start:
	/* Install our stack. */
	lis	%r1, _sigma0_stack_top@ha
	la	%r1, _sigma0_stack_top@l(%r1)
	addi	%r1, %r1, -16

	/* Point to 0 for the small data area. */
	li	%r13, 0

	/* Save the utcb pointer handed us by the kernel in r2. */
	lis	%r10, _l4_utcb@ha
	la	%r10, _l4_utcb@l(%r10)
	stw	%r2, 0(%r10)

	bl	__L4_Init

	/* Jump into C code. */
	bl	main

1:	b	1b	/* Spin. */

/****************************************************************************/

	.section ".bss"
	.globl _sigma0_stack_bottom
	.globl _sigma0_stack_top

#define SIGMA0_STACK_SIZE	(4096*2)
_sigma0_stack_bottom:
.lcomm	_sigma0_stack, SIGMA0_STACK_SIZE, 16
_sigma0_stack_top:

