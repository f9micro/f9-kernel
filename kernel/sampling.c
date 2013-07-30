/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <sampling.h>
#include <types.h>
#include <thread.h>
#include <debug.h>
#include <lib/stdlib.h>
#include <kprobes.h>

extern uint32_t end_of_MFlash;

// stack overflow
int sym_hit[MAX_KSYM];
int sym_tophit[MAX_KSYM];

int pre_handler()
{
	sampled_pcpush((void *)((uint32_t *) thread_current()->ctx.sp)[REG_PC]);
	return 0;
}

void sampling_stat()
{
	int i,symid;
	void **addr;

	sampled_disable();
	sampled_prepare();

	for (i = 0; i < MAX_KSYM; i++) {
		sym_hit[i] = 0;
		sym_tophit[i] = i;
	}

	// symbol processing
	for_each_sampled(addr, i) {
		symid = ksym_lookup(*addr);
		if (symid < 0) {
			continue;
		}
		sym_hit[symid]++;
	}

	int cmp_symhit(const void *p1, const void *p2)
	{
		int *symid1 = (int *)p1;
		int *symid2 = (int *)p2;
		return sym_hit[*symid2] - sym_hit[*symid1];
	}

	sort(sym_tophit, ksym_total(), sizeof(sym_tophit[0]), cmp_symhit);

	for (i = 0; i < ksym_total(); i++) {
		int symid;
		symid = sym_tophit[i];
		if (sym_hit[symid] == 0) {
			break;
		}
		dbg_printf(DL_KDB, "%d [%s]\n", sym_hit[symid], ksym_name(symid));
	}
	sampled_enable();

}

extern void __ktimer_handler();
void kdb_show_sampling()
{
	static int init = 0;
	if (init == 0) {
		int magic, sym_count;
		char *sym_strings;
		ksym *sym_tbl;
		static struct kprobe k;

		dbg_printf(DL_KDB, "Init sampling...\n");
		magic = *((int *)&end_of_MFlash);
		sym_count = *((int *)&end_of_MFlash + 1);
		sym_tbl = (void*)&end_of_MFlash + 4 + sizeof(sym_count);
		sym_strings = (void*)sym_tbl + sym_count * sizeof(ksym);

		ksym_init(magic, sym_count, sym_tbl, sym_strings);
		sampling_init();
		init++;

		k.addr = __ktimer_handler;
		k.pre_handler = pre_handler;
		k.post_handler = NULL;
		kprobe_register(&k);
		return;
	}
	sampling_stat();
}
