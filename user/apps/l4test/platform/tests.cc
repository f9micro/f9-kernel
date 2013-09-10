/*********************************************************************
 *                
 * Copyright (C) 2007-2008, 2010,  Karlsruhe University
 *                
 * File path:     l4test/ia32/tests.cc
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#include <l4/kip.h>
#include <l4/ipc.h>
#include <l4/schedule.h>
#include <l4/kdebug.h>
#include <l4/ia32/arch.h>

#include <l4io.h>
#include "../l4test.h"
#include "../assert.h"
#include "../menu.h"

#define START_ADDR(func)	((L4_Word_t) func)
#define NOUTCB	((void*)-1)

L4_Word_t exc_stack[2048] __attribute__ ((aligned (16)));
volatile L4_Word_t zero = 0;

void exc (void)
{
    L4_Set_ExceptionHandler(L4_Pager());
    __attribute__(( unused )) volatile int x = 1 / zero;
}

void exc2 (void)
{
    print_result("IPC ctrlxfer handling", true);
    L4_Send(L4_Pager());
    __attribute__(( unused )) volatile int x = 1 / zero;
}

void exc3 (void)
{
    print_result("ExRegs ctrlxfer write", true);
    L4_Send(L4_Pager());
    while (1);
}

void exception_test(void)

{
    L4_KernelInterfacePage_t * kip =
	(L4_KernelInterfacePage_t *) L4_KernelInterface ();
    
    L4_Word_t utcb_size = L4_UtcbSize (kip); 
    L4_MsgTag_t tag;
    L4_Msg_t exc_msg;
    L4_ThreadId_t exc_tid = L4_GlobalId (L4_ThreadNo (L4_Myself()) + 1, 2);
    L4_Word_t exc_utcb = L4_MyLocalId().raw;
    
    exc_utcb = (exc_utcb & ~(utcb_size - 1)) + utcb_size;

    // Touch the memory to make sure we never get pagefaults
    extern L4_Word_t _end, _start;
    for (L4_Word_t * x = (&_start); x < &_end; x++)
    {
	volatile L4_Word_t q;
	q = *(volatile L4_Word_t*) x;
    }

    exc_tid = create_thread();

    
    L4_Start (exc_tid, (L4_Word_t) exc_stack + sizeof(exc_stack) - 32,
	      START_ADDR (exc));

    /* Test Exception IPC */

    
    tag = L4_Receive (exc_tid);
    L4_Store (tag, &exc_msg);


    //printf ("ExcHandler got msg from %p (%p, %p, %p, %p, %p)\n",
    //    (void *) tid.raw, (void *) tag.raw,
    //    (void *) L4_Get (&exc_msg, 0), (void *) L4_Get (&exc_msg, 1),
    //    (void *) L4_Get (&exc_msg, 2), (void *) L4_Get (&exc_msg, 3));;
    
    print_result("IA32 exception IPC handling", true);

    kill_thread( exc_tid );

}

void ctrlxfer_test(void)
{
    if (!l4_has_feature("ctrlxfer"))
    {
        printf("[CtrlXfer features not compiled in]");
        return;
    }

    L4_KernelInterfacePage_t * kip =
	(L4_KernelInterfacePage_t *) L4_KernelInterface ();
    
    L4_Word_t utcb_size = L4_UtcbSize (kip); 
    L4_MsgTag_t tag;
    L4_Msg_t ctrlxfer_msg;
    L4_Word_t dummy, old_control;
    L4_ThreadId_t tid;
    L4_GPRegsCtrlXferItem_t gpr_item;
    L4_GPRegsCtrlXferItem_t *gpr_item_ptr; 
    L4_CtrlXferItem_t item;    
    L4_ThreadId_t exc_tid = L4_GlobalId (L4_ThreadNo (L4_Myself()) + 1, 2);
    L4_Word_t exc_utcb = L4_MyLocalId().raw;
    L4_Word_t eip, esp;
    
    exc_utcb = (exc_utcb & ~(utcb_size - 1)) + utcb_size;

    // Touch the memory to make sure we never get pagefaults
    extern L4_Word_t _end, _start;
    for (L4_Word_t * x = (&_start); x < &_end; x++)
    {
	volatile L4_Word_t q;
	q = *(volatile L4_Word_t*) x;
    }

    exc_tid = create_thread();
    
    /* Set exception ctrlxfer mask */
    L4_Word64_t fault_id_mask = (1<<2) | (1<<3) | (1<<5);
    L4_Word_t fault_mask = L4_CTRLXFER_FAULT_MASK(L4_CTRLXFER_GPREGS_ID);
    L4_Clear(&ctrlxfer_msg);
    L4_AppendFaultConfCtrlXferItems(&ctrlxfer_msg, fault_id_mask, fault_mask, 0);
    L4_Load(&ctrlxfer_msg);
    L4_ConfCtrlXferItems(exc_tid);
 
    L4_Start (exc_tid, (L4_Word_t) exc_stack + sizeof(exc_stack) - 32,
	      START_ADDR (exc));

    /* Test Exception IPC */
    
    tag = L4_Receive (exc_tid);
    L4_Store (tag, &ctrlxfer_msg);

    /* Test Exception IPC Reply */
    L4_GPRegsCtrlXferItemInit(&gpr_item); 
    eip = START_ADDR(exc2);
    esp = (L4_Word_t) exc_stack + sizeof(exc_stack) - 32;
    L4_GPRegsCtrlXferItemSet(&gpr_item, L4_CTRLXFER_GPREGS_EIP, eip);
    L4_GPRegsCtrlXferItemSet(&gpr_item, L4_CTRLXFER_GPREGS_ESP,  esp);

	
    L4_Clear (&ctrlxfer_msg);
    L4_Append(&ctrlxfer_msg, &gpr_item);
    L4_Load (&ctrlxfer_msg);
    tag = L4_ReplyWait (exc_tid, &tid);
    
    tag = L4_Receive (exc_tid);
    L4_Store (tag, &ctrlxfer_msg);
    /* Test Exregs Read */ 
    gpr_item_ptr = (L4_GPRegsCtrlXferItem_t *) &ctrlxfer_msg.msg[ctrlxfer_msg.tag.X.u + 1];
	
    item = gpr_item_ptr->item;
    eip = gpr_item_ptr->regs.reg[L4_CTRLXFER_GPREGS_EIP];
    esp = gpr_item_ptr->regs.reg[L4_CTRLXFER_GPREGS_ESP];

    L4_GPRegsCtrlXferItemInit(&gpr_item); 
    L4_GPRegsCtrlXferItemSet(&gpr_item, L4_CTRLXFER_GPREGS_EIP, 0);
    L4_GPRegsCtrlXferItemSet(&gpr_item, L4_CTRLXFER_GPREGS_ESP, 0);
	
    L4_Clear (&ctrlxfer_msg);
    L4_Append(&ctrlxfer_msg, &gpr_item);
    L4_Load (&ctrlxfer_msg);
	
    L4_ExchangeRegisters (exc_tid, (1<<12), 0, 0 , 0, 0, L4_nilthread,
			  &old_control, &dummy, &dummy, &dummy, &dummy, &tid);
	
    L4_Store (tag, &ctrlxfer_msg);
    gpr_item_ptr = (L4_GPRegsCtrlXferItem_t *) &ctrlxfer_msg.msg[ctrlxfer_msg.tag.X.u + 1];
	
    item = gpr_item_ptr->item;
	
    if (eip == gpr_item_ptr->regs.reg[L4_CTRLXFER_GPREGS_EIP] &&
	esp == gpr_item_ptr->regs.reg[L4_CTRLXFER_GPREGS_ESP])
        print_result("ExRegs ctrlxfer read", true);
    else
        print_result("ExRegs ctrlxfer read", false);

    
    printf("Testing Exregs ctrlxfer write\n");
    
    L4_GPRegsCtrlXferItemInit(&gpr_item); 
    eip = START_ADDR(exc3);
    esp = (L4_Word_t) exc_stack + sizeof(exc_stack) - 32;

    L4_GPRegsCtrlXferItemSet(&gpr_item, L4_CTRLXFER_GPREGS_EIP, eip);
    L4_GPRegsCtrlXferItemSet(&gpr_item, L4_CTRLXFER_GPREGS_ESP, esp);
    L4_Clear (&ctrlxfer_msg);
    L4_Append(&ctrlxfer_msg, &gpr_item);
    L4_Load (&ctrlxfer_msg);
	
    L4_ExchangeRegisters (exc_tid, L4_EXREGS_CTRLXFER_WRITE_FLAG, 0, 0 , 0, 0, L4_nilthread,
			  &old_control, &dummy, &dummy, &dummy, &dummy, &tid);

    
    L4_Clear (&ctrlxfer_msg);
    L4_Load (&ctrlxfer_msg);
    tag = L4_ReplyWait (exc_tid, &tid);

    kill_thread( exc_tid );

}

void all_arch_tests( void )
{
    exception_test();
    ctrlxfer_test();
}

static struct menuitem menu_items[] =
{
    { NULL,		"return" },
    { exception_test,	"Exception test" },
    { ctrlxfer_test,	"CtrlXfer test" },
    { all_arch_tests,	"All IA32 tests" }
};

static struct menu menu = 
{
    "IA32 Menu",
    0,
    NUM_ITEMS( menu_items ),
    menu_items
};

void arch_test(void)
{
    menu_input( &menu );
}
