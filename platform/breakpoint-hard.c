/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if !defined(CONFIG_KPROBES)
#error __FILE__ " is the part of KProbes implementation."
#endif

#include <debug.h>
#include <platform/breakpoint-hard.h>
#include <platform/breakpoint.h>
#include <platform/cortex_m.h>
#include <platform/hw_debug.h>
#include <types.h>

#define HW_BKPT_NULL_ID FPB_MAX_COMP
#define IS_UPPER_HALFWORLD(addr) (addr & 0x2)

/* Runtime FPB availability flag.
 * QEMU netduinoplus2 does not emulate FPB hardware.
 * Set to 1 if FPB is detected, 0 otherwise.
 */
static int fpb_available = 0;

/* Runtime FPB comparator count detected from hardware.
 * Capped at FPB_MAX_COMP to prevent array overflow.
 */
static int fpb_num_comp = 0;

static int hard_breakpoints[FPB_MAX_COMP];

static void hard_breakpoint_enable(struct breakpoint *b);
static void hard_breakpoint_disable(struct breakpoint *b);
static void hard_breakpoint_release(struct breakpoint *b);

static int get_avail_hard_breakpoint(void)
{
    int i;
    /* Use runtime-detected comparator count */
    for (i = 0; i < fpb_num_comp; i++) {
        if (hard_breakpoints[i] == -1)
            return i;
    }
    return -1;
}

/*
 * Detect FPB hardware availability and count comparators.
 * Returns number of code comparators if FPB is present, 0 otherwise.
 *
 * Detection method: Write enable bit to FPB_CTRL and verify it sticks.
 * On QEMU without FPB emulation, the write has no effect.
 *
 * ARMv7-M FP_CTRL register format:
 *   Bits [7:4]:   NUM_CODE1 - Number of code comparators, bits [3:0]
 *   Bits [14:12]: NUM_CODE2 - Number of code comparators, bits [6:4]
 *   Total NUM_CODE = (NUM_CODE2 << 4) | NUM_CODE1
 */
static int detect_fpb_hardware(void)
{
    uint32_t ctrl_val;
    int num_code;

    /* Try to enable FPB */
    *FPB_CTRL = FPB_CTRL_KEY | FPB_CTRL_ENABLE;

    /* Read back and check if enable bit is set */
    ctrl_val = *FPB_CTRL;

    if (!(ctrl_val & FPB_CTRL_ENABLE))
        return 0;

    /* Extract NUM_CODE from FP_CTRL per ARMv7-M spec:
     * NUM_CODE1 (bits [7:4]) + NUM_CODE2 (bits [14:12]) shifted
     */
    num_code = ((ctrl_val >> 4) & 0xF) | (((ctrl_val >> 12) & 0x7) << 4);

    /* Cap at FPB_MAX_COMP to prevent array overflow */
    if (num_code > FPB_MAX_COMP)
        num_code = FPB_MAX_COMP;

    return num_code;
}

void hard_breakpoint_pool_init(void)
{
    int i;

    /* Enable DebugMon exception - required for BOTH hardware and software
     * breakpoints. Without this, BKPT instructions cause HardFault.
     * Must be done before early return for FPB-absent case.
     */
    *DCB_DEMCR |= DCB_DEMCR_TRCENA | DCB_DEMCR_MON_EN;

    /* Clear debug status bits */
    *SCB_HFSR = SCB_HFSR_DEBUGEVT;
    *SCB_DFSR = SCB_DFSR_BKPT;
    *SCB_DFSR = SCB_DFSR_HALTED;
    *SCB_DFSR = SCB_DFSR_DWTTRAP;

    /* Detect FPB hardware and get comparator count */
    fpb_num_comp = detect_fpb_hardware();
    fpb_available = (fpb_num_comp > 0);

    if (!fpb_available) {
        dbg_printf(DL_KDB,
                   "FPB: Hardware not available (QEMU?), "
                   "using software breakpoints only\n");
        return;
    }

    dbg_printf(DL_KDB, "FPB: Hardware detected, %d comparators\n",
               fpb_num_comp);

    /* Initialize only the detected number of comparators */
    for (i = 0; i < fpb_num_comp; i++) {
        hard_breakpoints[i] = -1;
        *(FPB_COMP + i) = 0;
    }

    enable_all_hard_breakpoints();

    /* Memory barriers to ensure FPB config is visible before use.
     * ARM recommends DSB+ISB after changing breakpoint state.
     */
    __DSB();
    __ISB();
}

struct breakpoint *hard_breakpoint_config(uint32_t addr, struct breakpoint *b)
{
    /* If FPB hardware not available, fall back to soft breakpoints */
    if (!fpb_available)
        return NULL;

    if (breakpoint_type_by_addr(addr) == BKPT_HARD) {
        int _hard_breakpoint_id = get_avail_hard_breakpoint();
        int breakpoint_id = get_breakpoint_id(b);

        if (breakpoint_id >= 0 && _hard_breakpoint_id >= 0) {
            hard_breakpoints[_hard_breakpoint_id] = breakpoint_id;
            b->type = BKPT_HARD;
            b->addr = addr;
            b->hard_breakpoint_id = _hard_breakpoint_id;
            b->enable = hard_breakpoint_enable;
            b->disable = hard_breakpoint_disable;
            b->release = hard_breakpoint_release;
            return b;
        }
    }

    return NULL;
}

static void hard_breakpoint_enable(struct breakpoint *b)
{
    uint32_t addr = b->addr;

    if (IS_UPPER_HALFWORLD(addr)) {
        *(FPB_COMP + b->hard_breakpoint_id) = FPB_COMP_REPLACE_UPPER |
                                              (addr & FPB_COMP_ADDR_MASK) |
                                              FPB_COMP_ENABLE;
    } else {
        *(FPB_COMP + b->hard_breakpoint_id) = FPB_COMP_REPLACE_LOWER |
                                              (addr & FPB_COMP_ADDR_MASK) |
                                              FPB_COMP_ENABLE;
    }

    /* Ensure breakpoint is active before continuing execution */
    __DSB();
    __ISB();
}

static void hard_breakpoint_disable(struct breakpoint *b)
{
    *(FPB_COMP + b->hard_breakpoint_id) &= ~FPB_COMP_ENABLE;

    /* Ensure breakpoint is disabled before continuing */
    __DSB();
    __ISB();
}

static void hard_breakpoint_release(struct breakpoint *b)
{
    *(FPB_COMP + b->hard_breakpoint_id) &= ~FPB_COMP_ENABLE;
    hard_breakpoints[b->hard_breakpoint_id] = -1;
    b->type = BKPT_NONE;

    /* Ensure breakpoint is released before continuing */
    __DSB();
    __ISB();
}
