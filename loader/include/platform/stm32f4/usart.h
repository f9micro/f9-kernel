/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_STM32F4_USART_H_
#define PLATFORM_STM32F4_USART_H_

#include <platform/stm32f4/registers.h>
#include <platform/stm32f4/gpio.h>

struct usart_dev {
	uint32_t u_num;
	uint32_t baud;
	uint32_t base;
	volatile uint32_t *rcc_apbenr;
	uint32_t rcc_reset;
	struct gpio_cfg tx;
	struct gpio_cfg rx;
};

#define USART_NUM 6

/* status bit */
#define USART_CTS	((uint16_t) (0x1 << 9))
#define USART_LBD	((uint16_t) (0x1 << 8))
#define USART_TXE	((uint16_t) (0x1 << 7))
#define USART_TC	((uint16_t) (0x1 << 6))
#define USART_RXNE	((uint16_t) (0x1 << 5))
#define USART_IDLE	((uint16_t) (0x1 << 4))
#define USART_ORE	((uint16_t) (0x1 << 3))
#define USART_NF	((uint16_t) (0x1 << 2))
#define USART_FE	((uint16_t) (0x1 << 1))
#define USART_PE	((uint16_t) (0x1 << 0))


#define USART_IT_CR1	(0xc << 8)
#define USART_IT_CR2	(0x10 << 8)
#define USART_IT_CR3	(0x14 << 8)

#define USART_IT_PE	((uint16_t) USART_IT_CR1 | 0x8)
#define USART_IT_TXE	((uint16_t) USART_IT_CR1 | 0x7)
#define USART_IT_TC	((uint16_t) USART_IT_CR1 | 0x6)
#define USART_IT_RXNE	((uint16_t) USART_IT_CR1 | 0x5)
#define USART_IT_IDLE	((uint16_t) USART_IT_CR1 | 0x4)
#define USART_IT_LBD	((uint16_t) USART_IT_CR2 | 0x6)
#define USART_IT_CTS	((uint16_t) USART_IT_CR3 | 0xa)
#define USART_IT_ERR	((uint16_t) USART_IT_CR3 | 0x0)

#define USART_IT_ENB_REG_OFFSET(USART_IT)	((USART_IT >> 8) & 0xff)
#define USART_IT_POSITION(USART_IT)		(USART_IT & 0xff)

void usart_init(struct usart_dev *usart);
void usart_putc(struct usart_dev *usart, uint8_t c);
uint8_t usart_getc(struct usart_dev *usart);
int usart_status(struct usart_dev *usart, uint16_t st);
int usart_interrupt_status(struct usart_dev *usart, uint16_t it);
void usart_config_interrupt(struct usart_dev *usart, uint16_t it, uint8_t state);

#endif /* PLATFORM_STM32F4_USART_H_ */
