/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef BREAKPOINT_H_
#define BREAKPOINT_H_

enum {
	BKPT_NONE,
	BKPT_HARD,
	BKPT_SOFT,
};

struct breakpoint{
	uint16_t type;
	union{
		uint16_t hard_breakpoint_id;
		uint16_t back_instr;
		uint16_t raw_data;
	};
	uint32_t addr;
	void (*enable)(struct breakpoint *b);
	void (*disable)(struct breakpoint *b);
	void (*release)(struct breakpoint *b);
};

void breakpoint_pool_init(void);

struct breakpoint *breakpoint_install(uint32_t addr);
void breakpoint_uninstall(struct breakpoint* b);

int get_breakpoint_id(struct breakpoint *b);

void enable_breakpoint(struct breakpoint *b);
void disable_breakpoint(struct breakpoint *b);

#define BKPT_MAX_NUM	10
#define breakpoint_type_by_addr(addr) ((addr) < 0x20000000 ? BKPT_HARD : BKPT_SOFT)
#endif
