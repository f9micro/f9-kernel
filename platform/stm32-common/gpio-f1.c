/* Copyright (c) 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include INC_PLAT(gpio.h)
#include <error.h>

#include INC_PLAT(gpio.c)

inline static void gpio_cr(uint8_t port, uint8_t pin, uint8_t mode, uint8_t ospeed)
{
	uint32_t reg;
	if (pin < 8) {
		reg = *GPIO_CRL(port);
		reg &= ~(GPIO_CR_M(pin));
		reg |= (((mode << 2) | ospeed) << GPIO_CR_PIN(pin));
		*GPIO_CRL(port) = reg;
	} else {
		reg = *GPIO_CRH(port);
		reg &= ~(GPIO_CR_M(pin));
		reg |= (((mode << 2) | ospeed) << GPIO_CR_PIN((pin-8)));
		*GPIO_CRH(port) = reg;
	}
}

void __USER_TEXT gpio_config(struct gpio_cfg *cfg)
{
	uint8_t port, pin, cfg_data;

	port = cfg->port;
	pin = cfg->pin;

	*RCC_APB2ENR |= (1 << (port + 2));

	/* Mode and speed */
	gpio_cr(port, pin, cfg->mode, cfg->ospeed);
	
	cfg_data = cfg->mode;
	/* Input Push up or push down */
	if ((((uint32_t)cfg_data) & ((uint32_t)0x10)) != 0x00) {
		if (cfg_data == GPIO_MODE_IN_PD)
			*GPIO_BRR(port) = GPIO_BRR_BR(pin);
		else if (cfg_data == GPIO_MODE_IN_PU)
			*GPIO_BSRR(port) = GPIO_BSRR_BS(pin);
		return;
	}
}

void __USER_TEXT gpio_config_output(uint8_t port, uint8_t pin, uint8_t mode, uint8_t ospeed)
{
	struct  gpio_cfg cfg = {
		.port = port,
		.pin = pin,
		.mode = mode,
		.ospeed = ospeed,
		.func = 0,
	};

	gpio_config(&cfg);
}

void __USER_TEXT gpio_config_input(uint8_t port, uint8_t pin, uint8_t mode)
{
	struct  gpio_cfg cfg = {
		.port = port,
		.pin = pin,
		.mode = mode,
		.ospeed = 0,
		.func = 0,
	};

	gpio_config(&cfg);
}

void __USER_TEXT gpio_out_high(uint8_t port, uint8_t pin)
{
	*GPIO_ODR(port) |= GPIO_ODR_PIN(pin);
}

void __USER_TEXT gpio_out_low(uint8_t port, uint8_t pin)
{
	*GPIO_ODR(port) &= ~GPIO_ODR_PIN(pin);
}

uint8_t __USER_TEXT gpio_input_bit(uint8_t port, uint8_t pin)
{
	if (*GPIO_IDR(port) & GPIO_IDR_PIN(pin))
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
