#ifndef PLATFORM_STM32F1_EXTI_H_
#define PLATFORM_STM32F1_EXTI_H_

#include <platform/stm32f1/registers.h>
#include <platform/stm32f1/gpio.h>

/* EXTI mode */
#define EXTI_INTERRUPT_MODE				0x0
#define EXTI_EVENT_MODE					0x4

/* EXTI trigger type */
#define EXTI_RISING_TRIGGER				0x8
#define EXTI_FALLING_TRIGGER			0xc
#define EXTI_RISING_FALLING_TRIGGER		0x10

#define EXTI_LINE(x)	((uint32_t)0x1 << x)

struct exti_regs {
	volatile uint32_t IMR;
	volatile uint32_t EMR;
	volatile uint32_t RTSR;
	volatile uint32_t FTSR;
	volatile uint32_t SWIER;
	volatile uint32_t PR;
};


#endif
