/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DEBUG_DEVICE_H_
#define DEBUG_DEVICE_H_
#include <types.h>
#include <debug.h>

typedef enum {
	DBG_DEV_UART	= 0,
	DBG_DEV_MAX
} dbg_dev_id_t;

typedef struct {
	dbg_dev_id_t	dev_id;
	uint8_t 	(*getchar)(void);
	void 		(*putchar)(char);
	void 		(*start_panic)(void);
} dbg_dev_t;

int32_t dbg_register_device(dbg_dev_t *device);
void dbg_start_panic(void);
void dbg_device_init(void);
extern uint8_t dbg_getchar(void);
extern void dbg_putchar(uint8_t chr);
#endif /* DEBUG_DEVICE_H_ */
