/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <debug.h>
#include <init_hook.h>
#include <platform/cortex_m.h>
#include <platform/irq-latency.h>
#include <platform/irq.h>

/**
 * Latency statistics per priority level (0x0-0xF = 16 levels).
 */
static latency_stats_t latency_stats[16];

/**
 * Enable DWT cycle counter for latency measurements.
 *
 * Note: Called at INIT_LEVEL_PLATFORM with interrupts disabled.
 * Safe to reset statistics with plain writes (no atomic operations needed).
 */
void latency_init(void)
{
    uint32_t test_before, test_after;

    /* Enable DWT if not already enabled */
    if (!(*DEMCR & DEMCR_TRCENA)) {
        *DEMCR |= DEMCR_TRCENA; /* Enable trace and debug */
    }

    /* Enable cycle counter */
    *DWT_CTRL |= DWT_CTRL_CYCCNTENA;

    /* Reset cycle counter */
    *DWT_CYCCNT = 0;

    /* Verify DWT functionality (helpful diagnostic for QEMU vs hardware) */
    test_before = *DWT_CYCCNT;
    for (volatile int i = 0; i < 100; i++)
        ; /* Small busy loop */
    test_after = *DWT_CYCCNT;

    if (test_after == test_before) {
        /* QEMU: DWT not emulated, cycle counter stays at 0 */
        dbg_printf(DL_KDB,
                   "IRQ latency profiling enabled (DWT not available)\n");
    } else {
        /* Hardware: DWT working, show delta to confirm */
        dbg_printf(DL_KDB,
                   "IRQ latency profiling enabled (DWT cycle counter active, "
                   "test delta=%u)\n",
                   test_after - test_before);
    }

    /* Reset statistics - inline to avoid irq_save_flags during early boot */
    for (int i = 0; i < 16; i++) {
        latency_stats[i].count = 0;
        latency_stats[i].min = 0;
        latency_stats[i].max = 0;
        latency_stats[i].sum = 0;
        latency_stats[i].avg = 0;
    }
}

/*
 * Note: DWT cycle counter not emulated in QEMU (reads return 0).
 * On real hardware (STM32F4), DWT provides cycle-accurate latency measurement.
 * System boots safely in both environments - no hang.
 */
INIT_HOOK(latency_init, INIT_LEVEL_PLATFORM);

/**
 * Record latency sample for an interrupt.
 *
 * CRITICAL: This function is called from ISRs, including zero-latency ISRs
 * (priority 0x0-0x2). It MUST NOT use PRIMASK or any operation that blocks
 * zero-latency interrupts. Uses atomic operations for lock-free updates.
 *
 * Note: min/max updates use atomic compare-exchange loops to ensure
 * consistency even under heavy preemption from other zero-latency ISRs.
 */
void latency_record(uint8_t priority, int16_t irq_num, uint32_t cycles)
{
    latency_stats_t *stats;
    uint32_t old_min, old_max;

    (void) irq_num;

    /* Validate priority (0x0-0xF) */
    if (priority >= 16)
        return;

    /* Ignore obviously bogus samples (wraparound or >1M cycles).
     * 1M cycles at 168MHz is ~6ms, which is a reasonable upper bound
     * for most real-time ISRs.
     */
    if (cycles == 0 || cycles > 1000000)
        return;

    stats = &latency_stats[priority];

    /* Atomic updates for count and sum (lock-free, no PRIMASK). */
    __atomic_add_fetch(&stats->count, 1, __ATOMIC_RELAXED);
    __atomic_add_fetch(&stats->sum, cycles, __ATOMIC_RELAXED);

    /* Update min/max using atomic compare-exchange loops.
     * These ensure that even under heavy preemption, we never miss a new
     * min/max value.
     */

    /* Update min */
    old_min = __atomic_load_n(&stats->min, __ATOMIC_RELAXED);
    while ((old_min == 0 || cycles < old_min) &&
           !__atomic_compare_exchange_n(&stats->min, &old_min, cycles, 0,
                                        __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        /* If CAS failed, old_min was updated by another ISR; loop and retry */
    }

    /* Update max */
    old_max = __atomic_load_n(&stats->max, __ATOMIC_RELAXED);
    while (cycles > old_max &&
           !__atomic_compare_exchange_n(&stats->max, &old_max, cycles, 0,
                                        __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        /* If CAS failed, old_max was updated by another ISR; loop and retry */
    }
}

/**
 * Get latency statistics for a priority level.
 *
 * Calculates average lazily on read to avoid division in ISR hot path.
 */
const latency_stats_t *latency_get_stats(uint8_t priority)
{
    latency_stats_t *stats;

    if (priority >= 16)
        return NULL;

    stats = &latency_stats[priority];

    /* NOTE: This returns live stats. Callers must use atomic loads or
     * prefer latency_get_stats_snapshot() for a stable read.
     */
    return stats;
}

/**
 * Get a best-effort atomic snapshot of latency statistics for a priority.
 *
 * Uses only relaxed atomics (single-core). We retry if count changes during
 * the read. Because count and sum are updated separately, this provides a
 * consistent snapshot in the common case but is still best-effort.
 */
int latency_get_stats_snapshot(uint8_t priority, latency_stats_t *out)
{
    uint32_t count_before;
    uint32_t count_after;

    if (!out || priority >= 16)
        return 0;

    /*
     * Retry loop ensures all fields (count, sum, min, max) are from
     * the same snapshot generation. Reading count before and after
     * ensures no ISR updated the stats during our reads.
     */
    do {
        count_before =
            __atomic_load_n(&latency_stats[priority].count, __ATOMIC_RELAXED);
        out->sum =
            __atomic_load_n(&latency_stats[priority].sum, __ATOMIC_RELAXED);
        out->min =
            __atomic_load_n(&latency_stats[priority].min, __ATOMIC_RELAXED);
        out->max =
            __atomic_load_n(&latency_stats[priority].max, __ATOMIC_RELAXED);
        count_after =
            __atomic_load_n(&latency_stats[priority].count, __ATOMIC_RELAXED);
    } while (count_before != count_after);

    out->count = count_after;

    /* avg is computed by the caller to avoid shared writes. */
    out->avg = 0;

    return 1;
}

/**
 * Reset all latency statistics.
 */
void latency_reset(void)
{
    uint32_t flags;
    int i;

    flags = irq_save_flags();

    for (i = 0; i < 16; i++) {
        latency_stats[i].count = 0;
        latency_stats[i].min = 0;
        latency_stats[i].max = 0;
        latency_stats[i].sum = 0;
        latency_stats[i].avg = 0;
    }

    irq_restore_flags(flags);
}
