/* Copyright (c) 2013-2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/stm32f10/gpio.h>
#include <platform/stm32f10/usart.h>
#include "board.h"

struct usart_dev console_uart = {
	.u_num = 4,
	.baud = 115200,
	BOARD_USART_CONFIGS
	.tx = {
		.port = BOARD_USART_TX_IO_PORT,
		.pin = BOARD_USART_TX_IO_PIN,
		.cnf = GPIO_CNF_OUT_ALT_PP,
		.mode = GPIO_MODE_OUT_50M,
		.func = BOARD_USART_FUNC,
	},
	.rx = {
		.port = BOARD_USART_RX_IO_PORT,
		.pin = BOARD_USART_RX_IO_PIN,
		.cnf = GPIO_CNF_OUT_ALT_PP,
		.mode = GPIO_MODE_OUT_50M,
		.func = BOARD_USART_FUNC,
	},
};
