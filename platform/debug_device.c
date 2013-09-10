/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <types.h>
#include <errno.h>
#include <lib/stdio.h>
//#include <lib/queue.h>
#include <platform/debug_device.h>
#ifdef CONFIG_DEBUG_DEV_UART
#include <platform/debug_uart.h>
#endif

#ifdef CONFIG_DEBUG
static dbg_dev_t dbg_dev[DBG_DEV_MAX];
static dbg_dev_t *cur_dev = &dbg_dev[0];

uint8_t dbg_getchar(void)
{
	/* FIXME: getchar NULL error checking */
	return ((*cur_dev->getchar)());
}

void dbg_putchar(uint8_t chr)
{
	/* FIXME: putchar NULL error checking */
	(*cur_dev->putchar)(chr);
}

void dbg_start_panic(void)
{
	if (!cur_dev || !cur_dev->start_panic)
		return;
	cur_dev->start_panic();
}

void dbg_device_init(void)
{
#ifdef CONFIG_DEBUG_DEV_UART
	dbg_uart_init();
#endif
}

int32_t dbg_register_device(dbg_dev_t *device)
{
	dbg_dev_t *pdev;
	if (!device)
		return -EINVAL;

	if (!device->getchar ||
	    !device->putchar ||
	     device->dev_id >= DBG_DEV_MAX)
		return -EINVAL;

	pdev = &dbg_dev[device->dev_id];
	pdev->getchar = device->getchar;
	pdev->putchar = device->putchar;
	pdev->start_panic = device->start_panic;

	return 0;
}

/* FIXME: function to change device is required */
#else /* CONFIG_DEBUG */
void dbg_start_panic(void)
{
}

uint8_t dbg_getchar(void)
{
	return 0;
}

void dbg_putchar(uint8_t chr)
{
}
#endif /* CONFIG_DEBUG */

