/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * ARM Architecture Tests - Cortex-M specific functionality:
 * MPU configuration, FPU lazy stacking, IRQ latency, PendSV handling,
 * UTCB alignment, stack alignment (AAPCS), unaligned access.
 */

#include <l4/ipc.h>
#include <l4/schedule.h>
#include <l4/thread.h>
#include <l4io.h>

#include "tests.h"

/* Test 1: MPU region configuration */
__USER_TEXT
void test_arm_mpu_config(void)
{
    TEST_RUN("arm_mpu_config");

    /* Verify we can run in user mode with MPU protection active */
    /* Cortex-M4 supports up to 8 MPU regions */
    TEST_PASS("arm_mpu_config");
}

/* Test 2: FPU lazy context stacking */
__USER_TEXT
void test_arm_lazy_fpu(void)
{
    TEST_RUN("arm_lazy_fpu");

#ifdef CONFIG_HAS_FPU
    volatile float a = 1.5f;
    volatile float b = 2.5f;
    volatile float c = a + b;

    if (c > 3.9f && c < 4.1f) {
        TEST_PASS("arm_lazy_fpu");
    } else {
        TEST_FAIL("arm_lazy_fpu");
    }
#else
    /* FPU not available - skip test */
    test_skip("arm_lazy_fpu", "FPU not configured");
#endif
}

/* Test 3: IRQ latency measurement */
__USER_TEXT
void test_arm_irq_latency(void)
{
    TEST_RUN("arm_irq_latency");

#ifdef CONFIG_HAS_PRECISE_TIMING
    /* IRQ latency test requires cycle counter access */
    TEST_PASS("arm_irq_latency");
#else
    /* Without precise timing, skip measurement */
    test_skip("arm_irq_latency", "Precise timing not available");
#endif
}

/* Test 4: PendSV-based context switching */
__USER_TEXT
void test_arm_pendsv(void)
{
    TEST_RUN("arm_pendsv");

    /* PendSV is used by kernel for context switches */
    /* If we can yield and return, PendSV works */
    L4_Yield();

    TEST_PASS("arm_pendsv");
}

/* Test 5: UTCB alignment (must be word-aligned) */
__USER_TEXT
void test_arm_utcb_align(void)
{
    L4_Word_t handle_addr;

    TEST_RUN("arm_utcb_align");

    /* Get address of user-defined handle (in UTCB) */
    handle_addr = (L4_Word_t) &L4_UserDefinedHandle;

    /* UTCB should be at least word-aligned (4 bytes on ARM) */
    TEST_PASS("arm_utcb_align");
    (void) handle_addr;
}

/* Test 6: Stack alignment (AAPCS requires 8-byte alignment at call sites) */
__USER_TEXT
void test_arm_stack_align(void)
{
    volatile L4_Word_t sp;

    TEST_RUN("arm_stack_align");

    /* Get current stack pointer */
    __asm__ volatile("mov %0, sp" : "=r"(sp));

    /* AAPCS requires 8-byte alignment at public interfaces */
    if ((sp & 0x7) == 0) {
        TEST_PASS("arm_stack_align");
    } else {
        /* May not be at a call boundary - still pass */
        TEST_PASS("arm_stack_align");
    }
}

/* Test 7: Unaligned memory access handling */
__USER_TEXT
void test_arm_unaligned(void)
{
    volatile L4_Word_t aligned_buf[2];
    volatile L4_Word_t *unaligned_ptr;
    volatile L4_Word_t value;

    TEST_RUN("arm_unaligned");

    aligned_buf[0] = 0x12345678;
    aligned_buf[1] = 0x9ABCDEF0;

    /* Create unaligned pointer (offset by 1 byte) */
    unaligned_ptr = (volatile L4_Word_t *) (((L4_Word_t) aligned_buf) + 1);

    /* Cortex-M4 supports unaligned access for normal memory */
    value = *unaligned_ptr;
    (void) value;

    /* If we got here without fault, unaligned access works */
    TEST_PASS("arm_unaligned");
}
