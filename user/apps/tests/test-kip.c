/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/kip.h>
#include <l4io.h>

#include "tests.h"

/*
 * Test: Verify L4_GetKernelInterface() returns valid KIP.
 */
__USER_TEXT
void test_kip_access(void)
{
	void *kip;

	TEST_RUN("kip_access");

	kip = L4_GetKernelInterface();

	/* KIP pointer should be non-NULL */
	if (kip) {
		TEST_PASS("kip_access");
	} else {
		printf("L4_GetKernelInterface() returned NULL\n");
		TEST_FAIL("kip_access");
	}
}

/*
 * Test: Verify L4_NumProcessors() returns at least 1.
 */
__USER_TEXT
void test_kip_processors(void)
{
	void *kip;
	L4_Word_t num_procs;

	TEST_RUN("kip_processors");

	kip = L4_GetKernelInterface();
	if (!kip) {
		printf("L4_GetKernelInterface() returned NULL\n");
		TEST_FAIL("kip_processors");
		return;
	}

	num_procs = L4_NumProcessors(kip);

	/* Should have at least 1 processor */
	if (num_procs >= 1) {
		TEST_PASS("kip_processors");
	} else {
		printf("L4_NumProcessors() returned %lu\n",
		       (unsigned long)num_procs);
		TEST_FAIL("kip_processors");
	}
}
