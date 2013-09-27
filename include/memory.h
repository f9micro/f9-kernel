/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#include <types.h>
#include <fpage.h>
#include <platform/link.h>

/*
 * Each address space is associated with one or more thread.
 * AS represented as linked list of fpages, mappings are the same.
 */

typedef enum { MAP, GRANT, UNMAP } map_action_t;

typedef struct {
	uint32_t as_spaceid;	/*! Space Identifier */
	struct fpage *first;	/*! head of fpage list */

	struct fpage *mpu_first;	/*! head of MPU fpage list */
	struct fpage *mpu_stack_first;	/*! head of MPU stack fpage list */
} as_t;

/**
 * Memory pool represents area of physical address space
 * We set flags to it (kernel & user permissions),
 * and rules for fpage creation
 */
typedef struct {
#ifdef CONFIG_DEBUG
	char *name;
#endif

	memptr_t start;
	memptr_t end;

	uint32_t flags;
	uint32_t tag;
} mempool_t;

/* Kernel permissions flags */
#define MP_KR		0x0001
#define MP_KW		0x0002
#define MP_KX		0x0004

/* Userspace permissions flags */
#define MP_UR		0x0010
#define MP_UW		0x0020
#define MP_UX		0x0040

/* Fpage type */
#define MP_NO_FPAGE	0x0000		/*! Not mappable */
#define MP_SRAM		0x0100		/*! Fpage in SRAM: granularity 1 << */
#define MP_AHB_RAM	0x0200		/*! Fpage in AHB SRAM: granularity 64 words, bit bang mappings */
#define MP_DEVICES	0x0400		/*! Fpage in AHB/APB0/AHB0: granularity 16 kB */
#define MP_MEMPOOL	0x0800		/*! Entire mempool is mapped  */

/* Map memory from mempool always (for example text is mapped always because
 * without it thread couldn't run)
 * other fpages mapped on request because we limited in MPU resources)
 */
#define MP_MAP_ALWAYS 	0x1000

#define MP_FPAGE_MASK	0x0F00

#define MP_USER_PERM(mpflags) ((mpflags & 0xF0) >> 4)

typedef enum {
	MPT_KERNEL_TEXT,
	MPT_KERNEL_DATA,
	MPT_USER_TEXT,
	MPT_USER_DATA,
	MPT_AVAILABLE,
	MPT_DEVICES,
	MPT_UNKNOWN = -1
} mempool_tag_t;

/* If fpage_size = 0, memory is not allocable */
#ifdef CONFIG_DEBUG
#define DECLARE_MEMPOOL(name_, start_, end_, flags_, tag_) \
{						\
	.name = name_,				\
	.start = (memptr_t) (start_),		\
	.end  = (memptr_t) (end_),		\
	.flags = flags_,			\
	.tag = tag_				\
}
#else
#define DECLARE_MEMPOOL(name_, start_, end_, flags_, tag_) 	\
{						\
	.start = (memptr_t) (start_),		\
	.end  = (memptr_t) (end_),		\
	.flags = flags_,			\
	.tag = tag_				\
}
#endif

#define DECLARE_MEMPOOL_2(name, prefix, flags, tag) \
	DECLARE_MEMPOOL(name, \
			&(prefix ## _start), &(prefix ## _end), flags, tag)

typedef enum {
	MPU_DISABLED,
	MPU_ENABLED
} mpu_state_t;

void memory_init(void);

memptr_t mempool_align(int mpid, memptr_t addr);
int mempool_search(memptr_t base, size_t size);
mempool_t *mempool_getbyid(int mpid);

int map_area(as_t *src, as_t *dst, memptr_t base, size_t size,
		map_action_t action, int is_priviliged);

as_t *as_create(uint32_t as_spaceid);
void as_setup_mpu(as_t *as, memptr_t sp, memptr_t pc,
		memptr_t stack_base, size_t stack_size);
void as_map_user(as_t *as);
void as_map_ktext(as_t *as);

void mpu_enable(mpu_state_t i);
void mpu_setup_region(int n, struct fpage *fp);
int mpu_select_lru(as_t *as, uint32_t addr);

#endif /* MEMORY_H_ */
