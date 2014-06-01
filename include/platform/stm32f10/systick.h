#ifndef PLATFORM_STM32F10_SYSTICK_H_
#define PLATFORM_STM32F10_SYSTICK_H_

#include <platform/stm32f10/registers.h>

#define CORE_CLOCK		(0x02dc6c00) /* 48MHz */
#define SYSTICK_MAXRELOAD	(0x00ffffff)

void init_systick(uint32_t tick_reload, uint32_t tick_next_reload);
void systick_disable(void);
uint32_t systick_now(void);
uint32_t systick_flag_count(void);

#endif
