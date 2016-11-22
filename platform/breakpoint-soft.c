/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if !defined(CONFIG_KPROBES)
#error __FILE__ " is the part of KProbes implementation."
#endif

#include <platform/cortex_m.h>
#include <platform/breakpoint.h>
#include <platform/breakpoint-soft.h>
#include <types.h>

static inline void insert_bkpt(struct breakpoint *b)
{
	uint16_t breakpoint_instr = 0xbe11;	/* ARM BKPT instruction */

	*(volatile uint16_t *) b->addr = breakpoint_instr;
}

static inline void delete_bkpt(struct breakpoint *b)
{
	*(volatile uint16_t *) b->addr = b->back_instr;
}

static void soft_breakpoint_enable(struct breakpoint *b);
static void soft_breakpoint_disable(struct breakpoint *b);
static void soft_breakpoint_release(struct breakpoint *b);

void soft_breakpoint_pool_init(void)
{
	/* Nothing to do. */
	return;
}

struct breakpoint *soft_breakpoint_config(uint32_t addr, struct breakpoint *b)
{
	if (breakpoint_type_by_addr(addr) == BKPT_SOFT) {
		b->type = BKPT_SOFT;
		b->addr = addr;
		b->back_instr = *(uint16_t *) addr;
		b->enable = soft_breakpoint_enable;
		b->disable = soft_breakpoint_disable;
		b->release = soft_breakpoint_release;

		return b;
	}
	return NULL;
}

static void soft_breakpoint_enable(struct breakpoint *b)
{
	insert_bkpt(b);
}

static void soft_breakpoint_disable(struct breakpoint *b)
{
	delete_bkpt(b);
}

static void soft_breakpoint_release(struct breakpoint *b)
{
	b->type = BKPT_NONE;
}
