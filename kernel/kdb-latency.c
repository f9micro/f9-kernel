/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <debug.h>
#include <platform/irq-latency.h>

/**
 * KDB command: Display interrupt latency statistics.
 *
 * Shows min/avg/max latency for each priority level, highlighting
 * zero-latency ISRs (0x0-0x2) and standard user IRQs.
 */
void kdb_show_latency(void)
{
    int i;
    latency_stats_t stats;
    int has_data = 0;

    dbg_printf(DL_KDB, "\n=== Interrupt Latency Statistics ===\n");
    dbg_printf(DL_KDB, "Prio  Type              Count    Min    Avg    Max\n");
    dbg_printf(DL_KDB, "----  ----------------  ------  -----  -----  -----\n");

    for (i = 0; i < 16; i++) {
        if (!latency_get_stats_snapshot(i, &stats))
            continue;

        if (stats.count == 0)
            continue;

        has_data = 1;

        const char *type;
        if (i <= 0x2)
            type = "Zero-latency ISR";
        else if (i == 0x3)
            type = "SysTick";
        else if (i <= 0xE)
            type = "User IRQ";
        else
            type = "SVCall/PendSV";

        stats.avg = stats.count > 0 ? (stats.sum / stats.count) : 0;
        dbg_printf(DL_KDB, "0x%X   %-16s  %6u  %5u  %5u  %5u\n", i, type,
                   stats.count, stats.min, stats.avg, stats.max);
    }

    if (!has_data) {
        dbg_printf(DL_KDB, "(No latency samples recorded yet)\n");
    }

    dbg_printf(DL_KDB, "\nNotes:\n");
    dbg_printf(DL_KDB, "  - Zero-latency ISRs (0x0-0x2) target <10 cycles\n");
    dbg_printf(DL_KDB, "  - User IRQs (0x4-0xE) masked during kernel ops\n");
    dbg_printf(DL_KDB, "  - Use 'r' to reset statistics\n");
    dbg_printf(DL_KDB, "\n");
}

/**
 * KDB command: Reset latency statistics.
 */
void kdb_reset_latency(void)
{
    latency_reset();
    dbg_printf(DL_KDB, "Latency statistics reset.\n");
}
