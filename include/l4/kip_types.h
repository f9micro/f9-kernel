/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef L4_KIP_TYPES_H_
#define L4_KIP_TYPES_H_

#include <stdint.h>

/*
 * KIP page. based on L4 X.2 Reference manual Rev. 7
 */

/*
 * NOTE: kip_mem_desc_t differs from L4 X.2 standard
 */
typedef struct {
	uint32_t 	base;	/* Last 6 bits contains poolid */
	uint32_t	size;	/* Last 6 bits contains tag */
} kip_mem_desc_t;

typedef union {
	struct {
		uint8_t  version;
		uint8_t  subversion;
		uint8_t  reserved;
	} s;
	uint32_t raw;
} kip_apiversion_t;

typedef union {
	struct {
		uint32_t  reserved : 28;
		uint32_t  ww : 2;
		uint32_t  ee : 2;
	} s;
	uint32_t raw;
} kip_apiflags_t;

typedef union {
	struct {
		uint16_t memory_desc_ptr;
		uint16_t n;
	} s;
	uint32_t raw;
} kip_memory_info_t;

typedef union {
	struct {
		uint32_t user_base;
		uint32_t system_base;
	} s;
	uint32_t raw;
} kip_threadinfo_t;

struct kip {
	/* First 256 bytes of KIP are compliant with L4 reference
	 * manual version X.2 and built in into flash (lower kip)
	 */
	uint32_t kernel_id;
	kip_apiversion_t api_version;
	kip_apiflags_t api_flags;
	uint32_t kern_desc_ptr;

	uint32_t reserved1[17];

	kip_memory_info_t memory_info;

	uint32_t reserved2[20];

	uint32_t utcb_info;		/* Unimplemented */
	uint32_t kip_area_info;		/* Unimplemented */

	uint32_t reserved3[2];

	uint32_t boot_info;		/* Unimplemented */
	uint32_t proc_desc_ptr;		/* Unimplemented */
	uint32_t clock_info;		/* Unimplemented */
	kip_threadinfo_t thread_info;
	uint32_t processor_info;	/* Unimplemented */

	/* Syscalls are ignored because we use SVC/PendSV instead of
	 * mapping SC into thread's address space
	 */
	uint32_t syscalls[12];
};

typedef struct kip kip_t;

#endif	/* L4_KIP_TYPES_H_ */
