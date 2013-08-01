/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/irq.h>
#include <platform/hw_debug.h>
#include <platform/cortex_m.h>

#ifdef CONFIG_KPROBES

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

#define enter_frame() \
	uint32_t *stack;                                       \
	__asm__ __volatile__ ("mov %0, sp" : "=r" (stack) : ); \
	__asm__ __volatile__ ("push {lr}");                    \
	__asm__ __volatile__ ("push {r4-r11}");

#define leave_frame() \
	__asm__ __volatile__ ("pop {r4-r11}");                 \
	__asm__ __volatile__ ("pop {pc}");

void debugmon_handler() __NAKED;
void debugmon_handler()
{
	enter_frame();

	arch_kprobe_handler(stack);

	leave_frame();
}

#endif /* CONFIG_KPROBES */
