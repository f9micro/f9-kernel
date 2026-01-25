/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/ipc.h>
#include <l4io.h>

#include "tests.h"

/*
 * Test: Verify L4_TimePeriod() creates valid time values.
 */
__USER_TEXT
void test_timer_period(void)
{
	L4_Time_t time;

	TEST_RUN("timer_period");

	/* Create a 1ms time period */
	time = L4_TimePeriod(1000);

	/* Time value should be non-zero */
	if (time.raw != 0) {
		TEST_PASS("timer_period");
	} else {
		printf("L4_TimePeriod(1000) returned zero\n");
		TEST_FAIL("timer_period");
	}
}

/*
 * Test: Verify L4_Sleep() returns without error.
 * Note: We don't test timing accuracy since QEMU timing is unreliable.
 */
__USER_TEXT
void test_timer_sleep(void)
{
	TEST_RUN("timer_sleep");

	/* Just verify sleep returns without hanging */
	L4_Sleep(L4_TimePeriod(1000)); /* 1ms */

	/* If we get here, sleep works */
	TEST_PASS("timer_sleep");
}
