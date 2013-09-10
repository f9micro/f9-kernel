/*********************************************************************
 *                
 * Copyright (C) 2004, 2010,  University of New South Wales
 *                
 * File path:     l4test/schedule.cc
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
 * $Id: schedule.cc,v 1.2 2006/10/05 13:28:58 reichelt Exp $
 *                
 ********************************************************************/

#include <l4/types.h>
#include <l4/thread.h>
#include <l4/schedule.h>
#include <l4io.h>
#include <config.h>

/* for the current arch */
#include <arch.h>

/* generic stuff */
#include "l4test.h"
#include "menu.h"
#include "assert.h"
#include "threads.h"

#define IRQ_NUMBER 3


static void
test_irq_priority_unassociated(void)
{
	int ret;
	L4_ThreadId_t irq_thrd;
	irq_thrd.global.X.thread_no = IRQ_NUMBER;
	irq_thrd.global.X.version = 1;

	//printf("Changing priority of irq thread %lx\n", 
        //     irq_thrd.global.X.thread_no);
	ret  = L4_Set_Priority(irq_thrd, 12);
        
         print_result ("Change priority unassociated (ok=change failed)", ret == 0);
	
}

static void
test_irq_priority_associated(void)
{
	int ret;
	L4_ThreadId_t irq_thrd;
	irq_thrd.global.X.thread_no = IRQ_NUMBER;
	irq_thrd.global.X.version = 1;

	//printf("Associating irq thread\n");
	ret = L4_AssociateInterrupt(irq_thrd, L4_Myself());
	if (ret == 0) {
            print_result ("Change priority associated: associate failed", false);
            return;
	}
	ret  = L4_Set_Priority(irq_thrd, 13);
        print_result ("Change priority associated (ok=change succeeded)", ret != 0);

}

void all_schedule_tests(void)
{

    test_irq_priority_unassociated();
    test_irq_priority_associated();
}

/* the menu */
static struct menuitem menu_items[] = 
{
	{ NULL, "return" },
	{ test_irq_priority_unassociated,  
	  "Change irq thread priority of an unassociated thread" },
	{ test_irq_priority_associated,  
	  "Change irq thread priority of an associated thread" },
        { all_schedule_tests, "All schedule tests" },
};

static struct menu menu = 
{
	"Schedule Menu",
	0, 
	NUM_ITEMS(menu_items),
	menu_items
};


/* entry point */
void 
schedule_test(void)
{
	menu_input( &menu );
}

