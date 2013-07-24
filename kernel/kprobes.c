/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <kprobes.h>

#ifdef CONFIG_KPROBES

static struct kprobe *kp_list;

void kprobe_init()
{
	kp_list = NULL;
	kprobe_arch_init();
}

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
	int ret;
	kp->addr = (void *) ((uint32_t) kp->addr & ~(1UL));
	ret = kprobe_arch_add(kp);
	if (ret < 0)
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

void kprobe_prebreak(void *addr)
{
	struct kprobe *kp = kp_list;
	while (kp != NULL) {
		if (kp->addr == addr && kp->pre_handler) {
			kp->pre_handler();
		}
		kp = kp->next;
	}
}

void kprobe_postbreak(void *addr)
{
	struct kprobe *kp = kp_list;
	while (kp != NULL) {
		if (kp->addr == addr && kp->post_handler) {
			kp->post_handler();
		}
		kp = kp->next;
	}
}

#endif /* CONFIG_KPROBES */
