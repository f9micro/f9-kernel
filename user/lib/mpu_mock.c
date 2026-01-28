/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 * @file mpu_mock.c
 * @brief Minimal MPU mock for test_arm_mpu_config in QEMU
 *
 * Provides minimal MPU register simulation to enable test_arm_mpu_config.
 * Returns MPU_TYPE = 0x800 (8 regions, unified).
 *
 * Simplified from 307-line stateful implementation after analysis revealed
 * only single-register read was actually tested.
 */

#include <platform/link.h>
#include <platform/safe_mmio.h>
#include <stdint.h>

#ifdef CONFIG_MPU_MOCK

/* MPU register addresses */
#define MPU_TYPE_ADDR 0xE000ED90

/**
 * @brief Read MPU register (minimal mock)
 *
 * Returns MPU_TYPE = 0x800 (8 regions, unified) for test_arm_mpu_config.
 * All other addresses return 0.
 */
__USER_TEXT
uint32_t mpu_mock_read32(uintptr_t addr)
{
    return (addr == MPU_TYPE_ADDR) ? 0x00000800 : 0;
}

/**
 * @brief Write MPU register (no-op)
 *
 * Not used by current tests. No-op for interface compatibility.
 */
__USER_TEXT
void mpu_mock_write32(uintptr_t addr, uint32_t value)
{
    (void) addr;
    (void) value;
}

/**
 * @brief Initialize MPU mock (no-op)
 *
 * No state to initialize. Exists for interface compatibility.
 */
__USER_TEXT
void mpu_mock_init(void)
{
    /* No state to initialize */
}

#endif /* CONFIG_MPU_MOCK */
