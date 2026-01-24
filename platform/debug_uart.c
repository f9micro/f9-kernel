/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/debug_device.h>
#include <platform/debug_uart.h>
#include <platform/irq.h>
#include <lib/queue.h>

#ifdef CONFIG_KDB
#include <softirq.h>
#endif

/* board speficic UART definitions */
#include "board.h"

static struct dbg_uart_t dbg_uart;
static uint8_t dbg_uart_tx_buffer[SEND_BUFSIZE];
static uint8_t dbg_uart_rx_buffer[RECV_BUFSIZE];

enum { DBG_ASYNC, DBG_PANIC } dbg_state;

static void dbg_uart_recv(void);
static void dbg_uart_send(int avail);

void __uart_irq_handler(void)
{
	/* For RX: check SR status (RXNE) to ensure data is available.
	 * For TX: check only CR1 interrupt enable (TXEIE) because QEMU's
	 * USART emulation may not properly update SR TXE status bit.
	 * This is safe because TXE interrupt only fires when TXE is set.
	 */
	if (usart_status(&console_uart, USART_RXNE)) {
		/* USART RX - data received */
		dbg_uart_recv();
	}
	if (usart_interrupt_status(&console_uart, USART_IT_TXE)) {
		/* USART TX - transmit buffer empty */
		dbg_uart_send(1);
	}
}

IRQ_HANDLER(BOARD_UART_HANDLER, __uart_irq_handler);

static void dbg_uart_recv(void)
{
	uint8_t chr = usart_getc(&console_uart);

	/* Put sequence on queue */
	queue_push(&(dbg_uart.rx), chr);
#ifndef LOADER
#ifdef CONFIG_KDB
	softirq_schedule(KDB_SOFTIRQ);
#endif
#endif
}

static void dbg_uart_send(int avail)
{
	uint8_t chr;

	if (avail) {
		if (!queue_is_empty(&(dbg_uart.tx))) {
			queue_pop(&(dbg_uart.tx), &chr);
			usart_putc(&console_uart, chr);
			dbg_uart.ready = 0;
			usart_config_interrupt(&console_uart, USART_IT_TXE, 1);
		} else {
			dbg_uart.ready = 1;
			usart_config_interrupt(&console_uart, USART_IT_TXE, 0);
		}
	}
}

static void dbg_async_putchar(char chr)
{
	/* If UART is busy, try to put chr into queue until slot is freed,
	 * else write directly into UART
	 */
	if (!dbg_uart.ready) {
		while (queue_push(&(dbg_uart.tx), chr) != QUEUE_OK)
			/* wait */ ;
	} else {
		usart_putc(&console_uart, chr);
		dbg_uart.ready = 0;
		usart_config_interrupt(&console_uart, USART_IT_TXE, 1);
	}
}

static void dbg_sync_putchar(char chr)
{
	if (chr == '\n')
		dbg_sync_putchar('\r');

	usart_putc(&console_uart, chr);
	while (!usart_status(&console_uart, USART_TC))
		/* wait */ ;
}

uint8_t dbg_uart_getchar(void)
{
	uint8_t chr = 0;

	if (queue_pop(&(dbg_uart.rx), &chr) == QUEUE_EMPTY)
		return 0;
	return chr;

}

void dbg_uart_putchar(uint8_t chr)
{
	/* During panic, we cannot use async dbg uart, so switch to
	 * synchronious mode
	 */
	if (dbg_state != DBG_PANIC)
		dbg_async_putchar(chr);
	else
		dbg_sync_putchar(chr);
}
static void dbg_uart_start_panic(void)
{
	unsigned char chr;

	/* In panic condition, we can be in interrupt context or
	 * not, so will write sequence synchronously */

	/* Flush remaining sequence  in async buffer */
	while (queue_pop(&(dbg_uart.tx), &chr) != QUEUE_EMPTY) {
		dbg_sync_putchar(chr);
	}

	dbg_state = DBG_PANIC;
}

void dbg_uart_init(void)
{
	dbg_dev_t dbg_dev_uart = {
		.dev_id = DBG_DEV_UART,
		.getchar = &dbg_uart_getchar,
		.putchar = &dbg_uart_putchar,
		.start_panic = &dbg_uart_start_panic
	};

	usart_init(&console_uart);

	dbg_uart.ready = 1;
	queue_init(&(dbg_uart.tx), dbg_uart_tx_buffer, SEND_BUFSIZE);
	queue_init(&(dbg_uart.rx), dbg_uart_rx_buffer, RECV_BUFSIZE);

#ifdef CONFIG_QEMU
	/* Use sync output for reliable debugging under QEMU.
	 * QEMU's USART TXE interrupt emulation is unreliable,
	 * causing the async TX queue to block indefinitely.
	 */
	dbg_state = DBG_PANIC;
#else
	dbg_state = DBG_ASYNC;
#endif
	usart_config_interrupt(&console_uart, USART_IT_RXNE, 1);

	/* UART must have higher priority (lower number) than PendSV (0xf)
	 * so it can preempt and drain TX queue during debug output.
	 * Otherwise dbg_async_putchar deadlocks waiting for queue space.
	 */
	NVIC_SetPriority(BOARD_UART_DEVICE, 0xe, 0);
	NVIC_ClearPendingIRQ(BOARD_UART_DEVICE);
	NVIC_EnableIRQ(BOARD_UART_DEVICE);

	dbg_register_device(&dbg_dev_uart);
}
