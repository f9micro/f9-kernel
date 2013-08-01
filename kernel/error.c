/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <error.h>
#include <thread.h>
#include <debug.h>
#include <kdb.h>
#include <platform/irq.h>
#include <platform/armv7m.h>
#include <platform/link.h>
#include <platform/debug_uart.h>
#include <lib/stdarg.h>

extern volatile tcb_t *caller;

void set_user_error(tcb_t *thread, enum user_error_t error)
{
	assert(thread && thread->utcb);

	thread->utcb->error_code = error;
}

void set_caller_error(enum user_error_t error)
{
	if (caller)
		set_user_error((tcb_t *) caller, error);
	else
		panic("User-level error %d during in-kernel call!", error);
}

#ifdef CONFIG_PANIC_DUMP_STACK
static void panic_dump_stack()
{
	uint32_t *current_sp = (uint32_t *) read_msp();
	int word = 0;

	dbg_puts("\n\nStack dump:\n");

	while (current_sp < &kernel_stack_end) {
		dbg_printf(DL_EMERG, "%p ", *(++current_sp));

		if (++word % 8 == 0)
			dbg_putchar('\n');
	}
}
#endif	/* CONFIG_PANIC_DUMP_STACK */

void panic_impl(char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);

	dbg_start_panic();

	irq_disable();
	dbg_vprintf(DL_EMERG, fmt, va);

#ifdef CONFIG_KDB
	kdb_dump_error();
#endif

	panic_dump_stack();

	while (1)
		/* */ ;
}

void assert_impl(int cond, const char *condstr, const char *funcname)
{
	if (!cond) {
		/* Write to buffer */
		panic("Assertion %s failed @%s\n", condstr, funcname);
	}
}
