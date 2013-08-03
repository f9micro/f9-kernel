/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef KPROBES_H
#define KPROBES_H

#include <types.h>

#define is_thumb32(inst) ((unsigned)(inst) >= 0xe800)

enum kp_register_t {
	/* Saved by DebugMon Handler */
	KP_REG_R4,
	KP_REG_R5,
	KP_REG_R6,
	KP_REG_R7,
	KP_REG_R8,
	KP_REG_R9,
	KP_REG_R10,
	KP_REG_R11,
};

struct kprobe;
typedef int (*kprobe_pre_handler_t) (struct kprobe *kp, uint32_t *stack,
				     uint32_t *kp_regs);
typedef int (*kprobe_post_handler_t) (struct kprobe *kp, uint32_t *stack,
				      uint32_t *kp_regs);

struct kprobe {
	void *addr;
	kprobe_pre_handler_t pre_handler;
	kprobe_post_handler_t post_handler;

	void *step_addr;
	struct kprobe *next;
	int bkptid;
};

void kprobe_init();
int kprobe_register(struct kprobe *p);
int kprobe_unregister(struct kprobe *p);
void kprobe_prebreak(uint32_t *stack, uint32_t *kp_regs);
void kprobe_postbreak(uint32_t *stack, uint32_t *kp_regs);
struct kprobe *kplist_search(void *addr);

void kprobe_arch_init();
int kprobe_arch_add(struct kprobe *kp);
int kprobe_arch_del(struct kprobe *kp);
void arch_kprobe_handler();

#endif /* KPROBE_H */
