/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if !defined(CONFIG_KPROBES)
#error __FILE__ " is the part of KProbes implementation."
#endif

#include <platform/hw_debug.h>
#include <platform/cortex_m.h>
#include <platform/breakpoint.h>
#include <platform/breakpoint-hard.h>
#include <types.h>

#define HW_BKPT_NULL_ID FPB_MAX_COMP
#define IS_UPPER_HALFWORLD(addr)	(addr & 0x2)

static int hard_breakpoints[FPB_MAX_COMP];

static void hard_breakpoint_enable(struct breakpoint *b);
static void hard_breakpoint_disable(struct breakpoint *b);
static void hard_breakpoint_release(struct breakpoint *b);

static int get_avail_hard_breakpoint(void)
{
	int i;
	for (i = 0 ; i < FPB_MAX_COMP ; i++) {
		if (hard_breakpoints[i] == -1)
			return i;
	}
	return -1;
}

void hard_breakpoint_pool_init(void)
{
	int i;

	/* Enable FPB breakpoint */
	*FPB_CTRL = FPB_CTRL_KEY | FPB_CTRL_ENABLE ;

	/* Enable DWT watchpoint & DebugMon exception */
	*DCB_DEMCR |= DCB_DEMCR_TRCENA | DCB_DEMCR_MON_EN;

	/* Clear status bit */
	*SCB_HFSR = SCB_HFSR_DEBUGEVT;
	*SCB_DFSR = SCB_DFSR_BKPT;
	*SCB_DFSR = SCB_DFSR_HALTED;
	*SCB_DFSR = SCB_DFSR_DWTTRAP;

	for (i = 0; i < FPB_MAX_COMP; i++) {
		hard_breakpoints[i] = -1;
		*(FPB_COMP + i) = 0;		/* Reset each FPB_COMP register */
	}

	enable_all_hard_breakpoints();
}

struct breakpoint *hard_breakpoint_config(uint32_t addr, struct breakpoint *b)
{
	if (breakpoint_type_by_addr(addr) == BKPT_HARD) {
		int _hard_breakpoint_id = get_avail_hard_breakpoint();
		int breakpoint_id = get_breakpoint_id(b);

		if (breakpoint_id >= 0 && _hard_breakpoint_id >= 0) {
			hard_breakpoints[_hard_breakpoint_id] = breakpoint_id;
			b->type = BKPT_HARD;
			b->addr = addr;
			b->hard_breakpoint_id = _hard_breakpoint_id;
			b->enable = hard_breakpoint_enable;
			b->disable = hard_breakpoint_disable;
			b->release = hard_breakpoint_release;
			return b;
		}
	}

	return NULL;
}

static void hard_breakpoint_enable(struct breakpoint *b)
{
	uint32_t addr = b->addr;

	if (IS_UPPER_HALFWORLD(addr)) {
		*(FPB_COMP + b->hard_breakpoint_id) =
			FPB_COMP_REPLACE_UPPER |
		        (addr & FPB_COMP_ADDR_MASK) | FPB_COMP_ENABLE;
	} else {
		*(FPB_COMP + b->hard_breakpoint_id) =
			FPB_COMP_REPLACE_LOWER |
			(addr & FPB_COMP_ADDR_MASK) | FPB_COMP_ENABLE;
	}
}

static void hard_breakpoint_disable(struct breakpoint *b)
{
	*(FPB_COMP + b->hard_breakpoint_id) &= ~FPB_COMP_ENABLE;
}

static void hard_breakpoint_release(struct breakpoint *b)
{
	*(FPB_COMP + b->hard_breakpoint_id) &= ~FPB_COMP_ENABLE;
	hard_breakpoints[b->hard_breakpoint_id] = -1;
	b->type = BKPT_NONE;
}
