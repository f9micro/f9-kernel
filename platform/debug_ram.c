/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <lib/string.h>
#include <platform/debug_device.h>
#include <platform/debug_ram.h>

#ifdef CONFIG_DEBUG_DEV_RAM
typedef struct dbg_buf_t {
	uint32_t	idx;
	uint8_t		data[DBG_RAM_BUFSIZE];
} dbg_buf_t;

static dbg_buf_t dbg_buf;

static void dbg_ram_putchar(uint8_t chr)
{
	uint32_t idx;

	idx = dbg_buf.idx;
	dbg_buf.data[idx++] = chr;
	if (idx == DBG_RAM_BUFSIZE)
		idx = 0;

	dbg_buf.idx = idx;
}

void dbg_ram_init(void)
{
	dbg_dev_t dbg_dev_ram;

	/* Initialize buffer */
	memset(&dbg_buf, 0, sizeof(dbg_buf_t));

	/* Register debug ram virtual device */
	memset(&dbg_dev_ram, 0, sizeof(dbg_dev_t));
	dbg_dev_ram.dev_id = DBG_DEV_RAM;
	dbg_dev_ram.putchar = &dbg_ram_putchar;
	dbg_register_device(&dbg_dev_ram);
}
#endif /* CONFIG_DEBUG_DEV_RAM */
