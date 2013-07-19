/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/debug_uart.h>
#include <platform/cortex_m.h>

#include <debug.h>
#include <error.h>

#include <syscall.h>
#include <ktimer.h>

extern void __l4_start();
extern void memmanage_handler();
extern void arch_kprobe_handler(uint32_t *stack);

void busfault()
{
	while (1) {
		/* wait */ ;
	}
}

void nointerrupt()
{
	while (1) {
		/* wait */ ;
	}
}

void debugmon_handler() __NAKED;
void hard_fault_handler() __NAKED;

#define enter_frame() \
	uint32_t *stack;                                       \
	__asm__ __volatile__ ("mov %0, sp" : "=r" (stack) : ); \
	__asm__ __volatile__ ("push {lr}");                    \
	__asm__ __volatile__ ("push {r4-r11}");

#define leave_frame() \
	__asm__ __volatile__ ("pop {r4-r11}");                 \
	__asm__ __volatile__ ("pop {pc}");

void debugmon_handler()
{
	enter_frame();

#ifdef CONFIG_KPROBES
	arch_kprobe_handler(stack);
#else
	panic("Kernel panic: DebugMonitor Exception. Restarting");
#endif /* CONFIG_KPROBES */

	leave_frame();
}

void hard_fault_handler()
{
	enter_frame();

#ifdef CONFIG_KPROBES
	/*
	 * We are here when currently executing
	 * priority is higher than or equal to 
	 * the priority of debug exception,
	 * inhibiting normal preemption, then
	 * processor escalates the exception 
	 * priority to HardFault.
	 */
	if (*SCB_HFSR & SCB_HFSR_DEBUGEVT) {
		// break on handler-mode
		arch_kprobe_handler(stack);
		*SCB_HFSR = SCB_HFSR_DEBUGEVT;   // clear dbgevt
		leave_frame();
		return;
	}
#endif /* CONFIG_KPROBES */

	panic("Kernel panic: Hard fault. Restarting\n");
	leave_frame();
}

void nmi_handler()
{
	panic("Kernel panic: NMI. Restarting\n");
}

/*
 * Declare NVIC table
 */

extern void (* const g_pfnVectors[])();

#include INC_PLAT(nvic.h)

__ISR_VECTOR
void (* const g_pfnVectors[])() = {
	/* Core Level - ARM Cortex-M */
	(void *) &kernel_stack_end,	/* initial stack pointer */
	__l4_start,			/* reset handler */
	nmi_handler,			/* NMI handler */
	hard_fault_handler,		/* hard fault handler */
	memmanage_handler,		/* MPU fault handler */
	busfault,			/* bus fault handler */
	nointerrupt,			/* usage fault handler */
	0,				/* Reserved */
	0,				/* Reserved */
	0,				/* Reserved */
	0,				/* Reserved */
	svc_handler,			/* SVCall handler */
	debugmon_handler,		/* Debug monitor handler */
	0,				/* Reserved */
	nointerrupt,			/* PendSV handler */
	ktimer_handler, 		/* SysTick handler */

	/* Chip Level: vendor specific */
	/* FIXME: use better IRQ vector generator */
	#include INC_PLAT(nvic_table.h)
};
