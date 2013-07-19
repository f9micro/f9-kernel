/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/debug_uart.h>

#include <debug.h>
#include <error.h>

#include <syscall.h>
#include <ktimer.h>

extern void __l4_start();
extern void memmanage_handler();

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

void hard_fault_handler()
{
	panic("Kernel panic: Hard fault. Restarting\n");
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
	nointerrupt,			/* Debug monitor handler */
	0,				/* Reserved */
	nointerrupt,			/* PendSV handler */
	ktimer_handler, 		/* SysTick handler */

	/* Chip Level */
	PLAT_NVIC_ENTRIES
};
