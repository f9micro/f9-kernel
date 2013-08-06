/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fpage.h>
#include <memory.h>
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
			as->lru = fp;
			return 0;
		}

		fp = fp->as_next;
	}

	return 1;
}

void __memmanage_handler(void)
{
	uint32_t mmsr = *((uint32_t *) MPU_FAULT_STATUS_ADDR);
	uint32_t mmar = *((uint32_t *) MPU_FAULT_ADDRESS_ADDR);
	tcb_t *current = thread_current();

	/* stack / unstacking errors */
	if (mmsr & MPU_MUSTKERR || mmsr & MPU_MSTKERR) {
		/* Processor is not writing mmar, so we do it manually */
		if (mpu_select_lru(current->as, current->ctx.sp) == 0)
			goto ok;
	}

	if (mmsr & MPU_MEM_FAULT) {
		if (mpu_select_lru(current->as, mmar) == 0)
			goto ok;
	}

	panic("Memory fault\n");

ok:
	/* Clean status register */
	*((uint32_t *) MPU_FAULT_STATUS_ADDR) = 0;
	return;
}

IRQ_HANDLER(memmanage_handler, __memmanage_handler);
