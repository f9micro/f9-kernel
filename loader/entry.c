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
#include <platform/link.h>
#include <error.h>
#include <debug.h>
#include <types.h>
#include <debug.h>
#include <lib/string.h>

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

void __loader_start(void);

void nointerrupt(void)
{
	while (1)
		/* wait */ ;
}

void dummy_handler(void)
{
	return;
}

__ISR_VECTOR
void (* const g_pfnVectors[])(void) = {
	/* Core Level - ARM Cortex-M */
	(void *) &stack_end,	/* initial stack pointer */
	__loader_start,		/* reset handler */
	nointerrupt,		/* NMI handler */
	nointerrupt,		/* hard fault handler */
	nointerrupt,		/* MPU fault handler */
	nointerrupt,		/* bus fault handler */
	nointerrupt,		/* usage fault handler */
	0,			/* Reserved */
	0,			/* Reserved */
	0,			/* Reserved */
	0,			/* Reserved */
	nointerrupt,		/* SVCall handler */
	nointerrupt,		/* Debug monitor handler */
	0,			/* Reserved */
	dummy_handler,		/* PendSV handler */
	dummy_handler, 		/* SysTick handler */
	/* Chip Level: vendor specific */
	/* FIXME: use better IRQ vector generator */
#include INC_PLAT(nvic_table.h)
};

int main(void)
{
	void (*kernel_entry)(void);

	irq_disable();
	irq_init();
	irq_enable();

	dbg_layer = DL_BASIC;
	dbg_uart_init();

	dbg_printf(DL_BASIC, "loading kernel ...\n");

	dbg_printf(DL_BASIC, "ELF check result = %d\n",
	           elf_checkFile((void *) 0x800c000));

	kernel_entry = (void (*)(void))(elf_loadFile((void *) 0x800c000));
	irq_disable();

	memory_remap_sram();

	kernel_entry();

	while (1)
		;
	/* Not reached */
	return 0;
}

void __loader_start(void)
{
	/* Copy data segments */
	memcpy(&data_start, &kernel_flash_start + (&kernel_end - &kernel_start),
	       (&data_end - &data_start) * sizeof(uint32_t));
	/* DATA (ROM) -> DATA (RAM) */

	/* Fill bss with zeroes */
	memset(&bss_start, 0, (&bss_end - &bss_start) * sizeof(uint32_t));

	sys_clock_init();

	/* entry point */
	main();
}
