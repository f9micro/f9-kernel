/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 * @file scb_mock.c
 * @brief Minimal SCB mock for test_arm_unaligned in QEMU
 *
 * Provides minimal SCB register simulation to enable test_arm_unaligned.
 * Returns SCB_CCR = 0x200 (STKALIGN=1, UNALIGN_TRP=0).
 *
 * This follows the same pattern as mpu_mock.c, providing just enough
 * functionality for the test to validate register accessibility.
 */

#include <platform/link.h>
#include <stdint.h>

#ifdef CONFIG_SCB_MOCK

/* SCB register addresses */
#define SCB_CCR_ADDR 0xE000ED14

/**
 * @brief Read SCB register (minimal mock)
 *
 * Returns SCB_CCR = 0x200 for test_arm_unaligned:
 * - Bit 9 (STKALIGN): 1 = 8-byte stack alignment on exception entry
 * - Bit 3 (UNALIGN_TRP): 0 = unaligned access does not trap
 *
 * This matches typical Cortex-M4 configuration with stack alignment enabled
 * and unaligned trapping disabled.
 *
 * All other addresses return 0.
 */
__USER_TEXT
uint32_t scb_mock_read32(uintptr_t addr)
{
    return (addr == SCB_CCR_ADDR) ? 0x00000200 : 0;
}

/**
 * @brief Write SCB register (no-op)
 *
 * Not used by current tests. No-op for interface compatibility.
 */
__USER_TEXT
void scb_mock_write32(uintptr_t addr, uint32_t value)
{
    (void) addr;
    (void) value;
}

/**
 * @brief Initialize SCB mock (no-op)
 *
 * No state to initialize. Exists for interface compatibility.
 */
__USER_TEXT
void scb_mock_init(void)
{
    /* No state to initialize */
}

#endif /* CONFIG_SCB_MOCK */
