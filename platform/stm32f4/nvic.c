/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/stm32f4/nvic.h>
#include <platform/cortex_m.h>

#define AIRCR_VECTKEY_MASK    ((uint32_t) 0x05FA0000)

static PFN_ISR irq_handler[MAX_IRQn + 1];
static uint32_t irq_handler_input[MAX_IRQn + 1];

#define IRQ_VEC_N_HANDLER(n)					\
	void nvic_handler##n() {				\
		if (irq_handler[n])				\
			irq_handler[n](irq_handler_input[n]);	\
		else						\
			while (1);				\
	}

#define IRQ_VEC_N_OP	IRQ_VEC_N_HANDLER
#include "platform/stm32f4/nvic_private.h"
#undef IRQ_VEC_N_OP

void NVIC_PriorityGroupConfig(uint32_t NVIC_PriorityGroup)
{
	if (!(NVIC_PriorityGroup == NVIC_PriorityGroup_0 ||
	      NVIC_PriorityGroup == NVIC_PriorityGroup_1 ||
	      NVIC_PriorityGroup == NVIC_PriorityGroup_2 ||
	      NVIC_PriorityGroup == NVIC_PriorityGroup_3 ||
	      NVIC_PriorityGroup == NVIC_PriorityGroup_4))
		return;

	*SCB_AIRCR = AIRCR_VECTKEY_MASK | NVIC_PriorityGroup;
}

void NVIC_SetPriority(IRQn_Type IRQn, uint8_t group_priority,
		uint8_t sub_priority)
{
	uint8_t priority = 0x0, group_shifts = 0x0, sub_shifts = 0x0;

	sub_shifts = (0x700 - ((*SCB_AIRCR) & (uint32_t) 0x700)) >> 0x08;
	group_shifts = 0x4 - sub_shifts;

	priority = (group_priority << group_shifts) |
			(sub_priority & (0xf >> sub_shifts));

	NVIC->IP[IRQn] = priority << 0x4;
}

void NVIC_intAttached(IRQn_Type IRQn, PFN_ISR handler,
		uint32_t handler_input)
{
	__asm__ __volatile__ ("cpsid  i");

	if (IRQn < 0 || IRQn > MAX_IRQn)
		return;

	irq_handler[IRQn] = handler;
	irq_handler_input[IRQn] = handler_input;

	__asm__ __volatile__ ("cpsie  i");
}
