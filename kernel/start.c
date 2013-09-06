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
#include <ksym.h>
#include <init_hook.h>

static char banner[] = 
	"\n"
	"====================================================\n"
	"      Copyright(C) 2013 The F9 Microkernel Project  \n"
	"====================================================\n"
	"Git head: " GIT_HEAD "\n"
	"Host: " MACH_TYPE "\n"
	"Build: "  BUILD_TIME "\n"
	"\n";

#ifdef CONFIG_KDB
void debug_kdb_handler(void)
{
	kdb_handler(dbg_getchar());
}
#endif

#ifdef DEBUG
extern dbg_layer_t dbg_layer;
#endif

int main(void)
{
	irq_init();
	irq_disable();

#ifdef CONFIG_FPU
	*SCB_CPACR |= (SCB_CPACR_CP10_FULL | SCB_CPACR_CP11_FULL);
#endif

	dbg_uart_init();
	dbg_printf(DL_EMERG, "%s", banner);
#ifdef DEBUG
	dbg_layer = DL_KDB;
#endif
	init_hook(INIT_LEVEL_PLATFORM);

#ifdef CONFIG_SYMMAP
	ksym_init();
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
	init_hook(INIT_LEVEL_KERNEL);

	/* Not creating kernel thread here because it corrupts current stack
	 */
	create_idle_thread();
	create_root_thread();

	ktimer_event_create(64, ipc_deliver, NULL);

	mpu_enable(MPU_ENABLED);

	init_hook(INIT_LEVEL_LAST);

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

void __l4_start(void)
{
	init_hook(INIT_LEVEL_EARLIEST);

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

#ifdef TEST_INIT_HOOK
void hook_test1(unsigned int level)
{
	dbg_printf(DL_EMERG, "hook 1 level: %x\n", level);
}
INIT_HOOK(test1, hook_test1, INIT_LEVEL_PLATFORM - 1)

void hook_test2(unsigned int level)
{
	dbg_printf(DL_EMERG, "hook 2 level: %x\n", level);
}
INIT_HOOK(test2,hook_test2, INIT_LEVEL_KERNEL - 1)

void hook_test3(unsigned int level)
{
	dbg_printf(DL_EMERG, "hook 3 level: %x\n", level);
}
INIT_HOOK(test3, hook_test3, INIT_LEVEL_LAST - 1)
#endif	/* TEST_INIT_HOOK */
