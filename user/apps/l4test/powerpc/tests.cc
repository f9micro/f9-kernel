/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     apps/l4test/powerpc/tests.cc
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
 * $Id$
 *                
 ********************************************************************/

#include <l4/kip.h>
#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/kdebug.h>
#include <l4io.h>
#include <l4/powerpc/arch.h>

#include "arch.h"
#include "../l4test.h"
#include "../menu.h"

#define GENERIC_EXC_MR_IP	0
#define GENERIC_EXC_MR_SP	1
#define GENERIC_EXC_MR_FLAGS	2
#define GENERIC_EXC_MR_NO	3
#define GENERIC_EXC_MR_CODE	4
#define GENERIC_EXC_MR_LOCAL_ID	5
#define GENERIC_EXC_MR_MAX	6

#define GENERIC_EXC_LABEL	(L4_Word_t(((-5 << 4) << 16)) >> 16)

#define SC_EXC_MR_IP		9
#define SC_EXC_MR_SP		10
#define SC_EXC_MR_FLAGS		11
#define SC_EXC_MR_MAX		12

#define SC_EXC_LABEL		(L4_Word_t(((-5 << 4) << 16)) >> 16)

#define GENERIC_EXC_SUCCESS	1
#define SYSCALL_EXC_SUCCESS	2

#define TOT_EXCEPTIONS		10000

//#define dprintf(a...)	printf(a)
#define dprintf(a...)

static L4_ThreadId_t controller_tid;
static L4_ThreadId_t handler_tid;
static L4_ThreadId_t subject_tid;
static L4_Word_t which_test = 0;

static bool ipc_error( L4_MsgTag_t tag )
{
    if( !(tag.X.flags & 8) )
	return false;

    L4_Word_t err = L4_ErrorCode();
    if( err & 1 )
	printf( "IPC receive error: %lu, %lx\n", 
		(err >> 1) & 7, err >> 4 );
    else
	printf( "IPC send error: %lu, %lx\n",
		(err >> 1) & 7, err >> 4 );
    return true;
}

static void except_handler_thread( void )
{
    L4_ThreadId_t tid;
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    for(;;)
    {
	tag = L4_Wait( &tid );

	for(;;)
	{
	    if( ipc_error(tag) )
		break;

    	    L4_Store( tag, &msg );

    	    if( (L4_Label(tag) == GENERIC_EXC_LABEL) && 
    		    (L4_UntypedWords(tag) == GENERIC_EXC_MR_MAX) )
    	    {
		// We have received a generic exception message.
		static L4_Word_t tot = 0;
		int response = GENERIC_EXC_SUCCESS;
    		dprintf( "generic exception: ip %lx, sp %lx, flags %lx\n"
			"                   no %lx, code %lx, local ID %lx\n",
    			L4_Get(&msg, GENERIC_EXC_MR_IP),
    			L4_Get(&msg, GENERIC_EXC_MR_SP),
    			L4_Get(&msg, GENERIC_EXC_MR_FLAGS),
    			L4_Get(&msg, GENERIC_EXC_MR_NO),
    			L4_Get(&msg, GENERIC_EXC_MR_CODE),
    			L4_Get(&msg, GENERIC_EXC_MR_LOCAL_ID)
    		      );

		// Increment the instruction pointer and reply
		// to the excepting thread.
    		L4_MsgPutWord( &msg, GENERIC_EXC_MR_IP,
    			L4_Get(&msg, GENERIC_EXC_MR_IP) + 4 );
		L4_Load( &msg );

		tot++;
		if( tot >= TOT_EXCEPTIONS )
		{
		    tot = 0;

		    tag = L4_Reply( tid );
		    if( ipc_error(tag) )
			response = 0;

		    // Tell the controller thread that we finished the test.
		    tag.raw = 0;
		    L4_Set_Label( &tag, response );
		    L4_Clear( &msg );
		    L4_Set_MsgMsgTag( &msg, tag );
		    tid = controller_tid;
		}
    	    }
    	    else if( (L4_Label(tag) == SC_EXC_LABEL) && 
   		    (L4_UntypedWords(tag) == SC_EXC_MR_MAX) )
    	    {
		// We have received a system call emulation exception.
		static L4_Word_t tot = 0;
		int response = SYSCALL_EXC_SUCCESS;
    		dprintf( "syscall exception from %lx: ip %lx, sp %lx, flags %lx\n",
    			L4_GlobalId(tid).raw,
    			L4_Get(&msg, SC_EXC_MR_IP),
    			L4_Get(&msg, SC_EXC_MR_SP),
    			L4_Get(&msg, SC_EXC_MR_FLAGS)
    		      );

		// Reply to the faulting thread.  Don't touch the
		// instruction pointer!!
		L4_Load( &msg );

		tot++;
		if( tot >= TOT_EXCEPTIONS )
		{
		    tot = 0;

		    tag = L4_Reply( tid );
		    if( ipc_error(tag) )
			response = 0;

		    // Tell the controller thread that we finished the test.
		    tag.raw = 0;
		    L4_Set_Label( &tag, response );
		    L4_Clear( &msg );
		    L4_Set_MsgMsgTag( &msg, tag );
		    tid = controller_tid;
		}
    	    }
    	    else
    	    {
    		printf( "unknown exception from %lx! label %lx, untyped %lx\n",
    			tid.raw,
    			L4_Label(tag), L4_UntypedWords(tag) );

		// Tell the controller thread that we received an
		// unexpected result.
		tag.raw = 0;
		L4_Set_Label( &tag, 0 );
		L4_Clear( &msg );
		L4_Set_MsgMsgTag( &msg, tag );
		tid = controller_tid;
    	    }

    	    L4_Load( &msg );
    	    tag = L4_ReplyWait( tid, &tid );
	}
    }
}

static void subject_thread( void )
{
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    dprintf( "controller tid: %lx, handler tid: %lx\n",
	    L4_GlobalId(controller_tid).raw, handler_tid.raw );

    L4_Set_ExceptionHandler( L4_GlobalId(handler_tid) );

    if( which_test == GENERIC_EXC_SUCCESS )
    {
	// Generate a generic exception.
	for( int i=0; i < TOT_EXCEPTIONS; i++ )
	    __asm__ __volatile__ ("mr %%r0, 0 ; trap" ::: "r0");
	dprintf( "trap complete\n" );
    }
    else
    {
	// Generate a legacy system call exception.
	for( int i=0; i < TOT_EXCEPTIONS; i++ )
	    __asm__ __volatile__ (
	    	    "sc"
	    	    : : : "r0", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
		          "r11", "r12", "cr0", "cr1", "cr5", "cr6", "cr7",
		          "lr", "ctr", "xer"
		    );
       	dprintf( "syscall complete\n" );
    }

    // Tell the controller thread that we finished the exception test.
    tag.raw = 0;
    L4_Set_Label( &tag, which_test );
    L4_Clear( &msg );
    L4_Set_MsgMsgTag( &msg, tag );
    L4_Load( &msg );
    L4_Send( controller_tid );
}

static void exception_tests( L4_Word_t test_choice, const char *test_msg )
{
    L4_ThreadId_t tid;

    //L4_KDB_Enter( "starting" );

    // Start tests and initialize global values.
    which_test = test_choice;
    controller_tid = L4_Myself();
    tid = create_thread( except_handler_thread, false, -1 );
    handler_tid = L4_GlobalId(tid);
    tid = create_thread( subject_thread, false, -1 );
    subject_tid = L4_GlobalId(tid);

    dprintf( "handler tid %lx, subject tid %lx, controller tid %lx\n",
	    handler_tid.raw, subject_tid.raw, L4_Myself().raw );

    /*  Wait for results.
     */
    L4_ThreadId_t next_tid;
    L4_MsgTag_t tag;

    tag = L4_Wait( &tid );
    tid = L4_GlobalId( tid );
    if( ((tid != subject_tid) && (tid != handler_tid))
	    || ipc_error(tag) || (L4_Label(tag) != which_test) )
    {
	print_result( test_msg, false );
	goto clean;
    }
    else if( tid == handler_tid )
	next_tid = subject_tid;
    else if( tid == subject_tid )
	next_tid = handler_tid;

    tag = L4_Wait( &tid );
    tid = L4_GlobalId( tid );
    if( (tid != next_tid)
	    || ipc_error(tag) || (L4_Label(tag) != which_test) )
    {
	print_result( test_msg, false );
	goto clean;
    }

    print_result( test_msg, true );

clean:
    dprintf( "shutting down ...\n" );
    kill_thread( handler_tid );
    kill_thread( subject_tid );
}

static void exception_unwind_test( L4_Word_t test_choice, const char *test_msg)
{
    L4_ThreadId_t tid;

    // Start tests and initialize global values.
    which_test = test_choice;
    controller_tid = L4_Myself();
    handler_tid = L4_Myself();
    tid = create_thread( subject_thread, false, -1 );
    subject_tid = L4_GlobalId(tid);

    /*  Wait for results.
     */
    L4_MsgTag_t tag;
    L4_Word_t label =
	(which_test == GENERIC_EXC_SUCCESS) ?  GENERIC_EXC_LABEL:SC_EXC_LABEL;

    tag = L4_Wait( &tid );
    tid = L4_GlobalId( tid );
    if( (tid != subject_tid) 
	    || ipc_error(tag) || (L4_Label(tag) != label) )
    {
	print_result( test_msg, false );
	goto clean;
    }

    kill_thread( subject_tid );
    print_result( test_msg, true );
    return;

clean:
    dprintf( "shutting down ...\n" );
    kill_thread( subject_tid );
}


static void unhandled_exception_thread( void )
{
    __asm__ __volatile__ (
	    "mr %%r0, 0 ;\n\t"
	    ".globl __trigger ;\n\t"
	    "__trigger:\n\t"
	    "trap" ::: "r0");

    dprintf( "resumed after an unhandled exception\n" );

    // Tell the controller that we have finished.
    L4_Msg_t msg;
    L4_Clear( &msg );
    L4_Set_Label( &msg.tag, 0 );
    L4_Load( &msg );
    L4_Send( controller_tid );
}

static void unhandled_exception_test( void )
{
    controller_tid = L4_Myself();

    L4_ThreadId_t tid = create_thread( unhandled_exception_thread, false, -1 );
    subject_tid = L4_GlobalId(tid);

    L4_ThreadSwitch( subject_tid );

    L4_Word_t control = 0, sp = 0, ip = 0, flags = 0, handle = 0;
    L4_ThreadId_t pager;
    tid = L4_ExchangeRegisters( subject_tid, control, sp, ip, flags, handle,
	    pager, &control, &sp, &ip, &flags, &handle, &pager );

    if( L4_IsNilThread(tid) )
    {
	print_result( "Unhandled exception test", false );
	goto clean;
    }
    else
    {
	extern char __trigger[];
	L4_Word_t trigger_ip = L4_Word_t(__trigger);
	print_result( "Unhandled exception test", (trigger_ip == ip) );
    }

    // Restart the halted thread, after the faulting instruction.
    ip += 4;
    L4_Start( L4_LocalId(subject_tid), sp, ip );

    // Wait for the subject thread to finish.
    L4_MsgTag_t tag;
    tag = L4_Wait( &tid );
    tid = L4_GlobalId( tid );
    print_result( "Unhandled exception resume", 
	    (tid == subject_tid) && (L4_Label(tag) == 0) );

clean:
    dprintf( "shutting down ...\n" );
    kill_thread( subject_tid );
}

static void generic_exc_test( void )
{
    exception_tests( GENERIC_EXC_SUCCESS, "Generic exception test" );
}

static void generic_unwind_test( void )
{
    exception_unwind_test( GENERIC_EXC_SUCCESS, "Generic exception unwind" );
}

static void syscall_exc_test( void )
{
    exception_tests( SYSCALL_EXC_SUCCESS, "Legacy system call exception test");
}

static void syscall_unwind_test( void )
{
    exception_unwind_test( SYSCALL_EXC_SUCCESS, 
	    "Legacy system call exception unwind");
}

/**********************************************************************
 * hypervisor tests
 **********************************************************************/

#define SPR_XER		1
#define SPR_LR		8
#define SPR_CTR		9
#define SPR_DSISR	18
#define SPR_DAR		19
#define SPR_DEC		22
#define SPR_SDR1	25
#define SPR_SRR0	26
#define SPR_SRR1	27
#define SPR_SPRG0	272
#define SPR_SPRG1	273
#define SPR_SPRG2	274
#define SPR_SPRG3	275
#define SPR_SPRG4	276
#define SPR_SPRG5	277
#define SPR_SPRG6	278
#define SPR_SPRG7	279
#define SPR_ASR		280
#define SPR_EAR		282
#define SPR_PVR		287


#define TEST_MFSPR(x)     asm volatile("mfspr 0, %0\n" : :"i"(x))

inline L4_Word_t fault_id(L4_MsgTag_t tag)
{
    return 0x1000 - (L4_Label(tag) >> 4);
}

static void vmm_thread( void )
{
    L4_ThreadId_t stid, dtid = L4_nilthread;
    L4_MsgTag_t tag;
    L4_Msg_t msg;
    L4_Fpage_t fp;
    L4_Word_t fault;

    printf("VMM thread started (%lx)\n", L4_Myself().raw);

    for(;;)
    {
	tag = L4_ReplyWait( dtid, &stid );

	if( ipc_error(tag) ) {
	    printf("IPC error\n");
	    dtid = L4_nilthread;
	    continue;
	}

	L4_Store (tag, &msg);

	printf("got message from %lx (tag: %x, l: %x/%d, t:%d, u:%d)\n", 
	       stid.raw, tag.raw, L4_Label(tag), fault_id(tag),
	       L4_TypedWords (tag), L4_UntypedWords (tag));

	fault = fault_id(tag);

	if (fault == 2 && L4_UntypedWords(tag) == 2 && L4_TypedWords (tag) == 0)
	{
	    /* pagefault */
	    fp = L4_FpageLog2 (L4_Get (&msg, 0), PAGE_BITS) + L4_FullyAccessible;
	    L4_Clear (&msg);
	    L4_Append (&msg,  L4_MapItem (fp, L4_Address(fp)));
	    L4_Load (&msg);
	    dtid = stid;
	}
	else if (fault >= 4 && fault < 20)
	{
	    L4_Word_t ip = L4_Get(&msg, 1);
	    L4_GPRegsXCtrlXferItem_t gprx_item;

	    /* virtualization fault */
	    printf("virtualization fault %d\n", fault - 4);
	    printf("ip: %08x, instr: %08x, msr: %08x\n",
		   L4_Get(&msg, 0), ip, L4_Get(&msg, 2));
	    
	    L4_Init(&gprx_item);
	    L4_Set(&gprx_item, L4_CTRLXFER_GPREGS_IP, ip + 4);
	    L4_Clear(&msg);
	    L4_Append(&msg, &gprx_item);
	    L4_Load (&msg);
	    dtid = stid;
	}
	else
	    break;
    }

    printf("exiting vmm thread\n");
}

static void vm_test_thread( void )
{
#if 0
    TEST_MFSPR(1); TEST_MFSPR(8); TEST_MFSPR(9); TEST_MFSPR(18);
    TEST_MFSPR(19); TEST_MFSPR(22); TEST_MFSPR(272); TEST_MFSPR(273);
    TEST_MFSPR(274); TEST_MFSPR(275); TEST_MFSPR(276); TEST_MFSPR(277);
    TEST_MFSPR(278); TEST_MFSPR(279);
    // booke
    TEST_MFSPR(0x030); TEST_MFSPR(0x036); TEST_MFSPR(0x03a); TEST_MFSPR(0x03b);
    TEST_MFSPR(0x03d); TEST_MFSPR(0x03e); TEST_MFSPR(0x03f); TEST_MFSPR(0x100);
    TEST_MFSPR(0x104); TEST_MFSPR(0x105); TEST_MFSPR(0x106); TEST_MFSPR(0x107);
    TEST_MFSPR(0x10c); TEST_MFSPR(0x10d); TEST_MFSPR(0x11e); TEST_MFSPR(0x130);
    TEST_MFSPR(0x134); TEST_MFSPR(0x135); TEST_MFSPR(0x136); TEST_MFSPR(0x138);
    TEST_MFSPR(0x139); TEST_MFSPR(0x13a); TEST_MFSPR(0x13b); TEST_MFSPR(0x13c);
    TEST_MFSPR(0x13d); TEST_MFSPR(0x13e); TEST_MFSPR(0x13f); TEST_MFSPR(0x150);
    TEST_MFSPR(0x154); 
    // ivors
    TEST_MFSPR(0x190); TEST_MFSPR(0x191); TEST_MFSPR(0x192); TEST_MFSPR(0x193); 
    TEST_MFSPR(0x194); TEST_MFSPR(0x195); TEST_MFSPR(0x196); TEST_MFSPR(0x197); 
    TEST_MFSPR(0x198); TEST_MFSPR(0x199); TEST_MFSPR(0x19a); TEST_MFSPR(0x19b); 
    TEST_MFSPR(0x19c); TEST_MFSPR(0x19d); TEST_MFSPR(0x19e); TEST_MFSPR(0x19f); 

    TEST_MFSPR(0x23a); TEST_MFSPR(0x23b); TEST_MFSPR(0x23c); TEST_MFSPR(0x370);
    TEST_MFSPR(0x371); TEST_MFSPR(0x372); TEST_MFSPR(0x373); TEST_MFSPR(0x374);
    TEST_MFSPR(0x375); TEST_MFSPR(0x376); TEST_MFSPR(0x377); TEST_MFSPR(0x378);
    TEST_MFSPR(0x390); TEST_MFSPR(0x391); TEST_MFSPR(0x392); TEST_MFSPR(0x393);
    TEST_MFSPR(0x394); TEST_MFSPR(0x395); TEST_MFSPR(0x396); TEST_MFSPR(0x397);
    TEST_MFSPR(0x398); TEST_MFSPR(0x399); TEST_MFSPR(0x39b); TEST_MFSPR(0x39c);
    TEST_MFSPR(0x39d); TEST_MFSPR(0x39e); TEST_MFSPR(0x39f); TEST_MFSPR(0x3b2);
    TEST_MFSPR(0x3b3); TEST_MFSPR(0x3d3); TEST_MFSPR(0x3f3);
#endif

    while(1) {
	asm("mtdcrx %0, %1\n" : : "r"(0xc00), "r"(0x1234));
	asm("mfdcrx %0, %1\n" : : "r"(0xc00), "r"(0x1234));
    }
}

static void hvm_test( void )
{
    L4_ThreadId_t t1;
    L4_Msg_t ctrlxfer_msg;
 

    if (!l4_has_feature("powerpc-hvm")) 
    {
	printf("Kernel doesn't support hypervisor feature\n");
	return;
    }

    t1 = create_thread(true, -1, 1);

    printf("setting ctrlxfer\n");
    /* Set ctrlxfer mask */
    L4_Word64_t fault_id_mask = (1 << L4_FAULT_HVM_PROGRAM);
    L4_Word_t fault_mask = L4_CTRLXFER_FAULT_MASK(L4_CTRLXFER_GPREGS0) | 
	L4_CTRLXFER_FAULT_MASK(L4_CTRLXFER_GPREGS1) | 
	L4_CTRLXFER_FAULT_MASK(L4_CTRLXFER_GPREGSX);
    L4_Clear(&ctrlxfer_msg);
    L4_AppendFaultConfCtrlXferItems(&ctrlxfer_msg, fault_id_mask, fault_mask, 0);
    L4_Load(&ctrlxfer_msg);
    L4_ConfCtrlXferItems(t1);

    printf("starting thread\n");
    start_thread (t1, vm_test_thread);
    vmm_thread();
    kill_thread(t1);
}


void all_arch_tests( void )
{
    generic_exc_test();
    syscall_exc_test();
    generic_unwind_test();
    syscall_unwind_test();
    hvm_test();
    unhandled_exception_test();
}

static struct menuitem menu_items[] =
{
    { NULL,			"return" },
    { generic_exc_test,		"Generic exception test" },
    { syscall_exc_test,		"Legacy system call exception test" },
    { generic_unwind_test,	"Generic exception unwind" },
    { syscall_unwind_test,	"Legacy system call exception unwind" },
    { unhandled_exception_test,	"Unhandled exception test" },
    { hvm_test,			"hypervisor mode test" },
    { all_arch_tests,		"All PowerPC tests" },
};


static struct menu menu = 
{
    "PowerPC Menu",
    0,
    NUM_ITEMS( menu_items ),
    menu_items
};

void arch_test(void)
{
    menu_input( &menu );
}
