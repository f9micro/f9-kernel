/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/irq.h>
#include "board.h"

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

	/* Priority 0xF - debug_uart */
	NVIC_SetPriority(SVCall_IRQn, 0xF, 0);
	NVIC_SetPriority(PendSV_IRQn, 0xF, 0);
}

#ifndef LOADER
void pendsv_handler(void) __NAKED;
void pendsv_handler(void)
{
	irq_enter();
	schedule_in_irq();
	irq_return();
}
#endif
