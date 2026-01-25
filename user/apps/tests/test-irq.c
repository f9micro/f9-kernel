/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef CONFIG_EXTI_INTERRUPT_TEST

#include INC_PLAT(nvic.h)
#include INC_PLAT(registers.h)
#include INC_PLAT(exti.h)

#include <l4io.h>
#include <platform/link.h>
#include <user_interrupt.h>

#include "test-irq.h"
#include "tests.h"

__USER_BSS
static volatile uint32_t exti0_count;

__USER_BSS
static volatile uint32_t exti1_count;

__USER_TEXT
static void exti_config(uint32_t line, uint32_t mode, uint32_t trigger_type)
{
	struct exti_regs *exti_regs = (struct exti_regs *)EXTI_BASE;

	/* Clear EXTI mask */
	exti_regs->IMR &= ~EXTI_LINE(line);
	exti_regs->EMR &= ~EXTI_LINE(line);

	*((volatile uint32_t *)(EXTI_BASE + mode)) |= EXTI_LINE(line);

	if (trigger_type == EXTI_RISING_FALLING_TRIGGER) {
		exti_regs->RTSR |= EXTI_LINE(line);
		exti_regs->FTSR |= EXTI_LINE(line);
	} else {
		*((volatile uint32_t *)(EXTI_BASE + mode)) |= EXTI_LINE(line);
	}
}

__USER_TEXT
static void exti_launch_sw_interrupt(uint32_t line)
{
	struct exti_regs *exti_regs = (struct exti_regs *)EXTI_BASE;
	exti_regs->SWIER |= EXTI_LINE(line);
}

__USER_TEXT
static void exti_clear(uint32_t line)
{
	struct exti_regs *exti_regs = (struct exti_regs *)EXTI_BASE;
	exti_regs->PR = EXTI_LINE(line);
}

__USER_TEXT
static void exti0_handler(void)
{
	exti_clear(0);
	exti0_count++;
}

__USER_TEXT
static void exti1_handler(void)
{
	exti_clear(1);
	exti1_count++;
}

/*
 * Test: Verify user-space interrupt handling works.
 * Requires CONFIG_EXTI0_USER_IRQ and CONFIG_EXTI1_USER_IRQ.
 */
__USER_TEXT
void test_irq_exti(void)
{
	TEST_RUN("irq_exti");

	exti0_count = 0;
	exti1_count = 0;

	/* Register interrupt handlers */
	request_irq(EXTI0_IRQn, exti0_handler, 1);
	request_irq(EXTI1_IRQn, exti1_handler, 1);

	/* Configure EXTI lines */
	exti_config(0, EXTI_INTERRUPT_MODE, EXTI_RISING_TRIGGER);
	exti_config(1, EXTI_INTERRUPT_MODE, EXTI_RISING_TRIGGER);

	/* Trigger software interrupts */
	exti_launch_sw_interrupt(0);
	L4_Sleep(L4_TimePeriod(10000)); /* 10ms */

	exti_launch_sw_interrupt(1);
	L4_Sleep(L4_TimePeriod(10000)); /* 10ms */

	/* Cleanup */
	free_irq(EXTI0_IRQn);
	free_irq(EXTI1_IRQn);

	/* Verify interrupts fired */
	if (exti0_count > 0 && exti1_count > 0) {
		TEST_PASS("irq_exti");
	} else {
		printf("EXTI counts: exti0=%lu exti1=%lu\n",
		       (unsigned long)exti0_count,
		       (unsigned long)exti1_count);
		TEST_FAIL("irq_exti");
	}
}

#endif /* CONFIG_EXTI_INTERRUPT_TEST */
