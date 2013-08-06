/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/debug_uart.h>

#include <debug.h>
#include <error.h>

#include <syscall.h>
#include <ktimer.h>

extern void __l4_start(void);
extern void memmanage_handler(void);
extern void debugmon_handler(void);

void busfault(void)
{
	while (1)
		/* wait */ ;
}

void nointerrupt(void)
{
	while (1)
		/* wait */ ;
}

void hard_fault_handler(void)
{
	/*
	 * If we are here, it may mean currently executing priority is higher
	 * than or equal to the priority of fault exception, inhibiting normal
	 * preemption, then processor escalates the exception priority to
	 * HardFault.
	 */
	panic("Kernel panic: Hard fault. Restarting\n");
}

void nmi_handler(void)
{
	panic("Kernel panic: NMI. Restarting\n");
}

/*
 * Declare NVIC table
 */

extern void (* const g_pfnVectors[])(void);

#include INC_PLAT(nvic.h)

__ISR_VECTOR
void (* const g_pfnVectors[])(void) = {
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
#ifdef CONFIG_KPROBES
	debugmon_handler,		/* Debug monitor handler */
#else
	nointerrupt,
#endif
	0,				/* Reserved */
	nointerrupt,			/* PendSV handler */
	ktimer_handler, 		/* SysTick handler */

	/* Chip Level: vendor specific */
	/* FIXME: use better IRQ vector generator */
	#include INC_PLAT(nvic_table.h)
};
