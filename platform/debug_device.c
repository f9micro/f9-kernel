/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <types.h>
#include <errno.h>
#include <lib/stdio.h>
#include <platform/debug_device.h>
#ifdef CONFIG_DEBUG_DEV_UART
#include <platform/debug_uart.h>
#endif
#include <init_hook.h>

#ifdef CONFIG_DEBUG
static dbg_dev_t dbg_dev[DBG_DEV_MAX];
static dbg_dev_t *cur_dev = &dbg_dev[0];

/*
 * Receive a character from debug port
 */
uint8_t dbg_getchar(void)
{
	return ((*cur_dev->getchar)());
}

/*
 * Send a character to debug port
 */
void dbg_putchar(uint8_t chr)
{
	(*cur_dev->putchar)(chr);
}

/*
 * Do the preparing before entering panic. Previous characters in
 * debug queue must be flushed out first.
 */
void dbg_start_panic(void)
{
	if (!cur_dev || !cur_dev->start_panic)
		return;
	cur_dev->start_panic();
}

/*
 * Initialization procedure for debug IO port
 */
void dbg_device_init(void)
{
	int i;

	for (i = 0; i < DBG_DEV_MAX; i++) {
		dbg_dev_t *pdev = &dbg_dev[i];
		pdev->dev_id = DBG_DEV_MAX;
		pdev->getchar = NULL;
		pdev->putchar = NULL;
		pdev->start_panic = NULL;
	}

#ifdef CONFIG_DEBUG_DEV_UART
	dbg_uart_init();
#endif
}

#ifdef CONFIG_DEBUG
extern dbg_layer_t dbg_layer;

void dbg_device_init_hook(void)
{
	dbg_device_init();
	dbg_layer = DL_KDB;
}
INIT_HOOK(dbg_device_init_hook, dbg_device_init_hook, INIT_LEVEL_PLATFORM);
#endif

/*
 * Register device IO port objects
 */
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
uint8_t dbg_getchar(void)
{
	return (EOF);
}

void dbg_putchar(uint8_t chr)
{
	chr = 0;
}

void dbg_start_panic(void)
{
}
#endif /* CONFIG_DEBUG */

#ifdef CONFIG_STDIO_USE_DBGPORT
uint8_t __l4_getchar(void)
        __attribute__ ((weak, alias ("dbg_getchar")));

void __l4_putchar(uint8_t chr)
        __attribute__ ((weak, alias ("dbg_putchar")));
#endif
