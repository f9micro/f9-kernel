/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fpage.h>
#include <memory.h>
#include <debug.h>
#include <error.h>
#include <platform/irq.h>
#include <platform/mpu.h>

void mpu_setup_region(int n, fpage_t *fp)
{
	static uint32_t *mpu_base = (uint32_t *) MPU_BASE_ADDR;
	static uint32_t *mpu_attr = (uint32_t *) MPU_ATTR_ADDR;

	*mpu_base = (FPAGE_BASE(fp) & MPU_REGION_MASK) | 0x10 | (n & 0xF);
	*mpu_attr = ((mempool_getbyid(fp->fpage.mpid)->flags & MP_UX) ?
			0 :
			(1 << 28)) |	/* XN bit */
			(0x3 << 24) /* Full access */ |
			((fp->fpage.shift - 1) << 1) /* Region */ |
			1 /* Enable */;
}

void mpu_enable(mpu_state_t i)
{
	static uint32_t *mpu_ctrl = (uint32_t*) MPU_CTRL_ADDR;

	*mpu_ctrl = i | MPU_PRIVDEFENA;
}

int mpu_select_lru(as_t *as, uint32_t addr)
{
	fpage_t *fp = NULL;

	/* Kernel fault? */
	if (as == NULL)
		return 1;

	fp = as->first;

	/* No need to setup mpu here
	 * because it will be done when context switches
	 */

	while (fp) {
		if (addr_in_fpage(addr, fp)) {
			fp->mpu_next = as->mpu_first;
			as->mpu_first = fp;

			/* Remove circular link */
			while (fp->mpu_next != NULL && fp->mpu_next != as->mpu_first) {
				fp = fp->mpu_next;
			}
			fp->mpu_next = NULL;

			return 0;
		}

		fp = fp->as_next;
	}

	return 1;
}

void mpu_dump(void)
{
	int i = 0;
	uint32_t *mpu_rnr = (uint32_t *) MPU_RNR_ADDR;
	uint32_t *mpu_base = (uint32_t *) MPU_BASE_ADDR;
	uint32_t *mpu_attr = (uint32_t *) MPU_ATTR_ADDR;

	dbg_printf(DL_EMERG, "-------MPU------\n");
	for (i = 0; i < 8; i++) {
		*mpu_rnr = i;
		if (*mpu_attr & 0x1) {
			dbg_printf(DL_EMERG,
				"b:%p, sz:2**%d, attr:%04x\n", *mpu_base & MPU_REGION_MASK,
				((*mpu_attr & 0x3E) >> 1) + 1, *mpu_attr >> 16);
		}
	}
}

void __memmanage_handler(void)
{
	uint32_t mmsr = *((uint32_t *) MPU_FAULT_STATUS_ADDR);
	uint32_t mmar = *((uint32_t *) MPU_FAULT_ADDRESS_ADDR);
	tcb_t *current = thread_current();

	if (mmsr & MPU_MEM_FAULT) {
		if (mpu_select_lru(current->as, mmar) == 0)
			goto ok;
	}

	/* stack errors */
	if (mmsr & MPU_MSTKERR) {
		/* Processor is not writing mmar, so we do it manually */
		if (mpu_select_lru(current->as, (uint32_t)PSP()) == 0)
			goto ok;
	}

	/* unstacking errors */
	if (mmsr & MPU_MUSTKERR) {
		/* Processor is not writing mmar, so we do it manually */
		if (mpu_select_lru(current->as, (uint32_t)PSP() + 32) == 0) {
			goto ok;
		}
	}

	if (mmsr & MPU_IACCVIOL) {
		if (mpu_select_lru(current->as, PSP()[REG_PC]) == 0)
			goto ok;
	}

	mpu_dump();
	panic("Memory fault mmsr:%p, mmar:%p,\n"
		"             current:%t, psp:%p, pc:%p\n",
		mmsr, mmar, current->t_globalid, PSP(), PSP()[REG_PC]);

ok:
	/* Clean status register */
	*((uint32_t *) MPU_FAULT_STATUS_ADDR) = mmsr;
	assert(*(volatile uint32_t *) MPU_FAULT_STATUS_ADDR == 0);
	return;
}

IRQ_HANDLER(memmanage_handler, __memmanage_handler);
