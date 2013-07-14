#ifndef PLATFORM_STM32F4_SYSTICK_H_
#define PLATFORM_STM32F4_SYSTICK_H_

#include <platform/stm32f4/registers.h>

void init_systick(uint32_t tick_reload);

#endif

