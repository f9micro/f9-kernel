/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_ARMV7M_H_
#define PLATFORM_ARMV7M_H_

#include <types.h>

static inline void wait_for_interrupt(void)
{
	__asm__ __volatile__ ("wfi");
}

static inline uint32_t read_msp(void)
{
	uint32_t result;

	__asm__ __volatile__ (
		"mrs r0, msp\n"
		"mov %0, r0"
		: "=r"(result)
		:
		: "r0"
	);

	return result;
}

enum register_stack_t {
	/* Saved by hardware */
	REG_R0,
	REG_R1,
	REG_R2,
	REG_R3,
	REG_R12,
	REG_LR,
	REG_PC,
	REG_xPSR
};

#define RESERVED_STACK \
	(8 * sizeof(uint32_t))

#endif	/* PLATFORM_ARMV7M_H_ */
