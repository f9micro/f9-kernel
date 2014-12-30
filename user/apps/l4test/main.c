/* Copyright (c) 2002-2003, 2007, 2010 University of New South Wales
 * All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <user_runtime.h>
#include <l4/kip.h>
#include <l4/thread.h>
#include <l4/ipc.h>
#include <l4/kdebug.h>
#include <l4io.h>

/* for the current arch */
#include "platform/arch.h"
#include <platform/cortex_m.h>

/* generic stuff */
#include "config.h"
#include "l4test.h"
#include "assert.h"

#define STACK_SIZE 512

/* where to start allocating RAM */
__USER_BSS static char *free_page;

/* colours */
static void
set_colour(const char *col)
{
#ifdef USE_ANSI
	printf("%s", col);
#endif
}

void
print_uline(const char *msg, char c)
{
	int i, len = strlen(msg);

	printf("%s\n", msg);

	for (i = 0; i < len; i++)
		putc(c);
	putc('\n');
}

void
print_h1(const char *msg)
{
	set_colour(LIGHT_BLUE);
	print_uline(msg, '=');
	set_colour(BLACK);
}

void
print_h2(const char *msg)
{
	set_colour(LIGHT_RED);
	print_uline(msg, '-');
	set_colour(BLACK);
}

__USER_TEXT void
print_result(const char *str, bool test)
{
	printf("  %s: ", str);
	printf("  %s\n", (test) ? STR_OK : STR_FAILED);
	if (! test)
		L4_KDB_Enter("test failed");
}

__USER_TEXT L4_Word_t
safe_mem_touch(void *addr)
{
	volatile L4_Word_t *ptr;
	L4_Word_t copy;

	ptr = (L4_Word_t *) addr;
	copy = *ptr;
	*ptr = copy;

	return copy;
}

__USER_TEXT void *
get_pages(L4_Word_t count, int touch)
{
	void *ret = free_page;

	free_page += count * PAGE_SIZE;

	/* should we fault the pages in? */
	if (touch != 0) {
		char *addr = (char *) ret;
		L4_Word_t i;

		/* touch each page */
		for (i = 0; i < count; i++) {
			safe_mem_touch((void *) addr);
			for (int j = 0; j < PAGE_SIZE; j++)
				addr[j] = 0;
			addr += PAGE_SIZE;
		}
	}

	return (void *) ret;
}

__USER_TEXT void *
get_new_page(void)
{
	return get_pages(1, 0);
}

__USER_TEXT void
get_startup_values(void (*func)(void),
                   L4_Word_t *ip, L4_Word_t *sp,
                   L4_Word_t *stack_size)
{
	/* Calculate intial SP */
	L4_Word_t stack = (L4_Word_t) get_pages(STACK_PAGES, 1);
	stack += STACK_PAGES * PAGE_SIZE;

	*ip = (L4_Word_t) func;
	*sp = stack;
	*stack_size = STACK_PAGES * PAGE_SIZE;
}

void
start_thread_ip_sp(L4_ThreadId_t tid, L4_Word_t ip, L4_Word_t sp)
{
	L4_Msg_t msg;

	L4_MsgClear(&msg);
	L4_MsgAppendWord(&msg, ip);
	L4_MsgAppendWord(&msg, sp);
	L4_MsgLoad(&msg);

	L4_Send(tid);
}

void
msec_sleep(L4_Word_t msec)
{
	L4_Sleep(L4_TimePeriod(msec * 1000));
}

__USER_TEXT void all_tests(void)
{
	extern void all_ipc_tests(void);

	all_ipc_tests();
}

__USER_TEXT
static void *main(void *user)
{
	printf("\nL4/Pistachio test suite starts\n");
	all_tests();
	return NULL;
}

DECLARE_USER(
	256,
	l4test,
	main,
	DECLARE_FPAGE(0x0, 4 * (UTCB_SIZE + STACK_SIZE))
	DECLARE_FPAGE(0x0, 512)
);
