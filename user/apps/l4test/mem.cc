/*********************************************************************
 *                
 * Copyright (C) 2003, 2010,  Karlsruhe University
 *                
 * File path:     l4test/mem.cc
 * Description:   
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
 * $Id: mem.cc,v 1.5 2003/09/24 19:05:54 skoglund Exp $
 *                
 ********************************************************************/
/* memory testing */

#include <l4/types.h>
#include <l4io.h>
#include <config.h>

/* for the current arch */
#include <arch.h>

/* generic stuff */
#include "l4test.h"
#include "menu.h"
#include "assert.h"


static void
page_touch(void)
{
	volatile L4_Word_t *addr = (L4_Word_t*) get_new_page();
	int n = 0;
        int max = 1000;
        
	printf("test: Welcome to memtest!\n" );

	while (max--)
	{
            //printf( "Touching page %p (%d)\n", addr, n );
            *addr = 0x37ULL;
            addr += (PAGE_SIZE/sizeof(*addr));
            n++;
            //printf( "Touched page!\n" );
	}
        print_result ("Page touch", true);


}

void all_mem_tests()
{
    page_touch();
}
/* the menu */
static struct menuitem menu_items[] = 
{
	{ NULL, "return" },
	{ page_touch,  "Page Touch" },
	{ all_mem_tests,  "All mem tests" },
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
mem_test(void)
{
	menu_input( &menu );
}

