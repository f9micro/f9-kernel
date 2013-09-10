/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     l4test/menu.cc
 * Description:   Menu interface for test suite
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
 * $Id: menu.cc,v 1.4 2003/09/24 19:05:54 skoglund Exp $
 *                
 ********************************************************************/
#include <l4/kdebug.h>
#include <l4io.h>
#include <config.h>

#include "l4test.h"
#include "assert.h"
#include "menu.h"

#define KDEBUG_CHAR 27  /* escape */

void
clear_screen(void)
{
	printf( CLEAR_SCREEN );
	printf( HOME );
}

/* print out a menu */
static void
print_menu( struct menu *menu )
{
	int i;

	assert( menu != NULL );

#if USE_ANSI
	if( menu->clear )
		clear_screen();
	else
		printf( HOME );
#else       
	printf( "\n\n\n" );
#endif

	print_h1( menu->title );

	for( i = 0; i < menu->nitems; i++ )
		printf( "%d) %s\n", i, menu->items[i].prompt );

#if USE_ANSI
	while( i++ < 10 )
		printf( CLEAR_LINE "\n"  );
#endif
}

/* get the user input */
static void
get_item( struct menu *menu )
{
	int ch, i;
	void (*func)(void);

	assert( menu != NULL );

	while(1)
	{
		print_menu( menu );
	
		ch = L4_KDB_ReadChar_Blocked();

		if( ch == KDEBUG_CHAR )
			enter_kdebug("user hit break");

#if USE_ANSI
		if( ch == 'c' )
		{
			clear_screen();
			continue;
		}
#endif

		/* make sure they input a number */
		if( ch < '0' || ch > '9' )
			continue;

		i = ch - '0';

		/* make sure it's in the item list */
		if( i >= menu->nitems )
			continue;

		func = menu->items[i].func;

		/* NULL == return */
		if( func == NULL )
			return;

#if USE_ANSI
		/* clear out the previous output */
		clear_screen();
		printf( "\n\n\n\n\n\n\n\n\n\n\n\n\n" );
#endif

		func();
		
	}
}

/* prompt user for a menu item */
void 
menu_input( struct menu *menu )
{
	assert( menu != NULL );
	
	get_item( menu );
}
