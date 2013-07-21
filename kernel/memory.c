/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory.h>
#include <error.h>
#include <config.h>
#include <debug.h>
#include <thread.h>
#include <lib/ktable.h>
#include <fpage_impl.h>

#include <kip.h>

/**
 * @file    memory.c
 * @brief   Memory management subsystem.
 *
 * Unlike traditional L4 kernels, built for "large systems", we focus on
 * small MCU with energy efficiency.  Therefore,
 *   - We don't have virtual memory and pages
 *   - RAM is small, but physical address space is relatively large (32-bit),
 *     which includes devices, flash, bit-band regions
 *   - We have memory protection unit with only 8 regions
 *
 * Memory management is split into three conceptions:
 *   - Memory pool (mempool_t), which represent area of PAS with specific
 *     attributes (hardcoded in memmap table),
 *   - Flexible page (fpage_t) - unlike traditional fpages in L4, they
 *     represent MPU region instead
 *   - Address space (as_t) - sorted list of fpages bound to specific thread(s)
 *
 * MPU in Cortex-M supports regions of 2^n size only, so if we want to create
 * page of 96 bytes for example, we should split it into smaller ones, and
 * create fpage chain consisting of 32 byte and 64 byte fpages.
 * That is exactly the reason why the implementation looks complicated.
 *
 * Obvious way is to use regions of standard size (e.g. 128 bytes), but it is
 * very wasteful in terms of memory faults (we have only 8 regions for MPU,
 * so for large ASes, we will often receive memmanage faults), and fpage table.
 */

/**
 * Memory map of MPU.
 * Translated into memdesc array in KIP by memory_init
 */
static mempool_t memmap[] = {
	DECLARE_MEMPOOL_2("KTEXT", kernel_text,
		MP_KR | MP_KX | MP_NO_FPAGE, MPT_KERNEL_TEXT),
	DECLARE_MEMPOOL_2("UTEXT", user_text,
		MP_UR | MP_UX | MP_MEMPOOL | MP_MAP_ALWAYS, MPT_USER_TEXT),
	DECLARE_MEMPOOL_2("KIP", kip,
		MP_KR | MP_KW | MP_UR | MP_SRAM, MPT_KERNEL_DATA),
	DECLARE_MEMPOOL  ("KDATA", &kip_end, &kernel_data_end,
		MP_KR | MP_KW | MP_NO_FPAGE, MPT_KERNEL_DATA),
	DECLARE_MEMPOOL_2("KBSS",  kernel_bss,
		MP_KR | MP_KW | MP_NO_FPAGE, MPT_KERNEL_DATA),
	DECLARE_MEMPOOL_2("UDATA", user_data,
		MP_UR | MP_UW | MP_MEMPOOL | MP_MAP_ALWAYS, MPT_USER_DATA),
	DECLARE_MEMPOOL_2("UBSS",  user_bss,
		MP_UR | MP_UW | MP_MEMPOOL  | MP_MAP_ALWAYS, MPT_USER_DATA),
	DECLARE_MEMPOOL  ("MEM0",  &user_bss_end, 0x2001c000,
		MP_UR | MP_UW | MP_SRAM, MPT_AVAILABLE),
#ifdef CONFIG_BITMAP_BITBAND
	DECLARE_MEMPOOL  ("KBITMAP",  &bitmap_bitband_start, &bitmap_bitband_end,
		MP_KR | MP_KW | MP_NO_FPAGE, MPT_KERNEL_DATA),
#else
	DECLARE_MEMPOOL  ("KBITMAP",  &bitmap_start, &bitmap_end,
		MP_KR | MP_KW | MP_NO_FPAGE, MPT_KERNEL_DATA),
#endif
	DECLARE_MEMPOOL  ("MEM1",   &kernel_ahb_end, 0x10010000,
		MP_UR | MP_UW | MP_AHB_RAM, MPT_AVAILABLE),
	DECLARE_MEMPOOL  ("APB1DEV", 0x40000000, 0x40007800,
		MP_UR | MP_UW | MP_DEVICES, MPT_DEVICES),
	DECLARE_MEMPOOL  ("APB2_1DEV", 0x40010000, 0x40013400,
		MP_UR | MP_UW | MP_DEVICES, MPT_DEVICES),
	DECLARE_MEMPOOL  ("APB2_2DEV", 0x40014000, 0x40014c00,
		MP_UR | MP_UW | MP_DEVICES, MPT_DEVICES),
	DECLARE_MEMPOOL  ("AHB1_1DEV", 0x40020000, 0x40022400,
		MP_UR | MP_UW | MP_DEVICES, MPT_DEVICES),
	DECLARE_MEMPOOL  ("AHB1_2DEV", 0x40023c00, 0x40040000,
		MP_UR | MP_UW | MP_DEVICES, MPT_DEVICES),
	DECLARE_MEMPOOL  ("AHB2DEV", 0x50000000, 0x50061000,
		MP_UR | MP_UW | MP_DEVICES, MPT_DEVICES),
	DECLARE_MEMPOOL  ("AHB3DEV", 0x60000000, 0xA0001000,
		MP_UR | MP_UW | MP_DEVICES, MPT_DEVICES),
};

DECLARE_KTABLE(as_t, as_table, CONFIG_MAX_ADRESS_SPACES);

extern kip_mem_desc_t *mem_desc;
extern char *kip_extra;

/* Some helper functions */

static memptr_t addr_align(memptr_t addr, size_t size)
{
	if (addr & (size - 1))
		return (addr & ~(size - 1)) + size;
	return (addr & ~(size - 1));
}

memptr_t mempool_align(int mpid, memptr_t addr)
{
	switch (memmap[mpid].flags & MP_FPAGE_MASK) {
	case MP_MEMPOOL:
	case MP_SRAM:
	case MP_AHB_RAM:
		return addr_align(addr, CONFIG_SMALLEST_FPAGE_SIZE);
	case MP_DEVICES:
		return addr & 0xFFFC0000;
	}

	return addr_align(addr, CONFIG_SMALLEST_FPAGE_SIZE);
}

int mempool_search(memptr_t base, size_t size)
{
	int i;

	for (i = 0; i < sizeof(memmap) / sizeof(mempool_t); ++i) {
		if ((memmap[i].start <= base) &&
				(memmap[i].end >= (base + size))) {
			return i;
		}
	}
	return -1;
}

mempool_t *mempool_getbyid(int mpid)
{
	if (mpid == -1)
		return NULL;

	return memmap + mpid;
}

void memory_init()
{
	int i = 0, j = 0;
	uint32_t *shcsr = (uint32_t *) 0xE000ED24;

	fpages_init();

	ktable_init(&as_table);

	mem_desc = (kip_mem_desc_t *) kip_extra;

	/* Initialize mempool table in KIP */
	for (i = 0; i < sizeof(memmap) / sizeof(mempool_t); ++i) {
		switch(memmap[i].tag) {
		case MPT_USER_DATA:
		case MPT_USER_TEXT:
		case MPT_DEVICES:
		case MPT_AVAILABLE:
			mem_desc[j].base = addr_align((memmap[i].start),
					CONFIG_SMALLEST_FPAGE_SIZE) | i;
			mem_desc[j].size = addr_align(
				(memmap[i].end - memmap[i].start),
				CONFIG_SMALLEST_FPAGE_SIZE) | memmap[i].tag;
			j++;
			break;
		}
	}

	kip.memory_info.s.memory_desc_ptr =
			((void *) mem_desc) - ((void *) &kip);
	kip.memory_info.s.n = j;

	*shcsr |= 1 << 16;	/* Enable memfault */
}

/*
 * AS functions
 */

void as_setup_mpu(as_t *as, memptr_t sp)
{
	fpage_t *mpu[8];
	fpage_t *fp = as->first;
	int i = 0, j = 7, k = 0;

	/*
	 * We walk through fpage list
	 * [0:k] are always-mapped fpages + LRU page + stack page
	 * [k:7] are others
	 */
	while (fp != NULL) {
		if (fp->fpage.flags & FPAGE_ALWAYS ||
				fp == as->lru ||
				addr_in_fpage(sp, fp)) {
			if (k < 8)
				mpu[k++] = fp;
		}
		else {
			if (j >= k)
				mpu[j--] = fp;
		} /* Non-always fpage mapping */
		fp = fp->as_next;
	}

	as->lru = NULL;

	for (i = 0; i < k; ++i) {
		mpu_setup_region(i, mpu[i]);
	}

	for (i = 7; i > j; --i) {
		mpu_setup_region(i, mpu[i]);
	}
}

void as_map_user(as_t *as)
{
	int i;

	for (i = 0; i < sizeof(memmap) / sizeof(mempool_t); ++i) {
		switch(memmap[i].tag) {
		case MPT_USER_DATA:
		case MPT_USER_TEXT:
			/* Create fpages only for user text and user data */
			assign_fpages(as, memmap[i].start,
				(memmap[i].end - memmap[i].start));
		}
	}
}

void as_map_ktext(as_t *as)
{
	int i;

	for (i = 0; i < sizeof(memmap) / sizeof(mempool_t); ++i) {
		if (memmap[i].tag == MPT_KERNEL_TEXT) {
			assign_fpages(as, memmap[i].start,
				(memmap[i].end - memmap[i].start));
		}
	}
}

as_t *as_create(uint32_t as_spaceid)
{
	as_t *as = (as_t *) ktable_alloc(&as_table);

	/* assert as == NULL */

	as->as_spaceid = as_spaceid;
	as->first = NULL;

	return as;
}

int map_area(as_t *src, as_t *dst, memptr_t base, size_t size,
		map_action_t action, int is_priviliged)
{
	/* Most complicated part of mapping subsystem */
	memptr_t end = base + size, probe = base;
	fpage_t *fp = src->first, *first = NULL, *last = NULL;
	int last_invalid = 0;

	/* FIXME: reverse mappings (i.e. thread 1 maps 0x1000 to thread 2,
	 * than thread 2 does the same to thread 1).
	 */

	/* For priviliged thread (ROOT), we use shadowed mapping,
	 * so first we will check if that fpages exist and then
	 * create them.
	 */

	/* FIXME: checking existence of fpages */

	if (is_priviliged) {
		assign_fpages_ext(-1, src, base, size, &first, &last);
		if (src == dst) {
			/* Maps to itself, ignore other actions */
			return 0;
		}
	}
	else {
		/* We should determine first and last fpage we will map to:
		 *
		 * +----------+    +----------+    +----------+
		 * |   first  | -> |          | -> |  last    |
		 * +----------+    +----------+    +----------+
		 *     ^base            ^    +size      =    ^end
		 *                      | probe
		 *
		 * probe checks that addresses in fpage are sequental
		 */
		while (fp) {
			if (!first && addr_in_fpage(base, fp)) {
				first = fp;
			}

			if (!last && addr_in_fpage(end, fp)) {
				last = fp;
				break;
			}

			if (first) {
				/* Check weather if addresses in fpage list
				 * are sequental 
				 */
				if (!addr_in_fpage(probe, fp))
					return -1;

				probe += (1 << fp->fpage.shift);
			}

			fp = fp->as_next;
		}
	}

	if (!last || !first) {
		/* Not in address space or error */
		return -1;
	}

	if (first == last)
		last_invalid = 1;

	/* That is a problem because we should split
	 * fpages into two (and split all mappings too)
	 */

	first = split_fpage(src, first, base, 1);

	/* If first and last were same pages, after first split,
	 * last fpage will be invalidated, so we search it again
	 */
	if (last_invalid) {
		fp = first;
		while (fp) {
			if (addr_in_fpage(end, fp)) {
				last = fp;
				break;
			}
			fp = fp->as_next;
		}
	}

	last  = split_fpage(src, last, end, 0);

	/* Map chain of fpages */

	fp = first;
	do {
		map_fpage(src, dst, fp, action);
		fp = fp->as_next;
	} while (fp != last);

	return 0;
}

#ifdef CONFIG_KDB

static char *kdb_mempool_prop(mempool_t *mp)
{
	static char mempool[10] = "--- --- -";
	mempool[0] = (mp->flags & MP_KR)? 'r' : '-';
	mempool[1] = (mp->flags & MP_KW)? 'w' : '-';
	mempool[2] = (mp->flags & MP_KX)? 'x' : '-';

	mempool[4] = (mp->flags & MP_UR)? 'r' : '-';
	mempool[5] = (mp->flags & MP_UW)? 'w' : '-';
	mempool[6] = (mp->flags & MP_UX)? 'x' : '-';

	mempool[8] = (mp->flags & MP_DEVICES) ?
	             'D' : (mp->flags & MP_MEMPOOL) ?
	                   'M' : (mp->flags & MP_AHB_RAM) ?
	                         'A' : (mp->flags & MP_SRAM) ?
	                               'S' : 'N';
	return mempool;
}

void kdb_dump_mempool()
{
	int i = 0;

	dbg_printf(DL_KDB,
		"%10s %10s [%8s:%8s] %10s\n",
		"NAME", "SIZE", "START", "END", "FLAGS");

	for (i = 0; i < sizeof(memmap) / sizeof(mempool_t); ++i) {
		dbg_printf(DL_KDB,
			"%10s %10d [%p:%p] %10s\n",
			memmap[i].name, (memmap[i].end - memmap[i].start),
			memmap[i].start, memmap[i].end,
			kdb_mempool_prop(&(memmap[i])));
	}
}

void kdb_dump_as()
{
	int idx = 0, nl = 0;
	as_t *as = NULL;
	fpage_t *fpage = NULL;

	for_each_in_ktable(as, idx, &as_table) {
		fpage = as->first;
		dbg_printf(DL_KDB, "Address Space %p\n", as->as_spaceid);

		while (fpage) {
			dbg_printf(DL_KDB,
				"MEM: fpage %5s [b:%p, sz:2**%d]\n",
				memmap[fpage->fpage.mpid].name,
				fpage->fpage.base, fpage->fpage.shift);
			fpage = fpage->as_next;
			++nl;

			if (nl == 12) {
				dbg_puts("Press any key...\n");
				while (dbg_getchar() == 0)
					/* */ ;
				nl = 0;
			}
		}
	}
}

#endif	/* CONFIG_KDB */
