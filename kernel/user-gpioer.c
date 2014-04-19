/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <user-log.h>
#include <debug.h>
#include <thread.h>
#include <user-gpioer.h>
#include <platform/stm32f4/gpio.h>

#define GPIO_HIGH 1

static inline void gpio_out(uint8_t port, uint8_t pin, uint8_t action)
{
	if (action == GPIO_HIGH)
		gpio_out_high(port, pin);
	else 
		gpio_out_low(port, pin);
}

#define GPIO_CMD_CONFIG_OUTPUT 1
#define GPIO_CMD_OUT  2

static int counter = 0;
void user_gpioer(tcb_t *from)
{
	int cmd= (int)from->ctx.regs[1];
	uint8_t port = (uint8_t)from->ctx.regs[2];
	uint8_t pin = (uint8_t)from->ctx.regs[3];

	switch (cmd) {
		case GPIO_CMD_CONFIG_OUTPUT:
			{
				uint8_t pupd = (uint8_t)from->ctx.regs[4];
				uint8_t speed = (uint8_t)from->ctx.regs[5];
				dbg_printf(DL_KDB, "gpioer config (%d): %d %d %d %d\n", ++counter, port, pin, pupd, speed);
				gpio_config_output(port, pin, pupd, speed);
			}
			break;
		case GPIO_CMD_OUT:
			{
				uint8_t action = (uint8_t)from->ctx.regs[4];
				dbg_printf(DL_KDB, "gpioer out (%d): %d %d %d\n", ++counter, port, pin, action);
				gpio_out(port, pin, action);
			}
			break;
		default:
			dbg_printf(DL_KDB, "gpioer cmd unknown %d (%d): %d %d\n", cmd, ++counter, port, pin);
	}
}
