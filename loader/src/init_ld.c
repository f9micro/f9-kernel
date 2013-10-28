/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/debug_uart.h>
#include <error.h>


extern void __loader_start(void);

void nointerrupt(void)
{
	while (1)
	/* wait */;
}

void systick_handler(void)
{
	return;
}

/*
 * Declare NVIC table
 */

extern void (* const g_pfnVectors[])(void);

#include INC_PLAT(nvic.h)

__ISR_VECTOR
void (* const g_pfnVectors[])(void) = {
	/* Core Level - ARM Cortex-M */
	(void *) &stack_end,	/* initial stack pointer */
	__loader_start,			/* reset handler */
	nointerrupt,			/* NMI handler */
	nointerrupt,		/* hard fault handler */
	nointerrupt,		/* MPU fault handler */
	nointerrupt,			/* bus fault handler */
	nointerrupt,			/* usage fault handler */
	0,				/* Reserved */
	0,				/* Reserved */
	0,				/* Reserved */
	0,				/* Reserved */
	nointerrupt,			/* SVCall handler */
	nointerrupt,			/* Debug monitor handler */
	0,				/* Reserved */
	nointerrupt,			/* PendSV handler */
	systick_handler, 		/* SysTick handler */
	/* Chip Level: vendor specific */
	/* FIXME: use better IRQ vector generator */
	#include INC_PLAT(nvic_table.h)
};
