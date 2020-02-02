/* Copyright (c) 2002, 2003, 2007, 2010 Karlsruhe University.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/types.h>
#include <l4/ipc.h>
#include <l4io.h>
#include <platform/cortex_m.h>

#include "arch.h"
#include "config.h"
#include "l4test.h"
#include "assert.h"

__USER_TEXT
static void page_touch(void)
{
	volatile L4_Word_t *addr = (L4_Word_t *) get_new_page();
	int n = 0;
	int max = 1000;

	printf("test: Welcome to memtest!\n");

	while (max--) {
		// *addr = 0x37ULL;
		addr += (PAGE_SIZE / sizeof(*addr));
		n++;
	}

	print_result("Page touch", true);
}

__USER_TEXT
void all_mem_tests(void)
{
	page_touch();
}
