/* Copyright (c) 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <debug.h>
#include <error.h>
#include <fpage.h>
#include <memory.h>
#include <platform/cortex_m.h>
#include <platform/irq.h>
#include <platform/mpu.h>

#include INC_PLAT(mpu.c)

void mpu_setup_region(int n, fpage_t *fp)
{
    static uint32_t *mpu_base = (uint32_t *) MPU_BASE_ADDR;
    static uint32_t *mpu_attr = (uint32_t *) MPU_ATTR_ADDR;

    if (fp) {
        *mpu_base = (FPAGE_BASE(fp) & MPU_REGION_MASK) | 0x10 | (n & 0xF);
        *mpu_attr = ((mempool_getbyid(fp->fpage.mpid)->flags & MP_UX)
                         ? 0
                         : (1 << 28)) | /* XN bit */
                    (0x3 << 24)         /* Full access */
                    | ((fp->fpage.shift - 1) << 1) /* Region size*/ |
                    1 /* Enable */;
    } else {
        /* Clean MPU region */
        *mpu_base = 0x10 | (n & 0xF);
        *mpu_attr = 0;
    }

    /* Memory barriers ensure MPU changes take effect immediately */
    __DSB();
    __ISB();
}

void mpu_enable(mpu_state_t i)
{
    static uint32_t *mpu_ctrl = (uint32_t *) MPU_CTRL_ADDR;

    *mpu_ctrl = i | MPU_PRIVDEFENA;
}

int addr_in_mpu(uint32_t addr)
{
    static uint32_t *mpu_rnr = (uint32_t *) MPU_RNR_ADDR;
    static uint32_t *mpu_base = (uint32_t *) MPU_BASE_ADDR;
    static uint32_t *mpu_attr = (uint32_t *) MPU_ATTR_ADDR;
    int i;

    for (i = 0; i < 8; ++i) {
        *mpu_rnr = i;
        if (*mpu_attr & 0x1) {
            uint32_t base = *mpu_base & MPU_REGION_MASK;
            uint32_t size = 1 << (((*mpu_attr >> 1) & 0x1F) + 1);
            if (addr >= base && addr < base + size)
                return 1;
        }
    }

    return 0;
}

int mpu_select_lru(as_t *as, uint32_t addr)
{
    fpage_t *fp = NULL;
    int i;

    /* Kernel fault? */
    if (!as)
        return 1;

    if (addr_in_mpu(addr))
        return 1;

    fp = as->first;
    while (fp) {
        if (addr_in_fpage(addr, fp, 0)) {
            fpage_t *sfp = as->mpu_stack_first;

            /*
             * Fix from f9-riscv commit f88633c:
             * Remove fpage from list first to prevent circular list.
             * If fp is already in mpu_first list and we prepend without
             * removing, we create a cycle that causes infinite loops.
             */
            remove_fpage_from_list(as, fp, mpu_first, mpu_next);
            fp->mpu_next = as->mpu_first;
            as->mpu_first = fp;

            /* Get first avalible MPU index */
            i = 0;
            while (sfp) {
                ++i;
                sfp = sfp->mpu_next;
            }

            /* Update MPU */
            mpu_setup_region(i++, fp);

            while (i < 8 && fp->mpu_next) {
                mpu_setup_region(i++, fp->mpu_next);
                fp = fp->mpu_next;
            }

            return 0;
        }

        fp = fp->as_next;
    }

    return 1;
}

void mpu_dump(int print_title)
{
    int i = 0;
    uint32_t *mpu_rnr = (uint32_t *) MPU_RNR_ADDR;
    uint32_t *mpu_base = (uint32_t *) MPU_BASE_ADDR;
    uint32_t *mpu_attr = (uint32_t *) MPU_ATTR_ADDR;

    if (print_title)
        dbg_printf(DL_EMERG, "-------MPU------\n");
    for (i = 0; i < 8; i++) {
        *mpu_rnr = i;
        if (*mpu_attr & 0x1) {
            dbg_printf(DL_EMERG, "b:%p, sz:2**%d, attr:%04x\n",
                       *mpu_base & MPU_REGION_MASK,
                       ((*mpu_attr & 0x3E) >> 1) + 1, *mpu_attr >> 16);
        }
    }
}

#ifdef CONFIG_KDB
void kdb_dump_mpu(void)
{
    mpu_dump(0);
}
#endif

static void dump_as_fpages(as_t *as)
{
    fpage_t *fp = as ? as->first : NULL;
    int count = 0;

    dbg_printf(DL_EMERG, "---AS fpages---\n");
    while (fp && count < 16) {
        dbg_printf(DL_EMERG, "  fp[%d]: base:%p, sz:2**%d\n", count,
                   FPAGE_BASE(fp), fp->fpage.shift);
        fp = fp->as_next;
        count++;
    }
    if (fp)
        dbg_printf(DL_EMERG, "  ... more fpages\n");
}

void __memmanage_handler(void)
{
    uint32_t mmsr = *((uint32_t *) MPU_FAULT_STATUS_ADDR);
    uint32_t mmar = *((uint32_t *) MPU_FAULT_ADDRESS_ADDR);
    tcb_t *current = thread_current();
    int handled = 0;

    /* Try to handle the fault first before printing diagnostics */
    if (mmsr & MPU_MEM_FAULT) {
        if (mpu_select_lru(current->as, mmar) == 0)
            handled = 1;
    }

    if (mmsr & MPU_MUSTKERR) {
        if (mpu_select_lru(current->as, (uint32_t) PSP() + 31) == 0)
            handled = 1;
    }

    if (mmsr & MPU_IACCVIOL) {
        uint32_t pc = PSP()[REG_PC];
        if (mpu_select_lru(current->as, pc) == 0)
            handled = 1;
        else if (mpu_select_lru(current->as, pc + 2) == 0)
            handled = 1;
    }

    /* If handled successfully, just clear status and return silently */
    if (handled) {
        *((uint32_t *) MPU_FAULT_STATUS_ADDR) = mmsr;
        return;
    }

    /* Unhandled fault - show diagnostic info */
    dbg_printf(DL_EMERG, "MEMFAULT: tid:%t, as:%p (spaceid:%p)\n",
               current->t_globalid, current->as,
               current->as ? current->as->as_spaceid : 0);
    dbg_printf(DL_EMERG, "  mmsr:%p, mmar:%p, pc:%p, psp:%p\n", mmsr, mmar,
               PSP()[REG_PC], PSP());
    dbg_printf(DL_EMERG, "  flags: %s%s%s%s%s\n",
               (mmsr & MPU_MEM_FAULT) ? "MMARVALID " : "",
               (mmsr & MPU_DACCVIOL) ? "DACCVIOL " : "",
               (mmsr & MPU_IACCVIOL) ? "IACCVIOL " : "",
               (mmsr & MPU_MSTKERR) ? "MSTKERR " : "",
               (mmsr & MPU_MUSTKERR) ? "MUSTKERR " : "");
    dbg_printf(DL_EMERG, "  in_mpu(mmar)=%d, in_mpu(pc)=%d\n",
               addr_in_mpu(mmar), addr_in_mpu(PSP()[REG_PC]));

    /* stack errors - always fatal */
    if (mmsr & MPU_MSTKERR) {
        panic("Corrupted Stack, current = %t, psp = %p\n", current->t_globalid,
              PSP());
    }

    /* Unhandled fault - dump diagnostics and panic */
    dump_as_fpages(current->as);
    mpu_dump(1);
    panic(
        "Memory fault mmsr:%p, mmar:%p,\n"
        "             current:%t, psp:%p, pc:%p\n",
        mmsr, mmar, current->t_globalid, PSP(), PSP()[REG_PC]);
}

IRQ_HANDLER(memmanage_handler, __memmanage_handler);
