/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <types.h>
#include <errno.h>
#include <lib/stdio.h>
#include <init_hook.h>
#include <platform/debug_device.h>
#ifdef CONFIG_DEBUG_DEV_UART
#include <platform/debug_uart.h>
#endif
#ifdef CONFIG_DEBUG_DEV_RAM
#include <platform/debug_ram.h>
#endif

#ifdef DEBUG_DEVICE_EXIST
static dbg_dev_t dbg_dev[DBG_DEV_MAX];
static dbg_dev_t *cur_dev = &dbg_dev[0];

static uint8_t default_getchar(void)
{
	return (EOF);
}

static void default_putchar(uint8_t chr)
{
	chr = 0;
}

static void default_start_panic(void)
{
}

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
	(*cur_dev->start_panic)();
}

/*
 * Initialization procedure for debug IO port
 */
static void dbg_device_init(void)
{
	int i;

	for (i = 0; i < (DBG_DEV_MAX - 1); i++) {
		dbg_dev_t *pdev = &dbg_dev[i];
		pdev->dev_id = DBG_DEV_MAX;
		pdev->getchar = default_getchar;
		pdev->putchar = default_putchar;
		pdev->start_panic = default_start_panic;
	}

#ifdef CONFIG_DEBUG_DEV_UART
	dbg_uart_init();
#endif

#ifdef CONFIG_DEBUG_DEV_RAM
	dbg_ram_init();
#endif
}

/*
 * Register device IO port objects
 */
int32_t dbg_register_device(dbg_dev_t *device)
{
	dbg_dev_t *pdev;
	if (!device)
		return -EINVAL;

	if (device->dev_id >= DBG_DEV_MAX)
		return -EINVAL;

	pdev = &dbg_dev[device->dev_id];
	pdev->dev_id = device->dev_id;
	pdev->getchar = device->getchar;
	pdev->putchar = device->putchar;
	pdev->start_panic = device->start_panic;

	return 0;
}

int32_t dbg_change_device(dbg_dev_id_t dev_id)
{
	if (dev_id >= DBG_DEV_MAX)
		return -EINVAL;

	if (dbg_dev[dev_id].dev_id == DBG_DEV_MAX)
		return -ENXIO;

	cur_dev = &dbg_dev[dev_id];
	return 0;

}

#else /* DEBUG_DEVICE_EXIST */
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
#endif /* DEBUG_DEVICE_EXIST */

#ifdef CONFIG_DEBUG
extern dbg_layer_t dbg_layer;

void dbg_device_init_hook(void)
{
#ifdef DEBUG_DEVICE_EXIST
	dbg_device_init();
#endif
	dbg_layer = DL_KDB;
}
INIT_HOOK(dbg_device_init_hook, INIT_LEVEL_PLATFORM);
#endif

#ifdef CONFIG_STDIO_USE_DBGPORT
uint8_t __l4_getchar(void)
	__attribute__((weak, alias("dbg_getchar")));

void __l4_putchar(uint8_t chr)
	__attribute__((weak, alias("dbg_putchar")));
#endif
