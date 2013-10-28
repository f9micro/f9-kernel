/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_IRQ_H_
#define PLATFORM_IRQ_H_

#include <platform/link.h>
#include <platform/cortex_m.h>

void irq_init(void);

static inline void irq_disable(void)
{
	__asm__ __volatile__ ("cpsid i");
}

static inline void irq_enable(void)
{
	__asm__ __volatile__ ("cpsie i");
}

static inline void irq_svc(void)
{
	__asm__ __volatile__ ("svc #0");
}

static inline int irq_number(void)
{
	int irqno;

	__asm__ __volatile__ (
		"mrs r0, ipsr\n"
		"mov r0, %0"
		: "=r" (irqno) : : "r0");

	return irqno;
}


#define IRQ_HANDLER(name, sub)						\
	void name(void)							\
	{								\
		sub();							\
	}


#endif	/* PLATFORM_IRQ_H_ */
