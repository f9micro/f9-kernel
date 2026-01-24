/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/irq.h>
#include "board.h"

/*
 * Global to hold saved r4-r11 across irq_save_regs_only / irq_save.
 * Used to capture registers before compiler can corrupt them.
 * See irq.h for usage.
 */
uint32_t __irq_saved_regs[8];

void irq_init(void)
{
	/* Set all 4-bit to pre-emption priority bit */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* Set default priority from high to low */

	NVIC_SetPriority(DebugMonitor_IRQn, 0x0, 0);

	NVIC_SetPriority(MemoryManagement_IRQn, 0x1, 0);
	NVIC_SetPriority(BusFault_IRQn, 0x1, 0);
	NVIC_SetPriority(UsageFault_IRQn, 0x1, 0);

	NVIC_SetPriority(SysTick_IRQn, 0x3, 0);

	/* SVCall and PendSV at lowest priority */
	NVIC_SetPriority(SVCall_IRQn, 0xF, 0);
	NVIC_SetPriority(PendSV_IRQn, 0xF, 0);
}

#ifndef LOADER
void pendsv_handler(void) __NAKED;
void pendsv_handler(void)
{
	/* Save r4-r11 FIRST before any C code can corrupt them */
	irq_save_regs_only();
	irq_enter();
	schedule_in_irq();
	irq_return();
}
#endif
