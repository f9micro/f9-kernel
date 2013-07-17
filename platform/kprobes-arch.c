/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <kprobes.h>
#include <error.h>
#include <platform/cortex_m.h>
#include <platform/fpb.h>

#ifdef CONFIG_KPROBES

void kprobe_arch_init()
{
	fpb_init();
}

int kprobe_arch_add(struct kprobe *kp)
{
	int id;
	struct kprobe *found = kplist_search(kp->addr);
	if (found == NULL) {
		// the first kprobe at this address
		id = fpb_avail_bkpt();
		if (id >= 0 ) {
			kp->bkptid = id;
			fpb_setbkpt(id, (uint32_t) kp->addr);
		} else {
			goto arch_add_error;   // HW bkpt is not available
		}
	} else {
		// kprobe with the same address share the same HW bkpt
		kp->bkptid = found->bkptid;
	}

	return 0;

arch_add_error:
	return -1;
}

int kprobe_arch_del(struct kprobe *kp)
{
	struct kprobe *found = kplist_search(kp->addr);
	if (found == NULL) {
		// the last kprobe at this address
		fpb_unsetbkpt(kp->bkptid);
	}
	return 0;
}

void arch_kprobe_handler(uint32_t *stack)
{
	static void *addr;

	if ((*SCB_DFSR & SCB_DFSR_BKPT)) {

		addr = (void *) stack[REG_PC];

		kprobe_prebreak(addr);
		*SCB_DFSR  =  SCB_DFSR_BKPT;     // clear BKPT status bit

		fpb_disable();
		cpu_step_enable();

	} else if (*SCB_DFSR & SCB_DFSR_HALTED) {
		// assume no thread-switch while single-step

		kprobe_postbreak(addr);
		*SCB_DFSR  =  SCB_DFSR_HALTED;  // clear HALTED status bit

		cpu_step_disable();
		fpb_enable();

	} else {
		panic("Kernel panic: Debug fault. Restarting\n");
	}
}

#endif /* CONFIG_KPROBES */
