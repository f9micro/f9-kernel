/* Copyright (c) 2013-2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/stm32f1/gpio.h>
#include <platform/stm32f1/usart.h>
#include "board.h"

struct usart_dev console_uart = {
	.u_num = 3,
	.baud = 115200,
	BOARD_USART_CONFIGS
	.tx = {
		.port = BOARD_USART_TX_IO_PORT,
		.pin = BOARD_USART_TX_IO_PIN,
		.mode = GPIO_MODE_OUT_ALT_PP,
		.func = BOARD_USART_FUNC,
		.ospeed = GPIO_OSPEED_50M,
	},
	.rx = {
		.port = BOARD_USART_RX_IO_PORT,
		.pin = BOARD_USART_RX_IO_PIN,
		.mode = GPIO_MODE_OUT_ALT_PP,
		.func = BOARD_USART_FUNC,
		.ospeed = GPIO_OSPEED_50M,
	},
};
