/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __PLATFORM_SAFE_MMIO_H__
#define __PLATFORM_SAFE_MMIO_H__

/*
 * Safe MMIO Access Macros
 *
 * Purpose: Explicit handling of Memory-Mapped I/O (MMIO) accesses that may
 * hang or fail in emulation environments like QEMU.
 *
 * Background:
 * - QEMU's netduinoplus2 machine does not emulate all ARM System Control Space
 *   (SCS) registers (0xE000E000 - 0xE000FFFF)
 * - Direct MMIO access via LDR/STR instructions to these addresses hangs QEMU
 * - CPU instructions (MRS/MSR) and software variables are safe
 *
 * Pattern discovered:
 * ✅ Safe: CPU instructions (MRS, MSR), software variables
 * ❌ Unsafe: MMIO to SCS/PPB region (0xE000Exxx) on QEMU netduinoplus2
 *
 * Usage:
 * Instead of:  uint32_t val = *(volatile uint32_t *)0xE000ED90;  // May hang
 * Use:         uint32_t val = SAFE_MMIO_READ32(0xE000ED90, 0);   // Explicit
 *
 * See also:
 * - board/Kconfig: CONFIG_HAS_* flags for feature availability
 * - claudedocs/qemu_test_skips.md: MMIO pattern documentation
 */

#include <stdint.h>

/* MPU register address range (for CONFIG_MPU_MOCK routing) */
#define MPU_TYPE_ADDR 0xE000ED90
#define MPU_CTRL_ADDR 0xE000ED94
#define MPU_RNR_ADDR 0xE000ED98
#define MPU_RBAR_ADDR 0xE000ED9C
#define MPU_RASR_ADDR 0xE000EDA0

/* SCB register addresses (for CONFIG_SCB_MOCK routing) */
#define SCB_CCR_ADDR 0xE000ED14

#ifdef CONFIG_MPU_MOCK
/* Forward declarations for MPU mock functions (defined in mpu_mock.c) */
uint32_t mpu_mock_read32(uintptr_t addr);
void mpu_mock_write32(uintptr_t addr, uint32_t value);
#endif

#ifdef CONFIG_SCB_MOCK
/* Forward declarations for SCB mock functions (defined in scb_mock.c) */
uint32_t scb_mock_read32(uintptr_t addr);
void scb_mock_write32(uintptr_t addr, uint32_t value);
#endif

/*
 * SAFE_MMIO_READ32 - Read 32-bit value from MMIO address
 *
 * @addr: Physical address of MMIO register
 * @fallback: Value to return when MMIO not available (QEMU builds)
 *
 * Returns: Register value on hardware, fallback value on QEMU
 *
 * Priority (highest to lowest):
 * 1. CONFIG_MPU_MOCK: Software mock for MPU registers (QEMU testing)
 * 2. CONFIG_SCB_MOCK: Software mock for SCB registers (QEMU testing)
 * 3. CONFIG_HAS_MPU: Real MPU hardware access
 * 4. CONFIG_HAS_SCB_ACCESS: Real SCB hardware access
 * 5. Fallback: Return default value (QEMU without mock)
 *
 * Example:
 *   uint32_t mpu_type = SAFE_MMIO_READ32(0xE000ED90, 0);
 *   uint32_t scb_ccr = SAFE_MMIO_READ32(0xE000ED14, 0);
 *   if (mpu_type == 0) {
 *       // QEMU or MPU not present
 *   }
 */
#if defined(CONFIG_MPU_MOCK) && defined(CONFIG_SCB_MOCK)
/* Both MPU and SCB mocks: Route to appropriate mock based on address */
#define SAFE_MMIO_READ32(addr, fallback)                  \
    (((addr) >= MPU_TYPE_ADDR && (addr) <= MPU_RASR_ADDR) \
         ? mpu_mock_read32(addr)                          \
     : ((addr) == SCB_CCR_ADDR) ? scb_mock_read32(addr)   \
                                : (fallback))
#elif defined(CONFIG_MPU_MOCK)
/* MPU mock only: Route MPU register access to software mock */
#define SAFE_MMIO_READ32(addr, fallback)                  \
    (((addr) >= MPU_TYPE_ADDR && (addr) <= MPU_RASR_ADDR) \
         ? mpu_mock_read32(addr)                          \
         : (fallback))
#elif defined(CONFIG_SCB_MOCK)
/* SCB mock only: Route SCB register access to software mock */
#define SAFE_MMIO_READ32(addr, fallback) \
    (((addr) == SCB_CCR_ADDR) ? scb_mock_read32(addr) : (fallback))
#elif defined(CONFIG_HAS_MPU)
/* MPU registers accessible - direct MMIO read */
#define SAFE_MMIO_READ32(addr, fallback) (*(volatile uint32_t *) (addr))
#elif defined(CONFIG_HAS_SCB_ACCESS)
/* SCB registers accessible - direct MMIO read */
#define SAFE_MMIO_READ32(addr, fallback) (*(volatile uint32_t *) (addr))
#else
/* MMIO not available (QEMU) - return fallback value */
#define SAFE_MMIO_READ32(addr, fallback) (fallback)
#endif

/*
 * SAFE_MMIO_WRITE32 - Write 32-bit value to MMIO address
 *
 * @addr: Physical address of MMIO register
 * @value: Value to write
 *
 * Priority (highest to lowest):
 * 1. CONFIG_MPU_MOCK: Software mock for MPU registers (QEMU testing)
 * 2. CONFIG_SCB_MOCK: Software mock for SCB registers (QEMU testing)
 * 3. CONFIG_HAS_MPU: Real MPU hardware access
 * 4. CONFIG_HAS_SCB_ACCESS: Real SCB hardware access
 * 5. No-op: QEMU without mock
 *
 * Example:
 *   SAFE_MMIO_WRITE32(0xE000ED90, 0x12345678);
 */
#if defined(CONFIG_MPU_MOCK) && defined(CONFIG_SCB_MOCK)
/* Both MPU and SCB mocks: Route to appropriate mock based on address */
#define SAFE_MMIO_WRITE32(addr, value)                            \
    do {                                                          \
        if ((addr) >= MPU_TYPE_ADDR && (addr) <= MPU_RASR_ADDR) { \
            mpu_mock_write32(addr, value);                        \
        } else if ((addr) == SCB_CCR_ADDR) {                      \
            scb_mock_write32(addr, value);                        \
        }                                                         \
    } while (0)
#elif defined(CONFIG_MPU_MOCK)
/* MPU mock only: Route MPU register writes to software mock */
#define SAFE_MMIO_WRITE32(addr, value)                            \
    do {                                                          \
        if ((addr) >= MPU_TYPE_ADDR && (addr) <= MPU_RASR_ADDR) { \
            mpu_mock_write32(addr, value);                        \
        }                                                         \
    } while (0)
#elif defined(CONFIG_SCB_MOCK)
/* SCB mock only: Route SCB register writes to software mock */
#define SAFE_MMIO_WRITE32(addr, value)     \
    do {                                   \
        if ((addr) == SCB_CCR_ADDR) {      \
            scb_mock_write32(addr, value); \
        }                                  \
    } while (0)
#elif defined(CONFIG_HAS_MPU)
#define SAFE_MMIO_WRITE32(addr, value)           \
    do {                                         \
        *(volatile uint32_t *) (addr) = (value); \
    } while (0)
#elif defined(CONFIG_HAS_SCB_ACCESS)
#define SAFE_MMIO_WRITE32(addr, value)           \
    do {                                         \
        *(volatile uint32_t *) (addr) = (value); \
    } while (0)
#else
/* MMIO not available (QEMU) - write is no-op */
#define SAFE_MMIO_WRITE32(addr, value) \
    do {                               \
        (void) (value);                \
    } while (0)
#endif

/*
 * SAFE_MMIO_MODIFY32 - Read-modify-write 32-bit MMIO register
 *
 * @addr: Physical address of MMIO register
 * @mask: Bits to clear (set bits will be cleared)
 * @value: Bits to set (set bits will be set)
 *
 * Example:
 *   // Set bit 3, clear bit 5
 *   SAFE_MMIO_MODIFY32(0xE000ED14, (1 << 5), (1 << 3));
 */
#if defined(CONFIG_HAS_MPU) || defined(CONFIG_HAS_SCB_ACCESS)
#define SAFE_MMIO_MODIFY32(addr, mask, value)         \
    do {                                              \
        uint32_t reg = *(volatile uint32_t *) (addr); \
        reg = (reg & ~(mask)) | (value);              \
        *(volatile uint32_t *) (addr) = reg;          \
    } while (0)
#else
/* MMIO not available (QEMU) - modify is no-op */
#define SAFE_MMIO_MODIFY32(addr, mask, value) \
    do {                                      \
        (void) (mask);                        \
        (void) (value);                       \
    } while (0)
#endif

/*
 * MMIO_IS_AVAILABLE - Check if MMIO access is available
 *
 * Returns: 1 if MMIO is available, 0 if running on QEMU or MMIO disabled
 *
 * Example:
 *   if (!MMIO_IS_AVAILABLE()) {
 *       printf("Skipping MMIO test - not available\n");
 *       return;
 *   }
 */
#if defined(CONFIG_MPU_MOCK) || defined(CONFIG_SCB_MOCK) || \
    defined(CONFIG_HAS_MPU) || defined(CONFIG_HAS_SCB_ACCESS)
#define MMIO_IS_AVAILABLE() 1
#else
#define MMIO_IS_AVAILABLE() 0
#endif

/*
 * Common SCS/PPB Register Addresses (for reference)
 *
 * These are the registers that hang QEMU netduinoplus2:
 * - MPU_TYPE:  0xE000ED90 (hangs - use CONFIG_HAS_MPU guard)
 * - SCB_CCR:   0xE000ED14 (hangs - use CONFIG_HAS_SCB_ACCESS guard)
 * - SCB_CPACR: 0xE000ED88 (hangs - use CONFIG_HAS_DWT guard for FPU tests)
 * - SYSTICK_*: 0xE000E010-0xE000E01C (hangs - use CONFIG_HAS_SYSTICK_TIMING)
 *
 * Safe alternatives:
 * - PSP: Use MRS instruction (read_psp() in test-arm.c)
 * - UTCB: Read software variable (__L4_TCR_MyLocalId())
 */

#endif /* __PLATFORM_SAFE_MMIO_H__ */
