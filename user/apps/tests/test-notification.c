/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4io.h>

#include "tests.h"

/* Notification bit masks for different event types */
#define NOTIFY_TIMER_ONESHOT (1 << 0)
#define NOTIFY_TIMER_PERIODIC (1 << 1)
#define NOTIFY_TIMER_MULTI (1 << 2)

/*
 * Test: Verify basic timer notification creation.
 *
 * Creates a one-shot timer that delivers notification via the unified
 * notification system. Tests:
 * - Timer creation with valid parameters
 * - Invalid parameter rejection (zero ticks/bits)
 * - Return value validation (non-zero handle on success)
 */
__USER_TEXT
void test_notification_timer_oneshot(void)
{
    TEST_RUN("notification_timer_oneshot");

    /* Test 1: Valid one-shot timer creation */
    L4_Word_t timer1 = L4_TimerNotify(100, NOTIFY_TIMER_ONESHOT, 0);
    if (timer1 == 0) {
        printf("  ✗ Failed to create valid one-shot timer\n");
        TEST_FAIL("notification_timer_oneshot");
        return;
    }
    /* printf("  ✓ Created one-shot timer: handle=0x%x\n", timer1); */

    /* Test 2: Invalid parameters (zero ticks) should fail */
    L4_Word_t timer_invalid1 = L4_TimerNotify(0, NOTIFY_TIMER_ONESHOT, 0);
    if (timer_invalid1 != 0) {
        printf("  ✗ Zero ticks should fail but returned 0x%x\n",
               timer_invalid1);
        TEST_FAIL("notification_timer_oneshot");
        return;
    }
    /* printf("  ✓ Zero ticks rejected correctly\n"); */

    /* Test 3: Invalid parameters (zero bits) should fail */
    L4_Word_t timer_invalid2 = L4_TimerNotify(100, 0, 0);
    if (timer_invalid2 != 0) {
        printf("  ✗ Zero bits should fail but returned 0x%x\n", timer_invalid2);
        TEST_FAIL("notification_timer_oneshot");
        return;
    }
    /* printf("  ✓ Zero bits rejected correctly\n"); */

    TEST_PASS("notification_timer_oneshot");
}

/*
 * Test: Verify periodic timer creation.
 *
 * Creates a periodic timer that delivers multiple notifications.
 * Tests:
 * - Periodic timer creation with periodic=1
 * - Return value validation
 */
__USER_TEXT
void test_notification_timer_periodic(void)
{
    TEST_RUN("notification_timer_periodic");

    /* Test: Valid periodic timer creation */
    L4_Word_t timer_periodic = L4_TimerNotify(1000, NOTIFY_TIMER_PERIODIC, 1);
    if (timer_periodic == 0) {
        printf("  ✗ Failed to create valid periodic timer\n");
        TEST_FAIL("notification_timer_periodic");
        return;
    }
    /* printf("  ✓ Created periodic timer: handle=0x%x\n", timer_periodic); */

    TEST_PASS("notification_timer_periodic");
}

/*
 * Test: Verify multiple concurrent timer creation.
 *
 * Creates multiple timers with different notification bits.
 * Tests:
 * - Multiple timer creation with different parameters
 * - Different notification bit masks
 * - Resource management (handles distinct)
 */
__USER_TEXT
void test_notification_multi_timer(void)
{
    TEST_RUN("notification_multi_timer");

    /* printf("  Creating multiple concurrent timers\n"); */

    /* Create 3 timers with different bits and periods */
    L4_Word_t timer1 = L4_TimerNotify(100, (1 << 0), 0); /* 100 ticks, bit 0 */
    L4_Word_t timer2 = L4_TimerNotify(150, (1 << 1), 0); /* 150 ticks, bit 1 */
    L4_Word_t timer3 = L4_TimerNotify(200, (1 << 2), 0); /* 200 ticks, bit 2 */

    if (timer1 == 0 || timer2 == 0 || timer3 == 0) {
        printf("  ✗ Failed to create all timers\n");
        TEST_FAIL("notification_multi_timer");
        return;
    }

    /* printf("  ✓ Timer 1: handle=0x%x (100 ticks, bit 0x1)\n", timer1); */
    /* printf("  ✓ Timer 2: handle=0x%x (150 ticks, bit 0x2)\n", timer2); */
    /* printf("  ✓ Timer 3: handle=0x%x (200 ticks, bit 0x4)\n", timer3); */

    /* Verify handles are distinct */
    if (timer1 == timer2 || timer2 == timer3 || timer1 == timer3) {
        printf("  ✗ Timer handles not unique\n");
        TEST_FAIL("notification_multi_timer");
        return;
    }
    /* printf("  ✓ All timer handles unique\n"); */

    TEST_PASS("notification_multi_timer");
}

/*
 * Test: Document notification statistics via KDB.
 *
 * This test documents that KDB 'N' command should show
 * notification activity after the timer tests run.
 */
__USER_TEXT
void test_notification_statistics(void)
{
    TEST_RUN("notification_statistics");

    /* printf("  Statistics: KDB 'N' shows queue/mask/batch metrics\n"); */

    TEST_PASS("notification_statistics");
}

/*
 * Demonstration: Unified notification system architecture.
 *
 * This test documents the complete notification flow:
 */
__USER_TEXT
void test_notification_architecture(void)
{
    TEST_RUN("notification_architecture");

    /* printf("  3-layer: Basic(signal/clear) Async(post/softirq)
     * Mask(OR/AND)\n"); */

    TEST_PASS("notification_architecture");
}
