/*********************************************************************
 *                
 * Copyright (C) 2003, 2007-2008, 2010,  Karlsruhe University
 *                
 * File path:     l4test/exreg.cc
 * Description:   Various ExchangeRegisers() tests
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
 * $Id: exreg.cc,v 1.7 2006/10/05 13:28:47 reichelt Exp $
 *                
 ********************************************************************/
#include <l4/thread.h>
#include <l4/ipc.h>
#include <l4io.h>
#include <l4/kip.h>
#include <config.h>

/* for the current arch */
#include <arch.h>

/* generic stuff */
#include "l4test.h"
#include "menu.h"
#include "assert.h"
#include "threads.h"

static void
dummy_thread(void)
{
	/* don't do much! */
	while(1)
		msec_sleep( 500 );;
}

static void
printy_thread(void)
{
	while(1)
	{
            //printf( "." );
            msec_sleep( 500 );
	}
}

/* just exreg it for no particular reason. eg. enquire about its
 * existence. The thread shouldn't exist before you call this! 
 */
L4_ThreadId_t
dumb_exreg_thread( L4_ThreadId_t tid )
{
	L4_ThreadId_t ret, pager;
	L4_Word_t control, sp, pc, flags, user;

	pager = L4_Myself();

	/* Make an invalid exreg on it */
	control = EX_RECV | EX_SEND | EX_SP | EX_IP | EX_PAGR;

	ret = L4_ExchangeRegisters( tid, control, NULL, NULL, 0, 0, pager, 
				    &control, &sp, &pc, &flags, &user, 
				    &pager );

	/* jsXXX: this is an obnoxious cast to make GCC-4.3.1 happy */
	return (L4_ThreadId_t) ret;
}

L4_ThreadId_t
do_exreg_thread_pager( L4_ThreadId_t pager, L4_ThreadId_t tid, 
		       L4_Word_t ip, L4_Word_t sp )
{
	L4_ThreadId_t ret;
	L4_Word_t control, flags, user;

	/* Make an invalid exreg on it */
	control = EX_RECV | EX_SEND | EX_SP | EX_IP | EX_PAGR;

	ret = L4_ExchangeRegisters( tid, control, sp, ip, 0, 0, pager, 
				    &control, &sp, &ip, &flags, &user, 
				    &pager );

	return ret;
}

L4_ThreadId_t
do_exreg_thread( L4_ThreadId_t tid, L4_Word_t ip, L4_Word_t sp )
{
	L4_ThreadId_t pager = L4_Myself();
	return do_exreg_thread_pager( pager, tid, ip, sp );
}

static void
exreg_to_null(void)
{
	L4_Word_t res;
	L4_ThreadId_t tid, me;

	/* get a TID */
	tid = get_new_tid();
	me = L4_Myself();

	//printf( "ThreadControl..." );

	res = L4_ThreadControl (tid, me, me, L4_nilthread, (void *) -1);
	if( res != 1 )
	{
		printf( "ERROR: ThreadControl returned %ld\n", res );
		return;
	}

	//printf( "Sleeping around TC bug..." );
	msec_sleep( 1000 );

	//printf( "Starting with exreg..." );

	/* exreg it to the NULL */
	do_exreg_thread_pager( L4_nilthread, tid, NULL, NULL );

	/* wait a bit */
	/* it should print out now... */
	msec_sleep( 5000 );

	//printf( "Killing thread\n" );

	/* kill it :) */
	kill_thread( tid );
        
        print_result ("ExReg thread with no pager", true);

        
}


/* menu functions */

static void
exreg_ret(void)
{
	L4_ThreadId_t tid, ret;

	/* get an unused TID */
	tid = get_new_tid();

	ret = dumb_exreg_thread( tid );

	/* checking for error returns is always a pain */
        print_result ("ExReg return value", (ret.raw == L4_nilthread.raw));
}

static void
exreg_g2l(void)
{
	L4_ThreadId_t myg, myl;
	L4_ThreadId_t rmyg, rmyl;

	/* get my IDs out of TCRs */
	myg = L4_MyGlobalId();
	myl = L4_MyLocalId();

	/* use ex-reg to swap them */
	rmyg = L4_GlobalIdOf( myl );
	rmyl = L4_LocalIdOf( myg );


	/* check them */
        print_result ("ExRegs local  -> global", myg == rmyg);
        print_result ("ExRegs global -> local ", myl == rmyl);
}

static void
ex_thrash(void)
{
	L4_Word_t ip, sp;
	L4_ThreadId_t tid;
	int x = 0;

	/* init a new stack & all */
	sp = NULL;
	setup_exreg( &ip, &sp, printy_thread );

	/* get a TID */
	tid = create_thread();

	/* touch the entry point */
	safe_mem_touch( code_addr( (void*) printy_thread ) );
	safe_mem_touch( code_addr( (void*) dummy_thread ) );

	/* start it! */
	//printf( "Starting Thread..." );
	start_thread( tid, ip, sp );

	/* wait a bit */
	msec_sleep( 5000 );

	/* FIXME: do a recv() here to see if it page-faulted?? 
	 * ... and barf if it has? just in case?
	 */

	//printf( "Ex-Reg'ing Thread..." );

	/* exreg the thread many times */
	for( x = 0; x < 20; x++ )
	{
		setup_exreg( &ip, &sp, printy_thread );
		do_exreg_thread( tid, ip, sp );

		/* give it a time-slice */
		msec_sleep( 500 );
	}

	//printf( "Killing thread\n" );

	/* kill it :) */
	kill_thread( tid );
        
        print_result ("ExReg thrash (wait)", true);
}

static void
ex_thrash2(void)
{
	L4_Word_t ip, sp;
	L4_ThreadId_t tid;
	int x = 0;

	/* init a new stack & all */
	sp = NULL;
	setup_exreg( &ip, &sp, printy_thread );

	/* get a TID */
	tid = create_thread();

	/* start it! */
	//printf( "Starting Thread..." );
	start_thread( tid, ip, sp );

	//printf( "Ex-Reg'ing Thread..." );

	/* exreg the thread many times */
	for( x = 0; x < 100; x++ )
	{
		setup_exreg( &ip, &sp, printy_thread );
		do_exreg_thread( tid, ip, sp );
	}

	//printf( "Killing thread\n" );

	/* kill it :) */
	kill_thread( tid );
        
        print_result ("ExReg thrash (nowait)", true);

}

static void
ex_tc(void)
{
	L4_ThreadId_t tid;
	L4_Word_t res;
	L4_ThreadId_t mylocalid = L4_MyLocalId ();
        static L4_Word_t utcb_base;
        static void * kip;
	kip = L4_GetKernelInterface ();
	
        utcb_base = *(L4_Word_t *) &mylocalid;
	utcb_base &= ~(L4_UtcbAreaSize (kip) - 1);

	/* create us a thread ID */
	tid = get_new_tid();
        
        L4_Word_t utcb_location =
            utcb_base + L4_UtcbSize (kip) * L4_ThreadNo (tid);

	/* exreg it for no good reason */
	//printf( "Dumb exreg..." );
	dumb_exreg_thread( tid );

	/* now create it */
	//printf( "ThreadControl\n" );
	res = L4_ThreadControl( tid, L4_Myself(), L4_Myself(), L4_Myself(), (void *) utcb_location);
        
        print_result ("ExReg then ThreadControl", res == 1);
	
	/* I wonder if that killed it? 
	 * kill it now 
	 */
	kill_thread( tid );
        
}


static L4_ThreadId_t test_id;

static void dummy_exreg_thread (void)
{
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (test_id);

    for (;;);
}

static void
exreg_inactive_thread (void)
{
    L4_ThreadId_t tid, rt, dt;
    L4_Word_t ip, sp, dummy;
    bool ok = true;

    test_id = L4_Myself ();

    // Create inactive thread
    tid = create_thread ();

    // Start thread using ExchangeRegisters
    get_startup_values (dummy_exreg_thread, &ip, &sp);
    rt = L4_ExchangeRegisters (tid, (0x33 << 3) + 6, sp, ip, 0, 0, L4_Pager (),
			       &dummy, &dummy, &dummy, &dummy, &dummy, &dt);

    if (rt == L4_nilthread)
    {
	printf ("ExchangeRegisters() returned nilthread\n");
	ok = false;
    }
    else
    {
	L4_MsgTag_t tag = L4_Receive (tid, L4_TimePeriod (5*1000*1000));
	if (L4_IpcFailed (tag))
	{
	    printf ("No reply from newly started thread\n");
	    ok = false;
	}
    }

    print_result ("ExchangeRegisters on inactive thread", ok);
    kill_thread (tid);
}

void all_exreg_tests(void)
{
    exreg_ret();
    exreg_g2l();
    ex_thrash();
    ex_thrash2();
    ex_tc();
    exreg_inactive_thread();
    exreg_to_null();
       
}

/* the menu */
static struct menuitem menu_items[] = 
{
	{ NULL, "return" },
	{ exreg_ret,  "Test return value" },
	{ exreg_g2l,  "Test global <-> local" },
	{ ex_thrash,  "ExReg many times (with wait)" },
	{ ex_thrash2, "ExReg many times (without wait)" },
	{ ex_tc    ,  "ExReg then ThreadControl" },
	{ exreg_inactive_thread,
	  "ExchangeRegisters on inactive thread" },
	{ exreg_to_null, "Try to start a thread with no pager (@NULL)" },
        { all_exreg_tests, "All exreg tests" },

};

static struct menu menu = 
{
	"Exreg Menu",
	0, 
	NUM_ITEMS(menu_items),
	menu_items
};


/* entry point */
void 
exreg_test(void)
{
	menu_input( &menu );
}

