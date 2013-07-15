/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef KPROBES_H
#define KPROBES_H

#include <types.h>

#define is_thumb32(inst) ((unsigned)(inst) >= 0xe800)

struct kprobe {
	void *addr;
	int (*pre_handler)();
	int (*post_handler)();

	struct kprobe *next;
	int bkptid;
};

void kprobe_init();
int kprobe_register(struct kprobe *p);
int kprobe_unregister(struct kprobe *p);
void kprobe_prebreak(void *addr);
void kprobe_postbreak(void *addr);
struct kprobe *kplist_search(void *addr);

void kprobe_arch_init();
int kprobe_arch_add(struct kprobe *kp);
int kprobe_arch_del(struct kprobe *kp);
void arch_kprobe_handler();

#endif /* KPROBE_H */
