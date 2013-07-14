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

IRQ_VEC_N_HANDLER(0);
IRQ_VEC_N_HANDLER(1);
IRQ_VEC_N_HANDLER(2);
IRQ_VEC_N_HANDLER(3);
IRQ_VEC_N_HANDLER(4);
IRQ_VEC_N_HANDLER(5);
IRQ_VEC_N_HANDLER(6);
IRQ_VEC_N_HANDLER(7);
IRQ_VEC_N_HANDLER(8);
IRQ_VEC_N_HANDLER(9);
IRQ_VEC_N_HANDLER(10);
IRQ_VEC_N_HANDLER(11);
IRQ_VEC_N_HANDLER(12);
IRQ_VEC_N_HANDLER(13);
IRQ_VEC_N_HANDLER(14);
IRQ_VEC_N_HANDLER(15);
IRQ_VEC_N_HANDLER(16);
IRQ_VEC_N_HANDLER(17);
IRQ_VEC_N_HANDLER(18);
IRQ_VEC_N_HANDLER(19);
IRQ_VEC_N_HANDLER(20);
IRQ_VEC_N_HANDLER(21);
IRQ_VEC_N_HANDLER(22);
IRQ_VEC_N_HANDLER(23);
IRQ_VEC_N_HANDLER(24);
IRQ_VEC_N_HANDLER(25);
IRQ_VEC_N_HANDLER(26);
IRQ_VEC_N_HANDLER(27);
IRQ_VEC_N_HANDLER(28);
IRQ_VEC_N_HANDLER(29);
IRQ_VEC_N_HANDLER(30);
IRQ_VEC_N_HANDLER(31);
IRQ_VEC_N_HANDLER(32);
IRQ_VEC_N_HANDLER(33);
IRQ_VEC_N_HANDLER(34);
IRQ_VEC_N_HANDLER(35);
IRQ_VEC_N_HANDLER(36);
IRQ_VEC_N_HANDLER(37);
IRQ_VEC_N_HANDLER(38);
IRQ_VEC_N_HANDLER(39);
IRQ_VEC_N_HANDLER(40);
IRQ_VEC_N_HANDLER(41);
IRQ_VEC_N_HANDLER(42);
IRQ_VEC_N_HANDLER(43);
IRQ_VEC_N_HANDLER(44);
IRQ_VEC_N_HANDLER(45);
IRQ_VEC_N_HANDLER(46);
IRQ_VEC_N_HANDLER(47);
IRQ_VEC_N_HANDLER(48);
IRQ_VEC_N_HANDLER(49);
IRQ_VEC_N_HANDLER(50);
IRQ_VEC_N_HANDLER(51);
IRQ_VEC_N_HANDLER(52);
IRQ_VEC_N_HANDLER(53);
IRQ_VEC_N_HANDLER(54);
IRQ_VEC_N_HANDLER(55);
IRQ_VEC_N_HANDLER(56);
IRQ_VEC_N_HANDLER(57);
IRQ_VEC_N_HANDLER(58);
IRQ_VEC_N_HANDLER(59);
IRQ_VEC_N_HANDLER(60);
IRQ_VEC_N_HANDLER(61);
IRQ_VEC_N_HANDLER(62);
IRQ_VEC_N_HANDLER(63);
IRQ_VEC_N_HANDLER(64);
IRQ_VEC_N_HANDLER(65);
IRQ_VEC_N_HANDLER(66);
IRQ_VEC_N_HANDLER(67);
IRQ_VEC_N_HANDLER(68);
IRQ_VEC_N_HANDLER(69);
IRQ_VEC_N_HANDLER(70);
IRQ_VEC_N_HANDLER(71);
IRQ_VEC_N_HANDLER(72);
IRQ_VEC_N_HANDLER(73);
IRQ_VEC_N_HANDLER(74);
IRQ_VEC_N_HANDLER(75);
IRQ_VEC_N_HANDLER(76);
IRQ_VEC_N_HANDLER(77);
IRQ_VEC_N_HANDLER(78);
IRQ_VEC_N_HANDLER(79);
IRQ_VEC_N_HANDLER(80);
IRQ_VEC_N_HANDLER(81);


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
