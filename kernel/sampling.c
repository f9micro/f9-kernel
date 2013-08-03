/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <sampling.h>
#include <types.h>
#include <thread.h>
#include <debug.h>
#include <lib/stdlib.h>

#if !defined(CONFIG_KPROBES)
#error "Sampling feature depends on CONFIG_KPROBES"
#endif
#include <kprobes.h>
#include <platform/cortex_m.h>

extern uint32_t end_of_MFlash;

/* stack overflow */
static int sym_hit[MAX_KSYM];
static int sym_tophit[MAX_KSYM];

static int pre_handler(struct kprobe *kp, uint32_t *stack, uint32_t *kp_regs)
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

static int cmp_symhit(const void *p1, const void *p2)
{
	int *symid1 = (int *) p1;
	int *symid2 = (int *) p2;
	return sym_hit[*symid2] - sym_hit[*symid1];
}

static void sampling_stat()
{
	int i,symid;
	void **addr;

	sampled_disable();
	sampled_prepare();

	for (i = 0; i < MAX_KSYM; i++) {
		sym_hit[i] = 0;
		sym_tophit[i] = i;
	}

	/* symbol processing */
	for_each_sampled(addr, i) {
		symid = ksym_lookup(*addr);
		if (symid < 0)
			continue;
		sym_hit[symid]++;
	}

	sort(sym_tophit, ksym_total(), sizeof(sym_tophit[0]), cmp_symhit);

	for (i = 0; i < ksym_total(); i++) {
		int symid;
		symid = sym_tophit[i];
		if (sym_hit[symid] == 0)
			break;
		dbg_printf(DL_KDB, "%5d [ %24s ]\n", sym_hit[symid], ksym_name(symid));
	}
	sampled_enable();

}

#ifdef CONFIG_KDB
extern void ktimer_handler();
void kdb_show_sampling()
{
	static int init = 0;
	if (init == 0) {
		int magic, sym_count;
		char *sym_strings;
		ksym *sym_tbl;
		static struct kprobe k;

		dbg_printf(DL_KDB, "Init sampling...\n");
		magic = *((int *) &end_of_MFlash);
		sym_count = *((int *) &end_of_MFlash + 1);
		sym_tbl = (void *) &end_of_MFlash + 4 + sizeof(sym_count);
		sym_strings = (void *) sym_tbl + sym_count * sizeof(ksym);

		ksym_init(magic, sym_count, sym_tbl, sym_strings);
		sampling_init();
		init++;

		k.addr = ktimer_handler;
		k.pre_handler = pre_handler;
		k.post_handler = NULL;
		kprobe_register(&k);
		return;
	}
	sampling_stat();
}
#endif	/* ! CONFIG_KDB */
