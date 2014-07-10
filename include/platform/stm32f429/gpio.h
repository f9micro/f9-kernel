/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_STM32F429_GPIO_H_
#define PLATFORM_STM32F429_GPIO_H_

#include <platform/stm32f429/registers.h>

enum {
	AF0 = 0,
	AF1,
	AF2,
	AF3,
	AF4,
	AF5,
	AF6,
	AF7,
	AF8,
	AF9,
	AF10,
	AF11,
	AF12,
	AF13,
	AF14,
	AF15,
};

struct gpio_cfg {
	uint8_t port;
	uint8_t pin;
	uint8_t pupd;
	uint8_t speed;
	uint8_t type;
	uint8_t func;
	uint8_t o_type;
};

/* GPIO Alternative Function */
#define     af_system           AF0
#define     af_tim1             AF1
#define     af_tim2             AF1
#define     af_tim3             AF2
#define     af_tim4             AF2
#define     af_tim5             AF2
#define     af_tim8             AF3
#define     af_tim9             AF3
#define     af_tim10            AF3
#define     af_tim11            AF3
#define     af_i2c1             AF4
#define     af_i2c2             AF4
#define     af_i2c3             AF4
#define     af_spi1             AF5
#define     af_spi2             AF5
#define     af_spi5             AF5
#define     af_spi3             AF6
#define     af_usart1           AF7
#define     af_usart2           AF7
#define     af_usart3		AF7
#define     af_uart4		AF8
#define     af_uart5		AF8
#define     af_usart6           AF8
#define     af_can1             AF9
#define     af_can2             AF9
#define     af_tim12            AF9
#define     af_tim13            AF9
#define     af_tim14            AF9
#define     af_otg_fs           AF10
#define     af_otg_hs           AF10
#define     af_eth              AF11
#define     af_fsmc             AF12
#define     af_fmc              AF12
#define     af_sdio             AF12
#define     af_dcmi             AF13
#define     af_ltdc             AF14
#define     af_eventout         AF15

void gpio_config(struct gpio_cfg *cfg);
void gpio_config_output(uint8_t port, uint8_t pin, uint8_t pupd, uint8_t speed);
void gpio_config_input(uint8_t port, uint8_t pin, uint8_t pupd);
void gpio_out_high(uint8_t port, uint8_t pin);
void gpio_out_low(uint8_t port, uint8_t pin);
uint8_t gpio_input_bit(uint8_t port, uint8_t pin);
void gpio_writebit(uint8_t port, uint8_t pin, uint8_t bitval);

#endif /* PLATFORM_STM32F429_GPIO_H_ */
