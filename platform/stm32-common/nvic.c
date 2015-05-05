/* Copyright (c) 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include INC_PLAT(nvic.h)
#include <platform/cortex_m.h>
#include <platform/irq.h>

#include INC_PLAT(nvic.c)

#define AIRCR_VECTKEY_MASK    ((uint32_t) 0x05FA0000)

void _undefined_handler(void)
{
	while (1)
		/* wait */ ;
}

/* for the sake of saving space, provide default device IRQ handler with
 * weak alias.
 */
#define DEFAULT_IRQ_VEC(n)						\
	void nvic_handler##n(void)					\
		__attribute__((weak, alias("_undefined_handler")));

#define IRQ_VEC_N_OP	DEFAULT_IRQ_VEC
#include INC_PLAT(nvic_private.h)
#undef IRQ_VEC_N_OP

extern void (* const g_pfnVectors[])(void);
int nvic_is_setup(int irq)
{
	return !(g_pfnVectors[irq + 16] == _undefined_handler);
}

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

	if (IRQn < 0)
		((volatile uint8_t *) SCB_SHPR)[
		        (((uint32_t) IRQn) & 0xf) - 4] = priority << 0x4;
	else
		NVIC->IP[IRQn] = priority << 0x4;
}
