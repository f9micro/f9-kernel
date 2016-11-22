/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if !defined(CONFIG_KPROBES)
#error __FILE__ " depends on CONFIG_KPROBES"
#endif

#include <ksym.h>
#include <sampling.h>
#include <types.h>
#include <thread.h>
#include <debug.h>
#include <lib/stdlib.h>
#include <kprobes.h>
#include <platform/cortex_m.h>

#ifdef CONFIG_KDB
static int sampling_handler(struct kprobe *kp, uint32_t *stack,
                            uint32_t *kp_regs)
{
	uint32_t *target_stack;

	/* examine KTIMER LR */
	if (stack[REG_LR] & 0x4)
		target_stack = PSP();
	else
		target_stack = stack + 8;

	sampled_pcpush((void *) target_stack[REG_PC]);
	return 0;
}

extern void ktimer_handler(void);
void kdb_show_sampling(void)
{
	int *hitcount, *symid_list;
	static int init = 0;
	static struct kprobe k;

	if (init == 0) {
		dbg_printf(DL_KDB, "Init sampling...\n");
		sampling_init();
		sampling_enable();
		init++;

		k.addr = ktimer_handler;
		k.pre_handler = sampling_handler;
		k.post_handler = NULL;
		kprobe_register(&k);
		return;
	}

	sampling_disable();
	sampling_stats(&hitcount, &symid_list);

	for (int i = 0; i < ksym_total(); i++) {
		int symid = symid_list[i];
		if (hitcount[symid] == 0)
			break;
		dbg_printf(DL_KDB, "%5d [ %24s ]\n", hitcount[symid],
		           ksym_id2name(symid));
	}

	sampling_enable();
}
#endif /* CONFIG_KDB */
