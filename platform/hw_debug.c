/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if !defined(CONFIG_KPROBES)
#error __FILE__ " is the part of KProbes implementation."
#endif

#include <platform/irq.h>
#include <platform/hw_debug.h>
#include <platform/cortex_m.h>

extern void arch_kprobe_handler(uint32_t *stack);

char fp_comp[FPB_MAX_COMP];

void hw_debug_init()
{
	int i;

	/* Enable FPB breakpoint */
	*FPB_CTRL = FPB_CTRL_KEY |FPB_CTRL_ENABLE ;

	/* Enable DWT watchpoint & DebugMon exception */
	*DCB_DEMCR |= DCB_DEMCR_TRCENA | DCB_DEMCR_MON_EN;

	/* Clear status bit */
	*SCB_HFSR = SCB_HFSR_DEBUGEVT;
	*SCB_DFSR = SCB_DFSR_BKPT;
	*SCB_DFSR = SCB_DFSR_HALTED;
	*SCB_DFSR = SCB_DFSR_DWTTRAP;

	for (i = 0; i < FPB_MAX_COMP; i++) {
		fp_comp[i] = 0;
	}
}

int get_avail_bkpt()
{
	int i;
	for (i = 0; i < FPB_MAX_COMP; i++) {
		if (fp_comp[i] == 0) {
			return i;
		}
	}
	return -1;
}

int breakpoint_install(uint32_t addr)
{
	int id = get_avail_bkpt();

	if (id < 0) {
		return -1;
	}

	fp_comp[id] = 1; /* Mark comparator allocated */
	if (addr & 2) {
		*(FPB_COMP + id) = FPB_COMP_REPLACE_UPPER|addr|FPB_COMP_ENABLE;
	} else {
		*(FPB_COMP + id) = FPB_COMP_REPLACE_LOWER|addr|FPB_COMP_ENABLE;
	}
	return id;
}

void breakpoint_uninstall(int id)
{
	fp_comp[id] = 0; /* Mark comparator free */
	*(FPB_COMP + id) &= ~FPB_COMP_ENABLE;
}

void debugmon_handler() __NAKED;
void debugmon_handler()
{
	/* select interrupted stack */
	__asm__ __volatile__ ("and r0, lr, #4");
	__asm__ __volatile__ ("cmp r0, #0");
	__asm__ __volatile__ ("ite eq");
	__asm__ __volatile__ ("mrseq r0, msp");
	__asm__ __volatile__ ("mrsne r0, psp");

	/* save r4-r11 */
	__asm__ __volatile__ ("push {r4-r11}");
	__asm__ __volatile__ ("mov r1, sp");

	/*
	 * arch_kprobe_handler(uint32_t *stack, uint32_t *kp_regs)
	 * r0 = r0-r3,r12,lr,pc,psr
	 * r1 = r4-r11
	 */
	__asm__ __volatile__ ("push {lr}");
	__asm__ __volatile__ ("bl arch_kprobe_handler");
	__asm__ __volatile__ ("pop {lr}");

	/* override r4-r11 */
	__asm__ __volatile__ ("pop {r4-r11}");

	/* NOTE: No support stack modification for the time being */

	__asm__ __volatile__ ("bx lr");
}
