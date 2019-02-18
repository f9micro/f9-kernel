/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if !defined(CONFIG_KPROBES)
#error __FILE__ " is the part of KProbes implementation."
#endif

#include <kprobes.h>
#include <error.h>
#include <platform/cortex_m.h>
#include <platform/hw_debug.h>
#include <platform/breakpoint.h>

void kprobe_arch_init()
{
	breakpoint_pool_init();
}

int kprobe_arch_add(struct kprobe *kp)
{
	struct kprobe *found = kplist_search(kp->addr);
	struct breakpoint *b;

	/*
	 * If there is no kprobe at this addr, give it a new bkpt,
	 * otherwise share the existing bkpt.
	 */

	if (found == NULL) {
		b = breakpoint_install((uint32_t) kp->addr);
		if (b != NULL) {
			kp->bkpt = b;
			enable_breakpoint(b);
		} else
			goto arch_add_error;
	} else {
		kp->bkpt = found->bkpt;
	}

	return 0;

arch_add_error:
	return -1;
}

int kprobe_arch_del(struct kprobe *kp)
{
	struct kprobe *found = kplist_search(kp->addr);

	/* Free bkpt when there is no kprobe at this addr */
	if (found == NULL)
		breakpoint_uninstall(kp->bkpt);
	return 0;
}

void arch_kprobe_handler(uint32_t *stack, uint32_t *kp_regs)
{
	/*
	 * For convenience currently we assume all cpu single-step is
	 * enabled/disabled by arch_kprobe_handler.
	 *
	 * To execute instruction at the probed address, we have to disable
	 * breakpoint before return from handler, and re-enable it in the
	 * next instruction.
	 */

	if ((*SCB_DFSR & SCB_DFSR_DWTTRAP)) {
		panic("DWT Watchpoint hit\n");
	} else if ((*SCB_DFSR & SCB_DFSR_BKPT)) {

		kprobe_prebreak(stack, kp_regs);

		/* Clear BKPT status bit */
		*SCB_DFSR  =  SCB_DFSR_BKPT;

		cpu_enable_single_step();
		kprobe_breakpoint_disable(stack);
	} else if (*SCB_DFSR & SCB_DFSR_HALTED) {
		kprobe_postbreak(stack, kp_regs);

		/* Clear HALTED status bit */
		*SCB_DFSR  =  SCB_DFSR_HALTED;

		cpu_disable_single_step();
		kprobe_breakpoint_enable(stack);
	} else {
		/*
		 * sometimes DWT generates faults
		 * without setting SCB_DFSR_DWTTRAP
		 */
	}
}
