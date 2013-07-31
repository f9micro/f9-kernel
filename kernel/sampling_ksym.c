/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <types.h>
#include <debug.h>
#include <sampling.h>
#include <lib/stdlib.h>

void *sampled_pc[MAX_SAMPLING_COUNT];
static int sampled_count;
static int sampled_enabled;

static int __ksym_count;
static const ksym *__ksym_tbl;
static const char *__ksym_strings;

void sampling_init()
{
	int i ;
	if (__ksym_count > MAX_KSYM) {
		dbg_printf(DL_KDB, "ksym %d > MAX_KSYM\n", __ksym_count);
		return;
	}
	for (i = 0; i < MAX_SAMPLING_COUNT; i++) {
		sampled_pc[i] = 0;
	}
	sampled_count = 0;
	sampled_enable();
}

void sampled_pcpush(void *pc)
{
	if (sampled_enabled == 0)
		return;
	if (sampled_count == MAX_SAMPLING_COUNT)
		sampled_count = 0;
	sampled_pc[sampled_count++] = pc;
}

static int cmp_addr(const void *p1, const void *p2)
{
	return *(int *) p1 - *(int *) p2;
}

void sampled_prepare()
{
	sort(sampled_pc, MAX_SAMPLING_COUNT, sizeof(sampled_pc[0]), cmp_addr);
}

void sampled_enable()
{
	sampled_enabled = 1;
}

void sampled_disable()
{
	sampled_enabled = 0;
}

void ksym_init(int magic, int count, ksym *tbl, char *strings)
{
	if (magic == SAMPL_MAGIC) {
		__ksym_count = count;
		__ksym_tbl = tbl;
		__ksym_strings = strings;
	}
	else {
		static ksym _sym_tbl[] = { { 0,0 } };
		static char _sym_strings [] = "No symbol\0";
		__ksym_count = 1;
		__ksym_tbl = _sym_tbl;
		__ksym_strings = _sym_strings;
	}
}

int ksym_total()
{
	return __ksym_count;
}

static int cmp_key(const void *addr, const void *p1)
{
	ksym * sym = (ksym *)p1;
	if (sym == (__ksym_tbl+__ksym_count-1)) {
		return 0;
	}
	else if ((addr >= sym->addr) && (addr < (sym+1)->addr)) {
		return 0;
	}
	else {
		return addr - ((ksym *) sym)->addr;
	}
}

int ksym_lookup(void *addr)
{
	static int prev_index = 0;
	ksym *found;

	if (__ksym_tbl[prev_index].addr < addr) {
		found = (ksym *) bsearch(addr, __ksym_tbl + prev_index,
				__ksym_count - prev_index,
				sizeof(__ksym_tbl[0]), cmp_key);
	} else {
		found = (ksym *) bsearch(addr, __ksym_tbl, __ksym_count,
				sizeof(__ksym_tbl[0]), cmp_key);
	}
        if (found == NULL) {
		return -1;
	}
	else {
		return prev_index = (found - __ksym_tbl);
	}
}

char *ksym_name(int symid)
{
	return (char *)&__ksym_strings[__ksym_tbl[symid].strid];
}

void *ksym_addr(int symid)
{
	return __ksym_tbl[symid].addr;
}
