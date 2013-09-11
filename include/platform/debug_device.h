/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DEBUG_DEVICE_H_
#define DEBUG_DEVICE_H_
#include <types.h>
#include <debug.h>

/*
 * Identify the device of debug IO port
 */
typedef enum {
	DBG_DEV_UART	= 0,
#ifdef CONFIG_DEBUG_DEV_RAM
	DBG_DEV_RAM,
#endif
	DBG_DEV_MAX
} dbg_dev_id_t;

/*
 * Object context of an debug IO device. Including handlers ... etc
 */
typedef struct {
	dbg_dev_id_t	dev_id;
	uint8_t 	(*getchar)(void);
	void 		(*putchar)(char);
	void 		(*start_panic)(void);
} dbg_dev_t;

/*
 * Register device IO port objects
 */
int32_t dbg_register_device(dbg_dev_t *device);

/*
 * Change current debug IO port
 */
int32_t dbg_change_device(dbg_dev_id_t dev_id);

/*
 * Do the preparing before entering panic. Previous characters in
 * debug queue must be flushed out first.
 */
void dbg_start_panic(void);

/*
 * Receive a character from debug port
 */
extern uint8_t dbg_getchar(void);

/*
 * Send a character to debug port
 */
extern void dbg_putchar(uint8_t chr);

/*
 * Initialization procedure for debug IO port
 */
void dbg_device_init(void);

#endif /* DEBUG_DEVICE_H_ */
