/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/stm32f4/hwtimer.h>
#include <platform/stm32f4/registers.h>

void hwtimer_init()
{
	*RCC_APB1ENR |= 0x00000001;
	*TIM2_PSC = 0;
	*TIM2_ARR = 0xFFFFFFFF;
	*TIM2_CR1 = 0x00000001;
}

uint32_t hwtimer_now()
{
    return *TIM2_CNT;
}
