/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if !defined(CONFIG_KPROBES)
#error __FILE__ " is the part of KProbes implementation."
#endif

#include <platform/irq.h>
#include <platform/hw_debug.h>
#include <platform/cortex_m.h>

extern void arch_kprobe_handler(uint32_t *stack);

void debugmon_handler(void) __NAKED;
void debugmon_handler(void)
{
	/* select interrupted stack */
	__asm__ __volatile__("and r0, lr, #4");
	__asm__ __volatile__("cmp r0, #0");
	__asm__ __volatile__("ite eq");
	__asm__ __volatile__("mrseq r0, msp");
	__asm__ __volatile__("mrsne r0, psp");

	/* save r4-r11 */
	__asm__ __volatile__("push {r4-r11}");
	__asm__ __volatile__("mov r1, sp");

	/*
	 * arch_kprobe_handler(uint32_t *stack, uint32_t *kp_regs)
	 * r0 = r0-r3,r12,lr,pc,psr
	 * r1 = r4-r11
	 */
	__asm__ __volatile__("push {lr}");
	__asm__ __volatile__("bl arch_kprobe_handler");
	__asm__ __volatile__("pop {lr}");

	/* override r4-r11 */
	__asm__ __volatile__("pop {r4-r11}");

	/* NOTE: No support stack modification for the time being */

	__asm__ __volatile__("bx lr");
}
