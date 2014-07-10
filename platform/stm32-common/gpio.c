/* Copyright (c) 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include INC_PLAT(gpio.h)
#include <error.h>

#include INC_PLAT(gpio.c)

inline static void gpio_moder(uint8_t port, uint8_t pin, uint8_t type)
{
	uint32_t reg = *GPIO_MODER(port);

	reg &= ~(GPIO_MODER_M(pin));
	reg |= (type << GPIO_MODER_PIN(pin));

	*GPIO_MODER(port) = reg;
}

inline static void gpio_otyper(uint8_t port, uint8_t pin, uint8_t type)
{
	uint32_t reg = *GPIO_OTYPER(port);

	reg &= ~(GPIO_OTYPER_M(pin));
	reg |= (type << GPIO_OTYPER_PIN(pin));

	*GPIO_OTYPER(port) = reg;
}

inline static void gpio_ospeedr(uint8_t port, uint8_t pin, uint8_t speed)
{
	uint32_t reg = *GPIO_OSPEEDR(port);

	reg &= ~(GPIO_OSPEEDR_M(pin));
	reg |= (speed << GPIO_OSPEEDR_PIN(pin));

	*GPIO_OSPEEDR(port) = reg;
}

inline static void gpio_pupdr(uint8_t port, uint8_t pin, uint8_t mode)
{
	uint32_t reg = *GPIO_PUPDR(port);

	reg &= ~(GPIO_PUPDR_M(pin));
	reg |= (mode << GPIO_PUPDR_PIN(pin));

	*GPIO_PUPDR(port) = reg;
}

inline static void gpio_afr(uint8_t port, uint8_t pin, uint8_t func)
{
	uint32_t reg;

	if (pin < 8) {
		reg = *GPIO_AFRL(port);
		reg &= ~(GPIO_AFRL_M(pin));
		reg |= (func << GPIO_AFRL_PIN(pin));
		*GPIO_AFRL(port) = reg;
	} else {
		reg = *GPIO_AFRH(port);
		reg &= ~(GPIO_AFRH_M(pin));
		reg |= (func << GPIO_AFRH_PIN(pin));
		*GPIO_AFRH(port) = reg;
	}
}

void __USER_TEXT gpio_config(struct gpio_cfg *cfg)
{
	uint8_t port, pin, cfg_data;

	port = cfg->port;
	pin = cfg->pin;

	*RCC_AHB1ENR |= (1 << port);

	/* pupd */
	cfg_data = cfg->pupd;
	gpio_pupdr(port, pin, cfg_data);

	/* mode type */
	cfg_data = cfg->type;
	gpio_moder(port, pin, cfg_data);

	if (cfg_data == GPIO_MODER_IN)
		return;

	/* Alternative function */
	if (cfg_data == GPIO_MODER_ALT) {
		uint8_t func = cfg->func;
		gpio_afr(port, pin, func);
	}

	/* Sets pin output type */
	cfg_data = cfg->o_type;
	gpio_otyper(port, pin, cfg_data);

	/* Speed */
	cfg_data = cfg->speed;
	gpio_ospeedr(port, pin, cfg_data);
}

void __USER_TEXT gpio_config_output(uint8_t port, uint8_t pin, uint8_t pupd, uint8_t speed)
{
	struct  gpio_cfg cfg = {
		.port = port,
		.pin = pin,
		.pupd = pupd,
		.type = GPIO_MODER_OUT,
		.func = 0,
		.o_type = GPIO_OTYPER_PP,
		.pupd = pupd,
		.speed = speed,
	};

	*RCC_AHB1ENR |= (1 << port);
	gpio_config(&cfg);
}

void __USER_TEXT gpio_config_input(uint8_t port, uint8_t pin, uint8_t pupd)
{
	struct  gpio_cfg cfg = {
		.port = port,
		.pin = pin,
		.pupd = pupd,
		.type = GPIO_MODER_IN,
		.func = 0,
		.o_type = 0,
		.pupd = pupd,
		.speed = 0,
	};

	*RCC_AHB1ENR |= (1 << port);
	gpio_config(&cfg);
}

void __USER_TEXT gpio_out_high(uint8_t port, uint8_t pin)
{
	*GPIO_ODR(port) |= (1 << pin);
}

void __USER_TEXT gpio_out_low(uint8_t port, uint8_t pin)
{
	*GPIO_ODR(port) &= ~(1 << pin);
}

uint8_t __USER_TEXT gpio_input_bit(uint8_t port, uint8_t pin)
{
	if (*GPIO_IDR(port) & (1 << pin))
		return 1;

	return 0;
}

void __USER_TEXT gpio_writebit(uint8_t port, uint8_t pin, uint8_t bitval)
{
	if (bitval != 0)
		*GPIO_BSRR(port) = GPIO_BSRR_BS(pin);
	else
		*GPIO_BSRR(port) = GPIO_BSRR_BR(pin);
}
