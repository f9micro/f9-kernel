/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <kprobes.h>
#include <error.h>
#include <platform/cortex_m.h>
#include <platform/hw_debug.h>

#ifdef CONFIG_KPROBES

void kprobe_arch_init()
{
	hw_debug_init();
}

int kprobe_arch_add(struct kprobe *kp)
{
	int id;
	struct kprobe *found = kplist_search(kp->addr);

	/*
	 * If there is no kprobe at this addr, give it a new bkpt,
	 * otherwise share the existing bkpt.
	 */

	if (found == NULL) {
		id = breakpoint_install((uint32_t) kp->addr);
		if (id >= 0) {
			kp->bkptid = id;
		} else {
			goto arch_add_error;
		}
	} else {
		kp->bkptid = found->bkptid;
	}

	return 0;

arch_add_error:
	return -1;
}

int kprobe_arch_del(struct kprobe *kp)
{
	struct kprobe *found = kplist_search(kp->addr);

	/* Free bkpt when there is no kprobe at this addr */
	if (found == NULL) {
		breakpoint_uninstall(kp->bkptid);
	}
	return 0;
}

void arch_kprobe_handler(uint32_t *stack)
{
	static void *addr;

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

		addr = (void *) stack[REG_PC];

		kprobe_prebreak(addr);

		/* Clear BKPT status bit */
		*SCB_DFSR  =  SCB_DFSR_BKPT;

		disable_hw_breakpoint();
		cpu_enable_single_step();

	} else if (*SCB_DFSR & SCB_DFSR_HALTED) {

		kprobe_postbreak(addr);

		/* Clear HALTED status bit */
		*SCB_DFSR  =  SCB_DFSR_HALTED;

		cpu_disable_single_step();
		enable_hw_breakpoint();

	} else {
		panic("Kernel panic: Debug fault. Restarting\n");
	}
}

#endif /* CONFIG_KPROBES */
