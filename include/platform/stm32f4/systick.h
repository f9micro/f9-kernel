#ifndef PLATFORM_STM32F4_SYSTICK_H_
#define PLATFORM_STM32F4_SYSTICK_H_

#include <platform/stm32f4/registers.h>

#define SYSTICK_MAXRELOAD (0x00ffffff)

void init_systick(uint32_t tick_reload);
void systick_disable(void);
uint32_t systick_now(void);
uint32_t systick_load(void);
uint32_t systick_flag_count(void);

#endif

