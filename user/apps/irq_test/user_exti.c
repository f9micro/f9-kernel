#include INC_PLAT(registers.h)
#include INC_PLAT(exti.h)
#include "user_exti.h"

__USER_TEXT
void exti_config(uint32_t line, uint32_t mode, uint32_t trigger_type)
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
 void exti_enable(uint32_t line, uint32_t mode)
{
	*((volatile uint32_t *)(EXTI_BASE + mode)) |= EXTI_LINE(line);
}

__USER_TEXT
void exti_disable(uint32_t line)
{
	struct exti_regs *exti_regs = (struct exti_regs *)EXTI_BASE;

	/* Clear EXTI mask */
	exti_regs->IMR &= ~EXTI_LINE(line);
	exti_regs->EMR &= ~EXTI_LINE(line);
}

__USER_TEXT
void exti_launch_sw_interrupt(uint32_t line)
{
	struct exti_regs *exti_regs = (struct exti_regs *)EXTI_BASE;

	exti_regs->SWIER |= EXTI_LINE(line);
}

__USER_TEXT
void exti_clear(uint32_t line)
{
	struct exti_regs *exti_regs = (struct exti_regs *)EXTI_BASE;

	exti_regs->PR = EXTI_LINE(line);
}

__USER_TEXT
int exti_interrupt_status(uint32_t line)
{
	struct exti_regs *exti_regs = (struct exti_regs *)EXTI_BASE;

	return (exti_regs->PR & EXTI_LINE(line)) && (exti_regs->IMR & EXTI_LINE(line));
}

__USER_TEXT
void exti_interrupt_status_clear(uint32_t line)
{
	exti_clear(line);
}
