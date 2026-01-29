/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_IRQ_LATENCY_H_
#define PLATFORM_IRQ_LATENCY_H_

#include <stdint.h>

/* DWT (Data Watchpoint and Trace) registers for cycle counting */
#define DWT_CTRL ((volatile uint32_t *) 0xE0001000)
#define DWT_CYCCNT ((volatile uint32_t *) 0xE0001004)
#define DWT_CTRL_CYCCNTENA (1 << 0)

#define DEMCR ((volatile uint32_t *) 0xE000EDFC)
#define DEMCR_TRCENA (1 << 24)

/**
 * @file irq_latency.h
 * @brief Interrupt latency measurement and profiling infrastructure
 *
 * Provides cycle-accurate latency tracking for zero-latency ISRs and
 * standard IRQs. Enables validation of BASEPRI-based zero-latency
 * interrupt performance (<10 cycle target).
 *
 * Usage:
 *   1. Call latency_sample_start() at ISR entry
 *   2. Call latency_sample_end(priority, irq_num) at ISR exit
 *   3. View statistics via KDB 'L' command
 */

/**
 * Latency statistics per interrupt priority level.
 */
typedef struct {
    uint32_t count; /* Number of samples */
    uint32_t min;   /* Minimum latency (cycles) */
    uint32_t max;   /* Maximum latency (cycles) */
    uint32_t sum;   /* Sum for average calculation */
    uint32_t avg;   /* Average latency (cycles) */
} latency_stats_t;

/**
 * Get current cycle count from DWT_CYCCNT.
 * Returns 0 if DWT is not enabled.
 */
static inline uint32_t get_cycle_count(void)
{
    return *DWT_CYCCNT;
}

/**
 * Enable DWT cycle counter for latency measurements.
 * Called during system initialization.
 */
void latency_init(void);

/**
 * Record latency sample for an interrupt.
 *
 * @param priority Interrupt priority (0x0-0xF)
 * @param irq_num IRQ number (-15 to 239)
 * @param cycles Measured latency in cycles
 */
void latency_record(uint8_t priority, int16_t irq_num, uint32_t cycles);

/**
 * Get latency statistics for a priority level.
 *
 * @param priority Interrupt priority (0x0-0xF)
 * @return Pointer to statistics structure
 */
const latency_stats_t *latency_get_stats(uint8_t priority);

/**
 * Get a best-effort atomic snapshot of latency statistics.
 *
 * Uses relaxed atomics only; intended for diagnostic reads outside ISR
 * context. Returns 1 on success, 0 on invalid input.
 */
int latency_get_stats_snapshot(uint8_t priority, latency_stats_t *out);

/**
 * Reset all latency statistics.
 */
void latency_reset(void);

/**
 * Get interrupt number from IPSR.
 * Returns 0 for thread mode, 1-15 for exceptions, 16+ for IRQs.
 */
static inline uint32_t get_irq_number(void)
{
    uint32_t ipsr;
    __asm__ __volatile__("mrs %0, ipsr" : "=r"(ipsr));
    return ipsr & 0x1FF;
}

/**
 * Latency measurement helper - call at ISR entry.
 * Returns timestamp for latency_sample_end().
 */
static inline uint32_t latency_sample_start(void)
{
    return get_cycle_count();
}

/**
 * Latency measurement helper - call at ISR exit.
 *
 * @param start_cycles Timestamp from latency_sample_start()
 * @param priority Interrupt priority level
 * @param irq_num IRQ number from IPSR
 */
static inline void latency_sample_end(uint32_t start_cycles,
                                      uint8_t priority,
                                      int16_t irq_num)
{
    uint32_t end_cycles = get_cycle_count();
    uint32_t elapsed = end_cycles - start_cycles;
    latency_record(priority, irq_num, elapsed);
}

#endif /* PLATFORM_IRQ_LATENCY_H_ */
