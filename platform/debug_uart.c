/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/debug_uart.h>
#include <platform/irq.h>
#include <lib/fifo.h>
#include <softirq.h>

/* board speficic UART definitions */
#include "board.h"

static struct dbg_uart_t dbg_uart;
static uint8_t dbg_uart_tx_buffer[SEND_BUFSIZE];
static uint8_t dbg_uart_rx_buffer[RECV_BUFSIZE];

enum { DBG_ASYNC, DBG_PANIC } dbg_state;

static void dbg_uart_recv();
static void dbg_uart_send(int avail);

void __uart_irq_handler()
{
	if (usart_interrupt_status(&console_uart, USART_IT_TXE)) {
		/* USART TX */
		dbg_uart_send(1);
	}
	else if (usart_interrupt_status(&console_uart, USART_IT_RXNE)) {
		/* USART RX */
		dbg_uart_recv();
	}
}

IRQ_HANDLER(BOARD_UART_HANDLER, __uart_irq_handler);

void dbg_uart_init()
{
	usart_init(&console_uart);	

	dbg_uart.ready = 1;
	fifo_init(&(dbg_uart.tx), dbg_uart_tx_buffer, SEND_BUFSIZE);
	fifo_init(&(dbg_uart.rx), dbg_uart_rx_buffer, RECV_BUFSIZE);

	dbg_state = DBG_ASYNC;
	usart_config_interrupt(&console_uart, USART_IT_RXNE, 1);

	NVIC_SetPriority(BOARD_UART_DEVICE, 2, 0);
	NVIC_ClearPendingIRQ(BOARD_UART_DEVICE);
	NVIC_EnableIRQ(BOARD_UART_DEVICE);
}


static void dbg_uart_recv()
{
	uint8_t chr = usart_getc(&console_uart);

	/* Put sequence on queue */
	fifo_push(&(dbg_uart.rx), chr);

#ifdef CONFIG_KDB
	softirq_schedule(KDB_SOFTIRQ);
#endif
}

static void dbg_uart_send(int avail)
{
	uint8_t chr;

	if (avail) {
		if (fifo_state(&(dbg_uart.tx)) != FIFO_EMPTY) {
			fifo_pop(&(dbg_uart.tx), &chr);
			usart_putc(&console_uart, chr);
			dbg_uart.ready = 0;
			usart_config_interrupt(&console_uart, USART_IT_TXE, 1);
		}
		else {
			dbg_uart.ready = 1;
			usart_config_interrupt(&console_uart, USART_IT_TXE, 0);
		}
	}
}

uint8_t dbg_getchar()
{
	uint8_t chr = 0;

	if (fifo_pop(&(dbg_uart.rx), &chr) == FIFO_EMPTY)
		return 0;
	return chr;

}
uint8_t __l4_getchar()
	__attribute__ ((weak, alias ("dbg_getchar")));

static void dbg_async_putchar(char chr);
static void dbg_sync_putchar(char chr);

void dbg_putchar(char chr)
{
	/* During panic, we cannot use async dbg uart, so switch to
	 * synchronious mode
	 */
	if (dbg_state != DBG_PANIC)
		dbg_async_putchar(chr);
	else
		dbg_sync_putchar(chr);
}
void __l4_putchar(char chr)
	__attribute__ ((weak, alias ("dbg_putchar")));

static void dbg_async_putchar(char chr)
{
	/* If UART is busy, try to put chr into queue until slot is freed,
	 * else write directly into UART
	 */
	if (!dbg_uart.ready) {
		while (fifo_push(&(dbg_uart.tx), chr) != FIFO_OK)
			/* wait */ ;
	}
	else {
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

void dbg_start_panic()
{
	unsigned char chr;

	/* In panic condition, we can be in interrupt context or
	 * not, so will write sequence synchronously */

	/* Flush remaining sequence  in async buffer */
	while (fifo_pop(&(dbg_uart.tx), &chr) != FIFO_EMPTY) {
		dbg_sync_putchar(chr);
	}

	dbg_state = DBG_PANIC;
}
