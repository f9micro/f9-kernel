/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <config.h>
#include <platform/irq.h>
#include <platform/fpb.h>
#include <platform/cortex_m.h>

#ifdef CONFIG_KPROBES

char fp_comp[FPB_MAX_COMP];

void fpb_init()
{
	int i;

	*FPB_CTRL = FPB_CTRL_KEY |FPB_CTRL_ENABLE ;

	// Enable DebugMonitor Exception Generation
	*DCB_DEMCR |= DCB_DEMCR_MON_EN;

	// clear status bit
	*SCB_HFSR = SCB_HFSR_DEBUGEVT;          // clear DBGEVT
	*SCB_DFSR = SCB_DFSR_BKPT;              // clear BKPT
	*SCB_DFSR = SCB_DFSR_HALTED;            // clear STEP

	for (i = 0; i < FPB_MAX_COMP; i++) {
		fp_comp[i] = 0;
	}
}

int fpb_avail_bkpt()
{
	int i;
	for (i = 0; i < FPB_MAX_COMP; i++) {
		if (fp_comp[i] == 0) {
			return i;
		}
	}
	return -1;
}

void fpb_setbkpt(int id, uint32_t addr)
{
	fp_comp[id] = 1;                        // comp allocated
	if (addr & 2) {
		*(FPB_COMP + id) = FPB_COMP_REPLACE_UPPER|addr|FPB_COMP_ENABLE;
	} else {
		*(FPB_COMP + id) = FPB_COMP_REPLACE_LOWER|addr|FPB_COMP_ENABLE;
	}
}

void fpb_unsetbkpt(int id)
{
	fp_comp[id] = 0;                        // comp free
	*(FPB_COMP + id) &= ~FPB_COMP_ENABLE;
}

void fpb_bkpt_enable(int id)
{
	*(FPB_COMP + id) |= FPB_COMP_ENABLE;            // enable comparator
}

void fpb_bkpt_disable(int id)
{
	*(FPB_COMP + id) &= ~FPB_COMP_ENABLE;           // disable comparator
}

void fpb_enable()
{
	// enable FPB unit
	*FPB_CTRL = FPB_CTRL_KEY | FPB_CTRL_ENABLE ;
}

void fpb_disable()
{
	// disable FPB unit
	*FPB_CTRL = FPB_CTRL_KEY | ~FPB_CTRL_ENABLE ;
}

#endif /* CONFIG_KPROBES */
