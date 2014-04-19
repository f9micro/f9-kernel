/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4io.h>
#include <l4/ipc.h>
#include <thread.h>
#include <platform/link.h>
#include <gpioer.h>

#define GPIO_CMD_CONFIG_OUTPUT 1
#define GPIO_CMD_OUT  2

void __USER_TEXT gpioer_config_output(uint8_t port, uint8_t pin, uint8_t pupd, uint8_t speed) 
{
	L4_Msg_t msg;
	va_list va;

	L4_MsgClear(&msg);
	L4_MsgAppendWord(&msg, (L4_Word_t)GPIO_CMD_CONFIG_OUTPUT);
	L4_MsgAppendWord(&msg, (L4_Word_t)port);
	L4_MsgAppendWord(&msg, (L4_Word_t)pin);
	L4_MsgAppendWord(&msg, (L4_Word_t)pupd);
	L4_MsgAppendWord(&msg, (L4_Word_t)speed);

	L4_MsgLoad(&msg);
	L4_Send((L4_ThreadId_t) {
		.raw = TID_TO_GLOBALID(THREAD_GPIOER)
	});

	va_end(va);
}

void __USER_TEXT gpioer_out(uint8_t port, uint8_t pin, uint8_t action)
{
	L4_Msg_t msg;
	va_list va;

	L4_MsgClear(&msg);
	L4_MsgAppendWord(&msg, (L4_Word_t)GPIO_CMD_OUT);
	L4_MsgAppendWord(&msg, (L4_Word_t)port);
	L4_MsgAppendWord(&msg, (L4_Word_t)pin);
	L4_MsgAppendWord(&msg, (L4_Word_t)action);

	L4_MsgLoad(&msg);
	L4_Send((L4_ThreadId_t) {
		.raw = TID_TO_GLOBALID(THREAD_GPIOER)
	});

	va_end(va);
}
