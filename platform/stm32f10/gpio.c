/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/stm32f10/gpio.h>
#include <error.h>

void gpio_config(struct gpio_cfg *cfg)
{
	uint8_t port, pin;
	volatile uint32_t *gpio_cr;
	uint32_t reg;

	port = cfg->port;
	pin = cfg->pin;

	*RCC_APB2ENR |= (1 << (port + 2));

	if (pin < 8) { 
		/* low */
		gpio_cr = GPIO_CRL(port);
	} else { 
		/* high */
		gpio_cr = GPIO_CRH(port);
	}

	reg = *gpio_cr;
	reg &= ~(GPIO_CR_M(pin));
	reg |= (((cfg->cnf << 2) | cfg->mode) << GPIO_CR_PIN(pin));

	*gpio_cr = reg;
}

void gpio_config_output(uint8_t port, uint8_t pin, uint8_t cnf, uint8_t mode)
{
	struct  gpio_cfg cfg = {
		.port = port,
		.pin = pin,
		.cnf = cnf,
		.mode = mode,
		.func = 0,
	};

	*RCC_APB2ENR |= (1 << (port + 2));
	gpio_config(&cfg);
}

void gpio_config_input(uint8_t port, uint8_t pin, uint8_t cnf, uint8_t mode)
{
	struct  gpio_cfg cfg = {
		.port = port,
		.pin = pin,
		.cnf = cnf,
		.mode = mode,
		.func = 0,
	};

	*RCC_APB2ENR |= (1 << (port + 2));
	gpio_config(&cfg);
}

void gpio_out_high(uint8_t port, uint8_t pin)
{
	*GPIO_ODR(port) |= (1 << pin);
}

void gpio_out_low(uint8_t port, uint8_t pin)
{
	*GPIO_ODR(port) &= ~(1 << pin);
}

uint8_t gpio_input_bit(uint8_t port, uint8_t pin)
{
	if (*GPIO_IDR(port) & (1 << pin))
		return 1;

	return 0;
}
