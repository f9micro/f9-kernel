/* Copyright (c) 2013, 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * __l4_start initializes microcontroller
 */

#include INC_PLAT(gpio.h)
#include INC_PLAT(rcc.h)

#include <platform/debug_device.h>
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
#include <ksym.h>
#include <init_hook.h>
#include <lib/stdio.h>
#include <lib/string.h>

static char banner[] =
	"\n"
	"====================================================\n"
	" Copyright(C) 2013-2014 The F9 Microkernel Project  \n"
	"====================================================\n"
	"Git head: " GIT_HEAD "\n"
	"Host: " MACH_TYPE "\n"
	"Build: "  BUILD_TIME "\n"
	"\n";

int main(void)
{
	run_init_hook(INIT_LEVEL_PLATFORM_EARLY);
	irq_init();
	irq_disable();

#ifdef CONFIG_FPU
	*SCB_CPACR |= (SCB_CPACR_CP10_FULL | SCB_CPACR_CP11_FULL);
#endif

	run_init_hook(INIT_LEVEL_PLATFORM);

	__l4_printf("%s", banner);

	run_init_hook(INIT_LEVEL_KERNEL_EARLY);

	run_init_hook(INIT_LEVEL_KERNEL);

	/* Not creating kernel thread here because it corrupts current stack
	 */
	create_idle_thread();
	create_root_thread();

	ktimer_event_create(64, ipc_deliver, NULL);

	mpu_enable(MPU_ENABLED);

	run_init_hook(INIT_LEVEL_LAST);

	switch_to_kernel();

	/* Not reached */
	return 0;
}

void __l4_start(void)
{
	run_init_hook(INIT_LEVEL_EARLIEST);

#ifndef CONFIG_LOADER
	/* Copy data segments */
	memcpy(&kernel_data_start, &kernel_text_end,
	       (&kernel_data_end - &kernel_data_start) * sizeof(uint32_t));
	/* DATA (ROM) -> DATA (RAM) */
	memcpy(&user_text_start, &user_text_flash_start,
	       (&user_text_end - &user_text_start) * sizeof(uint32_t));
	/* USER TEXT (ROM) -> USER TEXT (RAM) */
	memcpy(&user_data_start, &user_text_flash_end,
	       (&user_data_end - &user_data_start) * sizeof(uint32_t));
	/* USER DATA (ROM) -> USER DATA (RAM) */
#endif

	/* Fill bss with zeroes */
	memset(&bss_start, 0,
	       (&bss_end - &bss_start) * sizeof(uint32_t));
	memset(&kernel_ahb_start, 0,
	       (&bss_end - &bss_start) * sizeof(uint32_t));
	memset(&user_bss_start, 0,
	       (&user_bss_end - & user_bss_start) * sizeof(uint32_t));

	sys_clock_init();

	/* entry point */
	main();
}
