/*********************************************************************
 *                
 * Copyright (C) 2003, 2010,  Karlsruhe University
 *                
 * File path:     l4test/powerpc64/tests.cc
 * Description:   Architecture dependent tests
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
 * $Id: tests.cc,v 1.5 2003/12/04 06:41:57 cvansch Exp $
 *                
 ********************************************************************/

#include <l4/types.h>
#include <l4/kip.h>
#include <l4/ipc.h>
#include <l4io.h>
#include <arch.h>
#include "../l4test.h"
#include "../menu.h"
#include "1275tree.h"

L4_Word_t Args[16];
char *devtree = NULL;

/* sigma0 fpage request stuff */
#define FP_REQUEST_LABEL ((-6UL) << 4)     /* that's what the API says! */

static int
request_page( void *page )
{
	/* setup an acceptor? */
	L4_Accept( L4_MapGrantItems( L4_CompleteAddressSpace )  );

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


static char * find_1275tree(void)
{
    L4_MemoryDesc_t * md;
    L4_KernelInterfacePage_t * kip = (L4_KernelInterfacePage_t *)
	    L4_KernelInterface( NULL, NULL, NULL );

    // Parse through all memory descriptors in kip.
    for (L4_Word_t n = 0; (md = L4_MemoryDesc (kip, n)); n++)
    {
	if (L4_IsVirtual (md))
	    continue;

	L4_Word_t low = (L4_MemoryDescLow (md));
	L4_Word_t high = (L4_MemoryDescHigh (md));

	if ( (L4_MemoryDescType (md)) == (0xf0 | L4_BootLoaderSpecificMemoryType) )
	{
	    for( L4_Word_t i = low; i < high; i += PAGE_SIZE )
	    {
		if( request_page( (void *)i ) )
		{
		    printf( "Cannot get 1275 device tree memory\n" );
		    return NULL;
		}
	    }
	    return (char *)low;
	}

    }
    return NULL;
}

void rtas_test(void)
{
    char *str = "\r\nl4test: RTAS\r\ntest successful";

    printf( "\nTesting RTAS\n\n" );

    if( !(devtree = find_1275tree()) )
    {
	printf( "Cannot find 1275 device tree\n" );
	return;
    }

    printf( "Found 1275 device tree at %p\n", devtree );

    device_t *root = device_first( devtree );
    device_t *rtas = device_find( root, "/rtas" );

    if (!rtas)
    {
	printf( "Cannot find \"/rtas\" in device tree\n" );
	return;
    }

    item_t *print = item_find( rtas, "display-character" );

    if (!print)
    {
	printf( "Cannot find \"display-character\" in property in rtas\n" );
	return;
    }

    L4_Word32_t *token = (L4_Word32_t *)print->data;

    printf( "Displaying characters..." );

    for( int i = 0; i < strlen( str ); i++ )
    {
	Args[0] = str[i];
	L4_RtasCall( *token, 1, 1, (L4_Word_t *)&Args );
    }

    printf( "done\n" );
}

void fpu_test(void)
{
    asm volatile (
	"fadd	1, 2, 3;   "
	);
}

void all_arch_tests( void )
{
    rtas_test();
    fpu_test();
}

/* the menu */
static struct menuitem menu_items[] = 
{
    { NULL, "return" },
    { rtas_test,  "Test RTAS" },
    { fpu_test, "Test FPU" },
    { all_arch_tests,	"All PowerPC tests" },
};

static struct menu menu = 
{
    "PowerPC64 Menu",
    0, 
    NUM_ITEMS(menu_items),
    menu_items
};


void arch_test(void)
{
    menu_input( &menu );
}

