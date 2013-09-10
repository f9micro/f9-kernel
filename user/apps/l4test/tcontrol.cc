/*********************************************************************
 *                
 * Copyright (C) 2003, 2010,  Karlsruhe University
 *                
 * File path:     l4test/tcontrol.cc
 * Description:   Various thread control tests
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
 * $Id: tcontrol.cc,v 1.6 2004/03/17 02:37:27 benno Exp $
 *                
 ********************************************************************/
#include <l4/types.h>
#include <l4/thread.h>
#include <l4io.h>
#include <config.h>

/* for the current arch */
#include <arch.h>

/* generic stuff */
#include "l4test.h"
#include "menu.h"
#include "assert.h"
#include "threads.h"

static int ok_thread_worked = 0;

static void
print_ok_thread(void)
{
	ok_thread_worked = 1;
	//printf( "This is OK Thread running...\n" );
	while(1)
		msec_sleep( 500 );
}

static void
printy_thread(void)
{
	while(1)
	{
		msec_sleep( 500 );
	}
}

static void
run_a_thread(void)
{
	L4_Word_t ip, sp;
	L4_ThreadId_t tid;

	/* init a new stack & all */
	sp = NULL;
	setup_exreg( &ip, &sp, printy_thread );

	/* get a TID */
	tid = create_thread();

	/* touch the entry point */
	safe_mem_touch( code_addr( (void*) printy_thread ) );

	/* start it! */
	start_thread( tid, ip, sp );

	//printf( "Starting Thread..." );

	/* wait a bit */
	msec_sleep( 5000 );

	/* FIXME: do a recv() here to see if it page-faulted?? 
	 * ... and barf if it has? just in case?
	 */

	//printf( "Killing thread\n" );

	/* kill it :) */
	kill_thread( tid );
        
        print_result ("Run a thread", true);

}


static void
tc_then_exreg(void)
{
	L4_Word_t ip, sp;
	L4_ThreadId_t tid;

	/* init a new stack & all */
	sp = NULL;
	setup_exreg( &ip, &sp, print_ok_thread );

	/* get a TID */
	tid = create_thread();

	/* touch the entry point */
	safe_mem_touch( code_addr( (void*) print_ok_thread  ) );
	ok_thread_worked = 0; /* touch and set! */

	/* exreg it to the start */
	do_exreg_thread( tid, ip, sp );

	/* wait a bit */
	/* it should print out now... */
	msec_sleep( 5000 );

	/* check if it flagged as OK */
        print_result ("ThreadControl+ExReg", ok_thread_worked != 0);
        

	/* kill it :) */
	kill_thread( tid );
}

static void
delete_self(void)
{
    L4_Word_t res = kill_thread(L4_Myself());
    print_result ("Delete self", (res == 0));

}

void all_tc_tests(void)
{
    run_a_thread();
    delete_self();
    tc_then_exreg();

}

/* the menu */
static struct menuitem menu_items[] = 
{
	{ NULL, "return" },
	{ run_a_thread,  "Run a thread" },
	{ delete_self,  "Delete self" },
	{ tc_then_exreg, "ThreadControl + ExReg" },
	{ all_tc_tests, "All ThreadControl tests" },
};

static struct menu menu = 
{
	"Thread Control Menu",
	0, 
	NUM_ITEMS(menu_items),
	menu_items
};


/* entry point */
void 
tcontrol_test(void)
{
	menu_input( &menu );
}

