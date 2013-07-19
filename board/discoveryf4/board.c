/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/stm32f4/gpio.h>
#include <platform/stm32f4/usart.h>

struct usart_dev console_uart = {
	.u_num = 4,
	.baud = 115200,
	.base = UART4_BASE,
	.rcc_apbenr = RCC_UART4_APBENR,
	.rcc_reset = RCC_APB1RSTR_USART4RST,
	.tx = {
		.port = GPIOA,
		.pin = 0,
		.pupd = GPIO_PUPDR_NONE,
		.type = GPIO_MODER_ALT,
		.func = af_uart4,
		.o_type = GPIO_OTYPER_PP,
		.speed = GPIO_OSPEEDR_50M,
	},
	.rx = {
		.port = GPIOA,
		.pin = 1,
		.pupd = GPIO_PUPDR_NONE,
		.type = GPIO_MODER_ALT,
		.func = af_uart4,
		.o_type = GPIO_OTYPER_PP,
		.speed = GPIO_OSPEEDR_50M,
	},
};
