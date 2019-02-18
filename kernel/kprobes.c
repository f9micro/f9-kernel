/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <kprobes.h>
#include <platform/armv7m.h>
#include <platform/breakpoint.h>
#include <init_hook.h>
#include <debug.h>

static struct kprobe *kp_list;

void kprobe_init()
{
	kp_list = NULL;
	kprobe_arch_init();
}

INIT_HOOK(kprobe_init, INIT_LEVEL_KERNEL);

struct kprobe *kplist_search(void *addr)
{
	struct kprobe *cur = kp_list;
	while (cur != NULL) {
		if (cur->addr == addr)
			return cur;
		cur = cur->next;
	}
	return NULL;
}

void kplist_add(struct kprobe *kp)
{
	kp->next = kp_list;
	kp_list = kp;
}

void kplist_del(struct kprobe *kp)
{
	struct kprobe *cur = kp_list;
	if (kp_list == kp) {
		kp_list = kp->next;
		return;
	}
	while (cur != NULL) {
		if (cur->next == kp) {
			cur->next = kp->next;
			break;
		}
		cur = cur->next;
	}
}

int kprobe_register(struct kprobe *kp)
{
	kp->addr = (void *) ((uint32_t) kp->addr & ~(1UL));
	if (is_thumb32(*(uint16_t *) kp->addr))
		kp->step_addr = kp->addr + 4;
	else
		kp->step_addr = kp->addr + 2;

	if (kprobe_arch_add(kp) < 0)
		return -1;

	kplist_add(kp);
	return 0;
}

int kprobe_unregister(struct kprobe *kp)
{
	kplist_del(kp);
	kprobe_arch_del(kp);
	return 0;
}

static int __kretprobe_post_handler(struct kprobe *kp, uint32_t *stack,
                                    uint32_t *kp_regs)
{
	struct kretprobe * rp = (struct kretprobe *) kp;

	kprobe_unregister(kp);

	kp->addr = rp->backup_addr;

	kretprobe_register(rp);
	return 0;
}

static int __kretprobe_pre_handler(struct kprobe *kp, uint32_t *stack,
                                   uint32_t *kp_regs)
{
	struct kretprobe * rp = (struct kretprobe *) kp;

	kprobe_unregister(kp);

	rp->backup_addr = kp->addr;
	kp->addr = (void *) stack[REG_LR];
	kp->pre_handler = rp->handler;
	kp->post_handler = __kretprobe_post_handler;

	kprobe_register(kp);
	return 0;
}

int kretprobe_register(struct kretprobe *rp)
{
	rp->kp.pre_handler = __kretprobe_pre_handler;
	rp->kp.post_handler = NULL;
	return kprobe_register((struct kprobe *) rp);
}

int kretprobe_unregister(struct kretprobe *rp)
{
	return kprobe_unregister(&rp->kp);
}

void kprobe_prebreak(uint32_t *stack, uint32_t *kp_regs)
{
	struct kprobe *kp = kp_list;
	while (kp != NULL) {
		if ((uint32_t) kp->addr == stack[REG_PC] && kp->pre_handler)
			kp->pre_handler(kp, stack, kp_regs);
		kp = kp->next;
	}
}

void kprobe_postbreak(uint32_t *stack, uint32_t *kp_regs)
{
	struct kprobe *kp = kp_list;
	while (kp != NULL) {
		if ((uint32_t) kp->step_addr == stack[REG_PC] &&
		    kp->post_handler)
			kp->post_handler(kp, stack, kp_regs);
		kp = kp->next;
	}
}

void kprobe_breakpoint_enable(uint32_t *stack)
{
	struct kprobe *kp = kp_list;
	while (kp != NULL) {
		if ((uint32_t) kp->step_addr == stack[REG_PC])
			enable_breakpoint(kp->bkpt);
		kp = kp->next;
	}
}

void kprobe_breakpoint_disable(uint32_t *stack)
{
	struct kprobe *kp = kp_list;
	while (kp != NULL) {
		if ((uint32_t) kp->addr == stack[REG_PC])
			disable_breakpoint(kp->bkpt);
		kp = kp->next;
	}
}
