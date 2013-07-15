/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <kprobes.h>
#include <debug.h>

extern void ktimer_handler();
extern void svc_handler();

int pre_count, post_count;

int test_prehandler()
{
	pre_count++;
	return 0;
}

int test_posthandler()
{
	post_count++;
	return 0;
}

int test_add(int a, int b)
{
	return a + b;
}

int kt_count, kt_adv_count;
int kt_prehandler()
{
	kt_count++;
	if (kt_count == 10000) {
		kt_count = 0;
		kt_adv_count++;
	}
	return 0;
}

int svc_count;
int svc_posthandler()
{
	svc_count++;
	return 0;
}

void kdb_show_kprobe_info()
{
	static int init = 0;
	if (init == 0) {
		init ++;
		static struct kprobe k1,k2;

		dbg_puts("Init kprobe test...\n");

		k1.addr = ktimer_handler;
		k1.pre_handler = kt_prehandler;
		k1.post_handler = NULL;
		kprobe_register(&k1);

		k2.addr = svc_handler;
		k2.pre_handler = NULL;
		k2.post_handler = svc_posthandler;
		kprobe_register(&k2);
	}
	dbg_printf(DL_KDB, "kt = %d, %d\n", kt_count, kt_adv_count);
	dbg_printf(DL_KDB, "SVCall = %d times\n", svc_count);
}

void kdb_test_kprobe()
{
	static struct kprobe k;
	int ret, ret2;

	k.addr = test_add;
	k.pre_handler = test_prehandler;
	k.post_handler = test_posthandler;

	pre_count = post_count = 0;

	kprobe_register(&k);
	ret = test_add(1, 2);
	ret2 = test_add(1, 2);
	kprobe_unregister(&k);

	// check function is correct
	if (ret == 3 && ret2 == 3) {
		dbg_printf(DL_KDB, "PASS: test_add\n");
	} else {
		dbg_printf(DL_KDB, "FAIL: test_add\n");
	}

	// check kprobe_register works
	if (pre_count == 2 && pre_count == post_count) {
		dbg_printf(DL_KDB, "PASS: kprobe_register\n");
	} else {
		dbg_printf(DL_KDB, "FAIL: kprobe_register count = %d, %d\n", pre_count, post_count);
	}

	ret = test_add(1, 2);

	// check kprobe_unregister works
	if (pre_count == 2 && pre_count == post_count) {
		dbg_printf(DL_KDB, "PASS: kprobe_unregister\n");
	} else {
		dbg_printf(DL_KDB, "FAIL: kprobe_unregister count = %d, %d\n", pre_count, post_count);
	}
}
