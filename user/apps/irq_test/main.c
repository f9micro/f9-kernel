#include INC_PLAT(nvic.h)
#include INC_PLAT(registers.h)
#include INC_PLAT(exti.h)

#include <l4io.h>
#include <platform/link.h>
#include <user_runtime.h>
#include <user_interrupt.h>
#include "user_exti.h"

__USER_BSS
static uint32_t exti0_interrupt_num = 0;

__USER_BSS
static uint32_t exti1_interrupt_num = 0;

__USER_TEXT
void exti0_interrupt_handler(void)
{
	exti_clear(0);
	L4_Sleep(L4_TimePeriod(500 * 1000));
	exti0_interrupt_num++;
	exti_launch_sw_interrupt(1);
	printf("%d: %s\n", L4_MyGlobalId(), __func__);
}

__USER_TEXT
void exti1_interrupt_handler(void)
{
	exti_clear(1);
	L4_Sleep(L4_TimePeriod(500 * 1000));
	exti1_interrupt_num++;
	exti_launch_sw_interrupt(0);
	printf("%d: %s\n", L4_MyGlobalId(), __func__);
}

__USER_TEXT
static void *main(void *user)
{
	request_irq(EXTI0_IRQn, exti0_interrupt_handler, 1);
	request_irq(EXTI1_IRQn, exti1_interrupt_handler, 1);
	exti_config(0, EXTI_INTERRUPT_MODE, EXTI_RISING_TRIGGER);
	exti_config(1, EXTI_INTERRUPT_MODE, EXTI_RISING_TRIGGER);

	exti_launch_sw_interrupt(0);

	/* Sleep 10s */
	L4_Sleep(L4_TimePeriod(10000000));
	free_irq(EXTI0_IRQn);
	free_irq(EXTI1_IRQn);

	return 0;
}

DECLARE_USER(
	128,
	irq_test,
	main,
	DECLARE_FPAGE(0x0, 4 * (UTCB_SIZE + IRQ_STACK_SIZE))
	DECLARE_FPAGE(0x0, 512)
	DECLARE_FPAGE(0x40010000, 0x4000)
);
