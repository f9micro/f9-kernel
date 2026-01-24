/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * Fpage chain functions
 */

#include <types.h>
#include <memory.h>
#include <fpage.h>
#include <fpage_impl.h>

#include <error.h>
#include <debug.h>

DECLARE_KTABLE(fpage_t, fpage_table, CONFIG_MAX_FPAGES);

/*
 * remove_fpage_from_list macro moved to include/fpage.h for shared use
 * (e.g., by platform/stm32-common/mpu.c to prevent circular list bugs)
 */

/*
 * Helper functions
 */

/*
 * Compute the position of the lowest set bit (trailing zeros).
 * This determines the alignment of an address for fpage sizing.
 * Returns the shift value such that (1 << shift) is the largest
 * power of 2 that divides addr.
 */
static int fp_addr_log2(memptr_t addr)
{
	int shift = 0;

	if (addr == 0)
		return CONFIG_LARGEST_FPAGE_SHIFT;

	while ((addr & 1) == 0) {
		++shift;
		addr >>= 1;
	}

	return shift;
}

/*
 * Compute floor(log2(size)) - the position of the highest set bit.
 * Returns the largest shift such that (1 << shift) <= size.
 * Used to determine the largest power-of-2 fpage that fits within size.
 */
static int fp_size_log2(size_t size)
{
	int shift = 0;

	if (size == 0)
		return 0;

	while (size > 1) {
		++shift;
		size >>= 1;
	}

	return shift;
}

void fpages_init(void)
{
	ktable_init(&fpage_table);
}

/**
 * Insert chain of fpages into address space
 * @param first, last - fpage chain
 * @param as address space
 *
 * NOTE: Not checks if as and this chain overlapping
 */
static void insert_fpage_chain_to_as(as_t *as, fpage_t *first, fpage_t *last)
{
	fpage_t *fp = as->first;

	if (!fp) {
		/* First chain in AS */
		as->first = first;
		return;
	}

	if (last->fpage.base < fp->fpage.base) {
		/* Add chain into beginning */
		last->as_next = as->first;
		as->first = first;
	} else {
		/* Search for chain in the middle */
		while (fp->as_next) {
			if (FPAGE_BASE(last) < FPAGE_BASE(fp->as_next)) {
				last->as_next = fp->as_next;
				break;
			}
			fp = fp->as_next;
		}
		fp->as_next = first;
	}

}

/**
 * Insert single fpage into address space
 * @param fpage fpage
 * @param as address space
 *
 * NOTE: Not checks if as and this chain overlapping
 */
static void insert_fpage_to_as(as_t *as, fpage_t *fpage)
{
	insert_fpage_chain_to_as(as, fpage, fpage);
}

/**
 * Remove fpage from address space
 * @param fp fpage
 * @param as address space
 *
 * If fpage is not exist in as fpages list, does nothing
 */
static void remove_fpage_from_as(as_t *as, fpage_t *fp)
{
	remove_fpage_from_list(as, fp, first, as_next);
	remove_fpage_from_list(as, fp, mpu_first, mpu_next);
}

/* FIXME: Support for bit-bang regions. */
/**
 * Create and initialize new fpage
 * @param base base address of fpage
 * @param shift (1 << shift) - fpage size
 * @param mpid - id of mpool
 */
static int fpage_alloc_count = 0;

static fpage_t *create_fpage(memptr_t base, size_t shift, int mpid)
{
	fpage_t *fpage = (fpage_t *) ktable_alloc(&fpage_table);

	if (!fpage) {
		dbg_printf(DL_KDB,
		           "FPAGE: alloc failed! count=%d base=%p shift=%d mpid=%d\n",
		           fpage_alloc_count, base, shift, mpid);
	} else {
		fpage_alloc_count++;
		if ((fpage_alloc_count % 50) == 0)
			dbg_printf(DL_KDB, "FPAGE: allocated %d fpages\n",
			           fpage_alloc_count);
	}

	assert((intptr_t) fpage);

	fpage->as_next = NULL;
	fpage->map_next = fpage; 	/* That is first fpage in mapping */
	fpage->mpu_next = NULL;
	fpage->fpage.mpid = mpid;
	fpage->fpage.flags = 0;
	fpage->fpage.rwx = MP_USER_PERM(mempool_getbyid(mpid)->flags);

	fpage->fpage.base = base;
	fpage->fpage.shift = shift;

	if (mempool_getbyid(mpid)->flags & MP_MAP_ALWAYS)
		fpage->fpage.flags |= FPAGE_ALWAYS;

	return fpage;
}

void destroy_fpage(fpage_t *fpage)
{
	ktable_free(&fpage_table, fpage);
}

static void create_fpage_chain(memptr_t base, size_t size, int mpid,
                               fpage_t **pfirst, fpage_t **plast)
{
	int shift, sshift, bshift;
	fpage_t *fpage = NULL;

	while (size) {
		/* Select minimum of base alignment and largest fitting size.
		 * bshift: base alignment (trailing zeros) ensures MPU alignment
		 * sshift: floor(log2(size)) gives largest power-of-2 that fits
		 */
		bshift = fp_addr_log2(base);
		sshift = fp_size_log2(size);

		shift = (bshift < sshift) ? bshift : sshift;

		if (!*pfirst) {
			/* Create first page */
			fpage = create_fpage(base, shift, mpid);
			*pfirst = fpage;
			*plast = fpage;
		} else {
			/* Build chain */
			fpage->as_next = create_fpage(base, shift, mpid);
			fpage = fpage->as_next;
			*plast = fpage;
		}

		size -= (1 << shift);
		base += (1 << shift);
	}
}

fpage_t *split_fpage(as_t *as, fpage_t *fpage, memptr_t split, int rl)
{
	memptr_t base = fpage->fpage.base,
	         end = fpage->fpage.base + (1 << fpage->fpage.shift);
	fpage_t *lfirst = NULL, *llast = NULL, *rfirst = NULL, *rlast = NULL;

	/* For rl=1 (right side), round DOWN to include the split point.
	 * For rl=0 (left side), round UP to exclude past the split point.
	 */
	if (rl)
		split = mempool_align_base(fpage->fpage.mpid, split);
	else
		split = mempool_align(fpage->fpage.mpid, split);

	if (!as)
		return NULL;

	/* Check if we can split something */
	if (split == base || split == end) {
		return fpage;
	}

	if (fpage->map_next != fpage) {
		/* Splitting not supported for mapped pages */
		/* UNIMPLIMENTED */
		return NULL;
	}

	/* Split fpage into two chains of fpages */
	create_fpage_chain(base, (split - base),
	                   fpage->fpage.mpid, &lfirst, &llast);
	create_fpage_chain(split, (end - split),
	                   fpage->fpage.mpid, &rfirst, &rlast);

	remove_fpage_from_as(as, fpage);
	ktable_free(&fpage_table, fpage);

	insert_fpage_chain_to_as(as, lfirst, llast);
	insert_fpage_chain_to_as(as, rfirst, rlast);

	if (rl == 0)
		return llast;
	return rfirst;
}


int assign_fpages_ext(int mpid, as_t *as, memptr_t base, size_t size,
                      fpage_t **pfirst, fpage_t **plast)
{
	fpage_t **fp;
	memptr_t  end;

	if (size <= 0)
		return -1;

	/* if mpid is unknown, search using base addr */
	if (mpid == -1) {
		if ((mpid = mempool_search(base, size)) == -1) {
			/* Cannot find appropriate mempool, return error */
			return -1;
		}
	}

	/* Check if mempool supports fpage creation */
	if (!(mempool_getbyid(mpid)->flags & MP_FPAGE_MASK)) {
		/* Mempool does not support fpages (e.g., kernel text/data/bss) */
		return -1;
	}

	end = base + size;

	if (as) {
		/* find unmapped space */
		fp = &as->first;
		while (base < end && *fp) {
			if (base < FPAGE_BASE(*fp)) {
				fpage_t *first = NULL, *last = NULL;
				size = (end < FPAGE_BASE(*fp) ? end : FPAGE_BASE(*fp)) - base;

				dbg_printf(DL_MEMORY,
				           "MEM: fpage chain %s [b:%p, sz:%p] as %p\n",
				           mempool_getbyid(mpid)->name, base, size, as);

				{
					/* Round UP base to prevent over-mapping.
					 * S1 fix: mempool_align_base rounds DOWN,
					 * granting access below requested address.
					 */
					memptr_t abase = mempool_align(mpid, base);
					memptr_t aend = mempool_align(mpid, base + size);

					/* Empty region after alignment: skip */
					if (abase >= aend) {
						base = FPAGE_BASE(*fp);
						fp = &(*fp)->as_next;
						continue;
					}

					create_fpage_chain(abase, aend - abase, mpid,
					                   &first, &last);
				}

				/* NULL guard: create_fpage_chain may fail */
				if (!first || !last)
					return -1;

				last->as_next = *fp;
				*fp = first;
				fp = &last->as_next;

				if (!*pfirst)
					*pfirst = first;
				*plast = last;

				base = FPAGE_END(*fp);
			} else if (base < FPAGE_END(*fp)) {
				if (!*pfirst)
					*pfirst = *fp;
				*plast = *fp;

				base = FPAGE_END(*fp);
			}

			fp = &(*fp)->as_next;
		}

		if (base < end) {
			fpage_t *first = NULL, *last = NULL;
			size = end - base;

			dbg_printf(DL_MEMORY,
			           "MEM: fpage chain %s [b:%p, sz:%p] as %p\n",
			           mempool_getbyid(mpid)->name, base, size, as);

			{
				/* S1 fix: round UP base to prevent over-mapping */
				memptr_t abase = mempool_align(mpid, base);
				memptr_t aend = mempool_align(mpid, base + size);

				/* Empty region check */
				if (abase < aend) {
					create_fpage_chain(abase, aend - abase, mpid,
					                   &first, &last);
				}
			}

			/* Only link if chain was created */
			if (first && last) {
				*fp = first;

				if (!*pfirst)
					*pfirst = first;
				*plast = last;
			}
		}
	} else {
		dbg_printf(DL_MEMORY,
		           "MEM: fpage chain %s [b:%p, sz:%p] as %p\n",
		           mempool_getbyid(mpid)->name, base, size, as);

		{
			/* S1 fix: round UP base to prevent over-mapping */
			memptr_t abase = mempool_align(mpid, base);
			memptr_t aend = mempool_align(mpid, base + size);

			/* Empty region check: return error if nothing to map */
			if (abase >= aend)
				return -1;

			create_fpage_chain(abase, aend - abase, mpid, pfirst, plast);
		}
	}

	return 0;
}

int assign_fpages(as_t *as, memptr_t base, size_t size)
{
	fpage_t *first = NULL, *last = NULL;
	return assign_fpages_ext(-1, as, base, size, &first, &last);
}


int map_fpage(as_t *src, as_t *dst, fpage_t *fpage, map_action_t action)
{
	fpage_t *fpmap = (fpage_t *) ktable_alloc(&fpage_table);

	/* FIXME: check for fpmap == NULL */
	fpmap->as_next = NULL;
	fpmap->mpu_next = NULL;

	/* Copy fpage description */
	fpmap->raw[0] = fpage->raw[0];
	fpmap->raw[1] = fpage->raw[1];

	/* Set flags correctly: preserve FPAGE_ALWAYS for MPU prioritization */
	if (action == MAP)
		fpage->fpage.flags |= FPAGE_MAPPED;
	fpmap->fpage.flags = FPAGE_CLONE | (fpage->fpage.flags & FPAGE_ALWAYS);

	/* Insert into mapee list */
	fpmap->map_next = fpage->map_next;
	fpage->map_next = fpmap;

	/* Insert into AS */
	insert_fpage_to_as(dst, fpmap);

	dbg_printf(DL_MEMORY, "MEM: %s fpage %p [b:%p sz:2**%d] from %p to %p\n",
	           (action == MAP) ? "mapped" : "granted", fpmap,
	           FPAGE_BASE(fpmap), fpmap->fpage.shift, src, dst);

	return 0;
}

int unmap_fpage(as_t *as, fpage_t *fpage)
{
	fpage_t *fpprev = fpage;

	dbg_printf(DL_MEMORY, "MEM: unmapped fpage %p from %p\n", fpage, as);

	/* Fpages that are not mapped or granted
	 * are destroyed with its AS
	 */
	if (!(fpage->fpage.flags & FPAGE_CLONE))
		return -1;

	while (fpprev->map_next != fpage)
		fpprev = fpprev->map_next;

	/* Clear flags */
	fpprev->fpage.flags &= ~FPAGE_MAPPED;

	fpprev->map_next = fpage->map_next;
	remove_fpage_from_as(as, fpage);

	ktable_free(&fpage_table, fpage);

	return 0;
}
