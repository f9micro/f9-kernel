/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <debug.h>
#include <error.h>

#include <syscall.h>
#include <ktimer.h>
#include <init_hook.h>

extern void __l4_start(void);
extern void memmanage_handler(void);
extern void debugmon_handler(void);
extern void pendsv_handler(void);

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
	pendsv_handler,			/* PendSV handler */
	ktimer_handler, 		/* SysTick handler */

	/* Chip Level: vendor specific */
	/* FIXME: use better IRQ vector generator */
#include INC_PLAT(nvic_table.h)
};

#define MAX(a, b)			\
	((a) > (b) ? (a) : (b))

extern const init_struct init_hook_start[];
extern const init_struct init_hook_end[];
static unsigned int last_level = 0;

int run_init_hook(unsigned int level)
{
	unsigned int max_called_level = last_level;

	for (const init_struct *ptr = init_hook_start;
	     ptr != init_hook_end; ++ptr)
		if ((ptr->level > last_level) &&
		    (ptr->level <= level)) {
			max_called_level = MAX(max_called_level, ptr->level);
			ptr->hook();
		}

	last_level = max_called_level;

	return last_level;
}
