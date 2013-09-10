/*********************************************************************
 *                
 * Copyright (C) 2003, 2007, 2010,  Karlsruhe University
 *                
 * File path:     l4test/sig0.cc
 * Description:   Various sigma0 tests
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
 * $Id: sig0.cc,v 1.7 2003/10/29 22:51:29 cvansch Exp $
 *                
 ********************************************************************/
#include <l4/ipc.h>
#include <l4io.h>
#include <config.h>

/* for the current arch */
#include <arch.h>

/* generic stuff */
#include "l4test.h"
#include "menu.h"
#include "assert.h"

/* sigma0 fpage request stuff */
#define FP_REQUEST_LABEL ((-6UL) << 4)     /* that's what the API says! */


static int
request_page( void *page )
{
	L4_ThreadId_t tid;
	L4_MsgTag_t tag;
	L4_Msg_t msg;
	L4_Word_t rattrib;
	L4_MapItem_t map;
	L4_Fpage_t rfpage;

	/* find our pager's ID */
	tid = L4_Pager();

	/* setup the request */
	rfpage = L4_FpageLog2( (L4_Word_t) page, PAGE_BITS );
	L4_Set_Rights( &rfpage, L4_ReadWriteOnly );
	rattrib = 0;  /* arch. default attributes */

	/* send it to our pager */
	L4_Clear( &msg );
	L4_Append(&msg, rfpage.raw );
	L4_Append(&msg, rattrib );
	L4_Set_Label( &msg.tag, FP_REQUEST_LABEL );
	L4_Load( &msg );

	/* make the call */
	tag = L4_Call(tid);

	/* check for an error */
	if( IPC_ERROR(tag) )
		return 1;

	/* FIXME: check no. typed/untyped words? */
	/* decipher the results */
	L4_Store(tag, &msg);
	L4_Get( &msg, 0, &map );

	/* rejected mapping? */
	if( map.X.snd_fpage.raw == L4_Nilpage.raw )
		return 1;

	/* PROFIT! */
	return 0;
}

static bool
do_req( int accept )
{
	void *page = get_new_page();
	int r;

	/* setup an acceptor? */
	if( accept )
		L4_Accept( L4_MapGrantItems( L4_CompleteAddressSpace )  );
	else
		L4_Accept( L4_UntypedWordsAcceptor  );
	
	/* make the request */
	r = request_page( page );

	/* give sigma0 a little time to recover */
	msec_sleep(1000);

        return (r == 0);
        
}

/* menu fns */
static void
request_mem(void)
{
    bool ok = do_req(1);
    print_result ("Sigma0 memory request", ok);

}

static void
bad_send(void)
{
	L4_MsgTag_t tag;
	L4_Msg_t msg;

	L4_Clear( &msg );
	L4_Append( &msg, 0 );
	L4_Append( &msg, 0 );
	L4_Load( &msg );

	tag = L4_Call_Timeouts( L4_Pager(), L4_Never, L4_TimePeriod( 1000 * 1000 ));
        bool ok = (L4_IpcFailed(tag) && (L4_ErrorCode() & 0x1 == 1));

	/* give sigma0 a little time to recover */
	msec_sleep(1000);
        
        print_result ("Sigma0 memory request (phony send, ok=noresponse)", ok);

}

static void
bad_recv(void)
{
    bool ok = !do_req(0);
    print_result ("Sigma0 memory request (phony receive, ok=nilmapping)", ok);
}


/*
 * Encoding for label of extended sigma0 protocol.
 */
#define L4_SIGMA0_EXT		(-1001UL << 4)

/*
 * Encoding for MR1 of extended sigma0 protocol.
 */
#define L4_S0EXT_VERBOSE	((L4_Word_t) 1)
#define L4_S0EXT_DUMPMEM	((L4_Word_t) 2)

static void
dump_mempools (void)
{
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    L4_Clear (&msg);
    L4_Set_Label (&msg, L4_SIGMA0_EXT);
    L4_Append (&msg, L4_S0EXT_DUMPMEM);
    L4_Append (&msg, 1UL);
    L4_Load (&msg);

    tag = L4_Call_Timeouts( L4_Pager(), L4_Never, L4_TimePeriod( 1000 * 1000 ));
    print_result ("Sigma0 pool dump", L4_IpcSucceeded(tag));
    
    
}

void all_s0_tests(void)
{
    request_mem();
    bad_send();
    bad_recv();
    dump_mempools();
}


/* the menu */
static struct menuitem menu_items[] = 
{
	{ NULL, "return" },
	{ request_mem,  "Request Memory" },
	{ bad_send,     "Phony Send" },
	{ bad_recv,     "Phony Receive" },
	{ dump_mempools, "Dump Mempools" },
        { all_s0_tests, "All sigma0 tests" },
};

static struct menu menu = 
{
	"Memory Menu",
	0, 
	NUM_ITEMS(menu_items),
	menu_items
};


/* entry point */
void 
sig0_test(void)
{
	menu_input( &menu );
}

