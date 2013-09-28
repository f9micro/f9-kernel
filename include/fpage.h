/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FPAGE_H_
#define FPAGE_H_

#include <memory.h>
#include <types.h>

#define FPAGE_ALWAYS    0x1     /*! Fpage is always mapped in MPU */
#define FPAGE_CLONE     0x2     /*! Fpage is mapped from other AS */
#define FPAGE_MAPPED    0x4     /*! Fpage is mapped with MAP (
				   unavailable in original AS) */

/**
 * Flexible page (fpage_t)
 *
 * as_next - next in address space chain
 * map_next - next in mappings chain (cycle list)
 *
 * base - base address of fpage
 * shift - size of fpage == 1 << shift
 * rwx - access bits
 * mpid - id of memory pool
 * flags - flags
 */
struct fpage {
	struct fpage *as_next;
	struct fpage *map_next;
	struct fpage *mpu_next;

	union {
		struct {
			uint32_t base;
			uint32_t mpid : 6;
			uint32_t flags : 6;
			uint32_t shift : 16;
			uint32_t rwx : 4;
		} fpage;
		uint32_t raw[2];
	};

#ifdef CONFIG_KDB
	int used;
#endif /* CONFIG_KDB */
};

typedef struct fpage fpage_t;

#define FPAGE_BASE(fp) 	(fp)->fpage.base
#define FPAGE_SIZE(fp)  (1 << (fp)->fpage.shift)
#define FPAGE_END(fp)	(FPAGE_BASE(fp) + FPAGE_SIZE(fp))

static inline int addr_in_fpage(memptr_t addr, fpage_t *fpage, int incl_end)
{
	return ((addr >= FPAGE_BASE(fpage) && addr < FPAGE_END(fpage)) ||
			(incl_end && addr == FPAGE_END(fpage)));
}

#endif /* FPAGE_H_ */
