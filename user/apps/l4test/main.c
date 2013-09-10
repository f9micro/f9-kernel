/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2007, 2010,  University of New South Wales
 *                
 * File path:     l4test/main.cc
 * Description:   main setup/loop for L4/pistachio testing suite
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: main.cc,v 1.12 2004/04/06 00:42:03 benno Exp $
 *                
 ********************************************************************/

#include <l4/kip.h>
#include <l4/thread.h>
#include <l4/ipc.h>
#include <l4/kdebug.h>
#include <l4io.h>
#include <config.h>

/* for the current arch */
#include <platform/arch.h>

/* generic stuff */
#include "l4test.h"
#include "menu.h"
#include "assert.h"

/* test modules */
extern void kip_test(void);
extern void arch_test(void);
extern void mem_test(void);
extern void ipc_test(void);
extern void sig0_test(void);
extern void exreg_test(void);
extern void tcontrol_test(void);
extern void schedule_test(void);

#if defined(L4TEST_AUTORUN)
static bool autorun = true;
#else
static bool autorun = false;
#endif

/* where to start allocating RAM */
static char *free_page = (char*) SCRATCHMEM_START;

/* colours */
static void 
set_colour( const char *col )
{
#ifdef USE_ANSI
	printf( "%s", col );
#endif
}

void 
print_uline( const char *msg, char c )
{
	int i, len = strlen( msg );

	printf( "%s\n", msg );

	for( i = 0; i < len; i++ )
		putc (c);
	putc ('\n');
}

void 
print_h1( const char *msg )
{
	set_colour( LIGHT_BLUE );
	print_uline( msg, '=' );
	set_colour( BLACK );
}

void 
print_h2( const char *msg )
{
	set_colour( LIGHT_RED );
	print_uline( msg, '-' );
	set_colour( BLACK );
}

void
print_result (const char * str, bool test)
{
    printf ("  %s: ", str);
    for (int __i = 60 - strlen (str); __i > 0; __i--)
	putc (' ');
    printf ("  %s\n", (test) ? STR_OK : STR_FAILED);
    if (! test && !autorun)
	L4_KDB_Enter ("test failed");
}



L4_Word_t
safe_mem_touch( void *addr )
{
	volatile L4_Word_t *ptr;
	L4_Word_t copy;

	ptr = (L4_Word_t*) addr;
	copy = *ptr;
	*ptr = copy;

	return copy;
}

void *
get_pages( L4_Word_t count, int touch )
{
	void *ret = free_page;

	free_page += count * PAGE_SIZE;
	
	/* should we fault the pages in? */
	if( touch != 0 )
	{
		char *addr = (char*) ret;
		L4_Word_t i;

		/* touch each page */
		for( i = 0; i < count; i++ )
		{
			safe_mem_touch( (void*) addr );
			for (int j=0; j<PAGE_SIZE; j++)
			    addr[j] = 0;
				
			addr += PAGE_SIZE;
			
		}
	}

	return (void*) ret;
}

void *
get_new_page(void)
{
	return get_pages( 1, 0 );
}

void
start_thread_ip_sp( L4_ThreadId_t tid, L4_Word_t ip, L4_Word_t sp )
{
	L4_Msg_t msg;

	L4_Clear( &msg );
	L4_Append( &msg, ip );
	L4_Append( &msg, sp );
	L4_Load( &msg );

	L4_Send( tid );
}

void
msec_sleep( L4_Word_t msec )
{
	L4_Sleep( L4_TimePeriod( msec * 1000 ) );
}

void all_tests(void)
{
    extern void all_kip_tests(void);
    extern void all_arch_tests(void);
    extern void all_mem_tests(void);
    extern void all_ipc_tests(void);
    extern void all_s0_tests(void);
    extern void all_exreg_tests(void);
    extern void all_tc_tests(void);
    extern void all_schedule_tests(void);

    all_kip_tests();
    all_arch_tests();
    all_mem_tests();
    all_ipc_tests();
    all_s0_tests();
    all_exreg_tests();
    all_tc_tests();
    all_schedule_tests();

}

/* Main menu code */
static struct menuitem main_menu_items[] = 
{
	{ kip_test, "Test KIP" },
	{ arch_test, ARCH_NAME " Tests" },
	{ mem_test, "Test Memory" },
	{ ipc_test, "Test IPC" },
	{ sig0_test, "Test Sigma0" },
	{ exreg_test, "Test ExReg" },
	{ tcontrol_test, "Test ThreadControl" },
	{ schedule_test, "Test Schedule" },
	{ all_tests, "All tests" },
};

static struct menu main_menu = 
{
	"Main menu",
	1, 
	NUM_ITEMS(main_menu_items),
	main_menu_items
};


int main (void)
{
	printf( "L4/Pistachio test suite ready to go.\n\n" );
        
        if (autorun)
            all_tests();
        
        menu_input( &main_menu );

	assert( !"Shouldn't get here!" );

	return 0;
}
