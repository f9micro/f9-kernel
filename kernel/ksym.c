/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <ksym.h>
#include <types.h>
#include <lib/stdlib.h>
#include <init_hook.h>
#include <platform/link.h>

static int __ksym_count;
static const ksym_t *__ksym_tbl;
static const char *__ksym_strings;

void ksym_init()
{
	void *ksym_addr = (void *) &symtab_start;

	if (*((int *) ksym_addr) == KSYM_MAGIC) {
		__ksym_count = *((int *) ksym_addr + 1);
		__ksym_tbl = (void *) ksym_addr + 8;
		__ksym_strings = (void *) __ksym_tbl +
		                 __ksym_count * sizeof(ksym_t);
	} else {
		static ksym_t _sym_tbl[] = { { 0, 0 } };
		static char _sym_strings [] = "No symbol\0";
		__ksym_count = 1;
		__ksym_tbl = _sym_tbl;
		__ksym_strings = _sym_strings;
	}
}

INIT_HOOK(ksym_init, INIT_LEVEL_KERNEL_EARLY);

int ksym_total()
{
	return __ksym_count;
}

static int cmp_key(const void *addr, const void *p1)
{
	ksym_t * sym = (ksym_t *)p1;
	if (sym == (__ksym_tbl + ksym_total() - 1)) {
		return 0;
	} else if ((addr >= sym->addr) && (addr < (sym + 1)->addr)) {
		return 0;
	} else {
		return addr - ((ksym_t *) sym)->addr;
	}
}

int ksym_lookup(void *addr)
{
	static int prev_index = 0;
	ksym_t *found;

	if (__ksym_tbl[prev_index].addr < addr) {
		found = (ksym_t *) bsearch(addr, __ksym_tbl + prev_index,
		                           __ksym_count - prev_index,
		                           sizeof(__ksym_tbl[0]), cmp_key);
	} else {
		found = (ksym_t *) bsearch(addr, __ksym_tbl, __ksym_count,
		                           sizeof(__ksym_tbl[0]), cmp_key);
	}

	if (found == NULL)
		return -1;
	return prev_index = (found - __ksym_tbl);
}

char *ksym_id2name(int symid)
{
	return (char *) &__ksym_strings[__ksym_tbl[symid].strid];
}

void *ksym_id2addr(int symid)
{
	return __ksym_tbl[symid].addr;
}
