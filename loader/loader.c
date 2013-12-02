/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include INC_PLAT(gpio.h)
#include INC_PLAT(rcc.h)
#include INC_PLAT(syscfg.h)
#include INC_PLAT(systick.h)
#include INC_PLAT(nvic.h)

#include <platform/debug_uart.h>
#include <platform/irq.h>
#include <error.h>
#include <debug.h>
#include <types.h>
#include <debug.h>

#include <elf/elf.h>

/* linker variables */
extern uint32_t kernel_flash_start;
extern uint32_t kernel_start;
extern uint32_t kernel_end;
extern uint32_t data_start;
extern uint32_t data_end;
extern uint32_t bss_start;
extern uint32_t bss_end;


extern dbg_layer_t dbg_layer;

int main(void)
{
	void (*kernel_entry)(void);

	irq_disable();
	irq_init();
	irq_enable();

	dbg_layer = DL_BASIC;
	dbg_uart_init();
	
	dbg_printf(DL_BASIC, "loading kernel ...\n");

	dbg_printf(DL_BASIC, "elf check result = %d\n",
		elf_checkFile((void*)0x800c000));

	kernel_entry = (void (*)(void))(elf_loadFile((void*)0x800c000));
	irq_disable();

	memory_remap_sram();
	
	kernel_entry();
	
	//switch_to_kernel();
	while(1);

	/* Not reached */
	return 0;
}

static inline void init_zero_seg(uint32_t *dst, uint32_t *dst_end)
{
	while (dst < dst_end)
		*dst++ = 0;
}

static inline void init_copy_seg(uint32_t *src, uint32_t *dst, uint32_t *dst_end)
{
	while (dst < dst_end)
		*dst++ = *src++;
}

void __loader_start(void)
{
	/* Copy data segments */
	init_copy_seg(&kernel_flash_start + (&kernel_end - &kernel_start),
		&data_start, &data_end);
	/* DATA (ROM) -> DATA (RAM) */

	/* Fill bss with zeroes */
	init_zero_seg(&bss_start, &bss_end);

	sys_clock_init();

	/* entry point */
	main();
}
