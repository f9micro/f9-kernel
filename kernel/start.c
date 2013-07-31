/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * __l4_start initializes microcontroller
 */

#include INC_PLAT(gpio.h)
#include INC_PLAT(rcc.h)

#include <platform/debug_uart.h>
#include <platform/irq.h>
#include <error.h>
#include <types.h>
#include <debug.h>
#include <kdb.h>
#include <ipc.h>
#include <ktimer.h>
#include <softirq.h>
#include <syscall.h>
#include <systhread.h>
#include <kprobes.h>

#ifdef CONFIG_KDB
void debug_kdb_handler()
{
	kdb_handler(dbg_getchar());
}
#endif

int main()
{

	irq_init();
	irq_disable();

	dbg_uart_init();
	dbg_puts("\n\n---------------------------------------"
			 "\nF9 microkernel is ready!\n");
#ifdef DEBUG
	extern dbg_layer_t dbg_layer;
	dbg_layer = DL_KDB;
#endif


	sched_init();
	memory_init();
	syscall_init();
	thread_init_subsys();
	ktimer_event_init();

#ifdef CONFIG_KPROBES
	kprobe_init();
#endif /* CONFIG_KPROBES */

#ifdef CONFIG_KDB
	softirq_register(KDB_SOFTIRQ, debug_kdb_handler);
	dbg_puts("Press '?' to print KDB menu\n");
#endif
	/* Not creating kernel thread here because it corrupts current stack
	 */
	create_idle_thread();
	create_root_thread();

	ktimer_event_create(64, ipc_deliver, NULL);

	mpu_enable(MPU_ENABLED);

	switch_to_kernel();

	/* Not reached */
	return 0;
}

static void init_zero_seg(uint32_t *dst, uint32_t *dst_end)
{
	while (dst < dst_end) {
		*dst++ = 0;
	}
}

static void init_copy_seg(uint32_t *src, uint32_t *dst, uint32_t *dst_end)
{
	while (dst < dst_end)
		*dst++ = *src++;
}

void __l4_start()
{
	/* Copy data segments */

	init_copy_seg(&kernel_text_end,
			&kernel_data_start, &kernel_data_end);
			/* DATA (ROM) -> DATA (RAM) */
	init_copy_seg(&user_text_end,
			&user_data_start, &user_data_end);
			/* USER DATA (ROM) -> USER DATA (RAM) */

	/* Fill bss with zeroes */
	init_zero_seg(&bss_start, &bss_end);
	init_zero_seg(&kernel_ahb_start, &kernel_ahb_end);
	init_zero_seg(&user_bss_start, &user_bss_end);

	sys_clock_init();

	/* entry point */
	main();
}
