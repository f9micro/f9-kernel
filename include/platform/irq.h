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
#include <platform/cortex_m.h>

void irq_init(void);

static inline void irq_disable(void)
{
	__asm__ __volatile__ ("cpsid i");
}

static inline void irq_enable(void)
{
	__asm__ __volatile__ ("cpsie i");
}

static inline void irq_svc(void)
{
	__asm__ __volatile__ ("svc #0");
}

static inline int irq_number(void)
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
 * Saves {r4-r11}, msp, psp
 */
#define __irq_save(ctx)		\
	__asm__ __volatile__ ("mov r0, %0"				\
			: : "r" ((ctx)->regs) : "r0");			\
	__asm__ __volatile__ ("stm r0, {r4-r11}");			\
	__asm__ __volatile__ ("and r4, lr, 0xf":::"r4");		\
	__asm__ __volatile__ ("teq r4, #0x9");				\
	__asm__ __volatile__ ("ite eq");				\
	__asm__ __volatile__ ("mrseq r0, msp"::: "r0");			\
	__asm__ __volatile__ ("mrsne r0, psp"::: "r0");			\
	__asm__ __volatile__ ("mov %0, r0" : "=r" ((ctx)->sp));	\
	__asm__ __volatile__ ("mov %0, lr" : "=r" ((ctx)->ret));

#ifdef CONFIG_FPU
#define irq_save(ctx)							\
	__asm__ __volatile__ ("cpsid i");				\
	(ctx)->fp_flag = 0;						\
	__irq_save(ctx);						\
	__asm__ __volatile__ ("tst lr, 0x10");				\
	__asm__ __volatile__ ("bne no_fp");				\
	__asm__ __volatile__ ("mov r3, %0"				\
			: : "r" ((ctx)->fp_regs) : "r3");		\
	__asm__ __volatile__ ("vstm r3!, {d8-d15}"			\
			::: "r3");					\
	__asm__ __volatile__ ("mov r4, 0x1": : :"r4");			\
	__asm__ __volatile__ ("stm r3, {r4}");				\
	__asm__ __volatile__ ("no_fp:");
#else	/* ! CONFIG_FPU */
#define irq_save(ctx)							\
	__asm__ __volatile__ ("cpsid i");				\
	__irq_save(ctx);
#endif

#define __irq_restore(ctx)						\
	__asm__ __volatile__ ("mov lr, %0" : : "r" ((ctx)->ret));	\
	__asm__ __volatile__ ("mov r0, %0" : : "r" ((ctx)->sp));	\
	__asm__ __volatile__ ("mov r2, %0" : : "r" ((ctx)->ctl));	\
	__asm__ __volatile__ ("and r4, lr, 0xf":::"r4");		\
	__asm__ __volatile__ ("teq r4, #0x9");				\
	__asm__ __volatile__ ("ite eq");				\
	__asm__ __volatile__ ("msreq msp, r0");				\
	__asm__ __volatile__ ("msrne psp, r0");				\
	__asm__ __volatile__ ("mov r0, %0" : : "r" ((ctx)->regs));	\
	__asm__ __volatile__ ("ldm r0, {r4-r11}");			\
	__asm__ __volatile__ ("msr control, r2");

#ifdef CONFIG_FPU
#define irq_restore(ctx)						\
	__irq_restore(ctx);						\
	if ((ctx)->fp_flag) {						\
		__asm__ __volatile__ ("mov r0, %0" 			\
				: : "r" ((ctx)->fp_regs): "r0");	\
		__asm__ __volatile__ ("vldm r0, {d8-d15}");		\
	}								\
	__asm__ __volatile__ ("cpsie i");
#else	/* ! CONFIG_FPU */
#define irq_restore(ctx) \
	__irq_restore(ctx);						\
	__asm__ __volatile__ ("cpsie i");
#endif

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

#define irq_enter()							\
	__asm__ __volatile__ ("push {lr}");

#define irq_return()							\
	__asm__ __volatile__ ("pop {lr}");				\
	__asm__ __volatile__ ("bx lr");

#define context_switch(from, to)					\
	{								\
		__asm__ __volatile__ ("pop {lr}");			\
		irq_save(&(from)->ctx);					\
		thread_switch((to));					\
		irq_restore(&(from)->ctx);				\
		__asm__ __volatile__ ("bx lr");				\
	}

#define schedule_in_irq()						\
	{								\
		register tcb_t *sel;					\
		sel = schedule_select();				\
		if (sel != current)					\
			context_switch(current, sel);			\
	}

#define request_schedule()						\
	do { *SCB_ICSR |= SCB_ICSR_PENDSVSET; } while (0)


#define NO_PREEMPTED_IRQ						\
	(*SCB_ICSR & SCB_ICSR_RETTOBASE)

/*
 * Context is switched when the selected thead is different
 * to current then do context switch.
 *
 * Idea is that on interrupt we'll save all registers under
 * irq_stack_pointer than on return we copy registers to
 * thread's structure or to kernel_ctx
 * */

#define IRQ_HANDLER(name, sub)						\
	void name(void) __NAKED;					\
	void name(void)							\
	{								\
		irq_enter();						\
		sub();							\
		request_schedule();					\
		irq_return();						\
	}

extern volatile tcb_t *current;

#endif	/* PLATFORM_IRQ_H_ */
