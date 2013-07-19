/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_IRQ_H_
#define PLATFORM_IRQ_H_

#include <softirq.h>
#include <thread.h>
#include <sched.h>
#include <platform/link.h>
/*
 * TODO: current implementation is highly ineffective because:
 * 1. It is does not support Nested Interrupts. At present, interrupt nesting
 *    will rewrite irq_stack_pointer which is global)
 * 2. We always save all registers even if we do not perform context switching.
 */

static inline void irq_disable()
{
	__asm__ __volatile__ ("cpsid i");
}

static inline void irq_enable()
{
	__asm__ __volatile__ ("cpsie i");
}

static inline void irq_svc()
{
	__asm__ __volatile__ ("svc #0");
}

static inline int irq_number()
{
	int irqno;

	__asm__ __volatile__ (
		"mrs r0, ipsr\n"
		"mov r0, %0"
		: "=r" (irqno) : : "r0");

	return irqno;
}

/*
 * irq_save()
 *
 * Saves {r4-r11}, msp, psp, EXC_RETURN
 */
#define irq_save(ctx) \
	__asm__ __volatile__ ("cpsid i");				\
	__asm__ __volatile__ ("mov r0, %0" : : "r" ((ctx)->regs));	\
	__asm__ __volatile__ ("stm r0, {r4-r11}");			\
	__asm__ __volatile__ ("cmp lr, #0xFFFFFFF9");			\
	__asm__ __volatile__ ("ite eq");				\
	__asm__ __volatile__ ("mrseq r0, msp");				\
	__asm__ __volatile__ ("mrsne r0, psp");				\
	__asm__ __volatile__ ("mov %0, r0" : "=r" ((ctx)->sp) : );

#define irq_return(ctx) \
	__asm__ __volatile__ ("mov lr, %0" : : "r" ((ctx)->ret));	\
	__asm__ __volatile__ ("mov r0, %0" : : "r" ((ctx)->sp));	\
	__asm__ __volatile__ ("mov r2, %0" : : "r" ((ctx)->ctl));	\
	__asm__ __volatile__ ("cmp lr, #0xFFFFFFF9");			\
	__asm__ __volatile__ ("ite eq");				\
	__asm__ __volatile__ ("msreq msp, r0");				\
	__asm__ __volatile__ ("msrne psp, r0");				\
	__asm__ __volatile__ ("mov r0, %0" : : "r" ((ctx)->regs));	\
	__asm__ __volatile__ ("ldm r0, {r4-r11}");			\
	__asm__ __volatile__ ("msr control, r2");			\
	__asm__ __volatile__ ("cpsie i");				\
	__asm__ __volatile__ ("bx lr");

/* Initial context switches to kernel.
 * It simulates interrupt to save corect context on stack.
 * When interrupts are enabled, it will jump to interrupt handler
 * and then return to normal execution of kernel code.
 */
#define init_ctx_switch(ctx, pc) \
	__asm__ __volatile__ ("mov r0, %0" : : "r" ((ctx)->sp));	\
	__asm__ __volatile__ ("msr msp, r0");				\
	__asm__ __volatile__ ("mov r1, %0" : : "r" (pc));		\
	__asm__ __volatile__ ("cpsie i");				\
	__asm__ __volatile__ ("bx r1");

/*
 * Context is switched on interrupt return
 * We check if nobody schedules actions in kernel (SOFTIRQs)
 * Then do context switch
 *
 * Idea is that on interrupt we'll save all registers under
 * irq_stack_pointer than on return we copy registers to
 * thread's structure or to kernel_ctx
 * */
#define IRQ_HANDLER(name, sub) \
	void name() __NAKED;			\
	void name()				\
	{					\
		irq_save(&current->ctx);	\
		sub();				\
		schedule();			\
		irq_return(&current->ctx);	\
	}

extern volatile tcb_t *current;

#endif	/* PLATFORM_IRQ_H_ */
