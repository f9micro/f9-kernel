/*********************************************************************
 *                
 * Copyright (C) 2002, 2003, 2007, 2010,  Karlsruhe University
 *                
 * File path:     l4test/ipc.cc
 * Description:   Various IPC tests
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
 * $Id: ipc.cc,v 1.15 2003/10/28 02:55:58 cvansch Exp $
 *                
 ********************************************************************/
#include <l4/ipc.h>
#include <l4/space.h>
#include <l4/thread.h>
#include <l4/arch.h>
#include <l4/kdebug.h>
#include <l4io.h>
#include <config.h>

#include <arch.h>

#include "l4test.h"
#include "menu.h"
#include "assert.h"

__USER_BSS L4_ThreadId_t ipc_t1;
__USER_BSS L4_ThreadId_t ipc_t2;
__USER_BSS bool ipc_ok;

__USER_BSS L4_Word_t ipc_pf_block_address = 0;
__USER_BSS L4_Word_t ipc_pf_abort_address = 0;

__USER_TEXT L4_Word_t Word (L4_ThreadId_t t) { return t.raw; }

__USER_TEXT const char * ipc_errorcode (L4_Word_t errcode)
{
    switch ((errcode >> 1) & 0x7)
    {
    case 0: return "ok (0)";
    case 1: return "timeout";
    case 2: return "non-existing partner";
    case 3: return "cancelled";
    case 4: return "message overflow";
    case 5: return "xfer timeout (current)";
    case 6: return "xfer timeout (partner)";
    case 7: return "aborted";
    }
    return "unknown";
}

__USER_TEXT const char * ipc_errorphase (L4_Word_t errcode)
{
    return errcode & 0x1 ? "receive-phase" : "send-phase";
}


__USER_TEXT void setup_ipc_threads (void (*f1)(void), void (*f2)(void),
			bool rcv_same_space, bool snd_same_space,
			bool xcpu)
{
    int threads_to_kill;

    ipc_pf_block_address = 0;
    
    ipc_t1 = create_thread (NULL, !rcv_same_space, -1, 0);
    ipc_t2 = create_thread (NULL, !snd_same_space,
			    xcpu ? ((L4_ProcessorNo () + 1) % 2) : (L4_Word_t) -1, 0);


    // Do not start threads unless both threads have been created.
    start_thread (ipc_t1, f1);
    start_thread (ipc_t2, f2);

    threads_to_kill = 2;

    for (;;)
    {
	L4_Msg_t msg;
	L4_ThreadId_t tid;
	L4_MsgTag_t tag = L4_Wait (&tid);

	for (;;)
	{
	    L4_Fpage_t fp;

	    L4_MsgStore (tag, &msg);

	    if (L4_UntypedWords (tag) != 2 || L4_TypedWords (tag) != 0 ||
		!L4_IpcSucceeded (tag))
	    {
		kill_thread (tid);
		if (--threads_to_kill == 0)
		    return;
		break;
	    }

	    // Check if we should abort thread doing pagefault on
	    // current address.
	    if (L4_MsgWord (&msg, 0) == ipc_pf_abort_address)
	    {
		L4_Word_t dw;
		L4_ThreadId_t dt;
		L4_ExchangeRegisters (tid, (3 << 1), 0, 0, 0, 0, L4_nilthread,
				      &dw, &dw, &dw, &dw, &dw, &dt);
		ipc_pf_abort_address = 0;
		break;
	    }

	    // Check if we should not serve the pagefault at this
	    // address.
	    if (L4_MsgWord (&msg, 0) == ipc_pf_block_address)
		break;


	    L4_MsgClear (&msg);
	    fp.raw = L4_MsgWord(&msg, 0);
	    
#if defined(L4_ARCH_IA32) || defined(L4_ARCH_AMD64)
#define L4_IO_PAGEFAULT		(-8UL << 20)
#define L4_REQUEST_MASK		( ~((~0UL) >> ((sizeof (L4_Word_t) * 8) - 20)))
	    // If pagefault is an IO-Page, return that IO Fpage 
	    if ((tag.raw & L4_REQUEST_MASK) != L4_IO_PAGEFAULT || !L4_IsIoFpage(fp))
#endif
	    {
		// Touch memory
		volatile L4_Word_t * mem = (L4_Word_t *)
		    (L4_MsgWord (&msg, 0) & ~(sizeof (L4_Word_t) - 1));
		*mem = *mem;
		
		fp = L4_FpageAddRights(L4_FpageLog2 (L4_MsgWord (&msg, 0), PAGE_BITS), L4_FullyAccessible);
	    }	    
	    
	    if ((tid.raw == ipc_t1.raw && !rcv_same_space) ||
		(tid.raw == ipc_t2.raw && !snd_same_space))
		L4_MsgAppendMapItem (&msg,  L4_MapItem (fp, L4_MsgWord (&msg, 0)));
	    L4_MsgLoad (&msg);
	    tag = L4_ReplyWait (tid, &tid);
	}
    }
}



/*
**
** IPC test with only untyped words
**
*/

__USER_TEXT static void simple_ipc_t1_l (void)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    L4_Word_t i, w;
    L4_ThreadId_t tid;
    unsigned char * buf;
    L4_Fpage_t fp;
    L4_ThreadId_t from;

    // Correct message contents
    ipc_ok = true;
    
    for (L4_Word_t n = 0; n <= 63; n++)
    {
	for (L4_Word_t k = 1; k <= 63; k++)
	    L4_LoadMR (k, 0);
	tag = L4_Receive (ipc_t2);
        if (!L4_IpcSucceeded (tag))
	{
            printf ("Xfer %d words -- IPC failed %s %s\n", (int) n,
		    ipc_errorcode (L4_ErrorCode ()),
		    ipc_errorphase (L4_ErrorCode ()));
	    ipc_ok = false;
            break;
	}

	L4_MsgStore (tag, &msg);
	if (L4_Label (tag) != 0xf00f)
	{
	    printf ("Xfer %d words -- wrong label: 0x%lx != 0xf00f\n",
		    (int) n, (long) L4_Label (tag));
	    ipc_ok = false;
            break;
	}

	for (i = 1; i <= n; i++)
	{
	    L4_Word_t val = L4_MsgWord (&msg, i - 1);
	    if (val != i)
	    {
		printf ("Xfer %d words -- wrong value in MR[%d]: "
			"0x%lx != 0x%lx\n",
			(int) n, (int) i, (long) val, (long) i);
		ipc_ok = false;
	    }
            if (!ipc_ok) 
                break;
	}
        if (!ipc_ok) 
            break;
    }
    
    print_result ("Send Message transfer", ipc_ok);

    // Correct message contents
    ipc_ok = true;
    tag = L4_Call (ipc_t2);
    
    for (L4_Word_t n = 0; n <= 63; n++)
    {
	L4_MsgStore (tag, &msg);
	if (L4_Label (tag) != 0xf00d)
	{
            L4_KDB_Enter("2 label");
	    printf ("Xfer %d words -- wrong label: 0x%lx != 0xf00d\n",
		    (int) n, (long) L4_Label (tag));
                ipc_ok = false;
                break;
	}
	for (i = 1; i <= n; i++)
	{
	    L4_Word_t val = L4_MsgWord (&msg, i - 1);
	    if (val != i)
	    {
		printf ("Xfer %d words -- wrong value in MR[%d]: "
			"0x%lx != 0x%lx\n",
			(int) n, (int) i, (long) val, (long) i);
		ipc_ok = false;
                break;
            }
            if (!ipc_ok) 
                break;
            
	}
        
        if (n == 63)
            break;
        
	for (L4_Word_t k = 1; k <= 63; k++)
	    L4_LoadMR (k, 0);
	tag = L4_ReplyWait (ipc_t2, &tid);

        if (!L4_IpcSucceeded (tag))
        {
            printf ("Xfer %d words -- IPC failed %s %s\n", (int) n,
                    ipc_errorcode (L4_ErrorCode ()),
                    ipc_errorphase (L4_ErrorCode ()));
            ipc_ok = false;
            break;
        }

    }
    print_result ("ReplyWait Message transfer", ipc_ok);

    // Send timeout
    L4_Set_MsgTag (L4_Niltag);
    tag = L4_Send_Timeout (ipc_t2, L4_TimePeriod (1000*1000));
    ipc_ok = true;
    if (L4_IpcSucceeded (tag))
    {
	printf ("SND: IPC send incorrectly succeeded\n");
	ipc_ok = false;
    }
    else
    {
	if ((L4_ErrorCode () & 0x1) || ((L4_ErrorCode () >> 1) & 0x7) != 1)
	{
	    printf ("SND: Incorrect error code: %s %s\n",
		    ipc_errorcode (L4_ErrorCode ()),
		    ipc_errorphase (L4_ErrorCode ()));
	    ipc_ok = false;
	}
    }
    print_result ("Send timeout", ipc_ok);
    L4_Receive (ipc_t2);

    // Receive timeout
    tag = L4_Receive_Timeout (ipc_t2, L4_TimePeriod (1000*1000));
    ipc_ok = true;
    if (L4_IpcSucceeded (tag))
    {
	printf ("RCV: IPC receive incorrectly succeeded\n");
	ipc_ok = false;
    }
    else
    {
	if ((L4_ErrorCode () & 0x1) == 0 ||
	    ((L4_ErrorCode () >> 1) & 0x7) != 1)
	{
	    printf ("RCV: Incorrect error code: %s %s\n",
		    ipc_errorcode (L4_ErrorCode ()),
		    ipc_errorphase (L4_ErrorCode ()));
	    ipc_ok = false;
	}
    }
    print_result ("Receive timeout", ipc_ok);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);

    // Local destintation Id
    tag = L4_Receive_Timeout (ipc_t2, L4_TimePeriod (5*1000*1000));
    print_result ("Local destination Id", L4_IpcSucceeded (tag));
    L4_Set_MsgTag (L4_Niltag);
    tag = L4_Send (ipc_t2);

    // Send cancel
    L4_Set_MsgTag (L4_Niltag);
    tag = L4_Send (ipc_t2);
    ipc_ok = true;
    if (L4_IpcSucceeded (tag))
    {
	printf ("SND: IPC send incorrectly succeeded\n");
	ipc_ok = false;
    }
    else
    {
	if ((L4_ErrorCode () & 0x1) || ((L4_ErrorCode () >> 1) & 0x7) != 3)
	{
	    printf ("SND: Incorrect error code: %s %s\n",
		    ipc_errorcode (L4_ErrorCode ()),
		    ipc_errorphase (L4_ErrorCode ()));
	    ipc_ok = false;
	}
    }
    print_result ("Send cancelled", ipc_ok);
    L4_Receive (ipc_t2);

    // Receive cancel
    tag = L4_Call(ipc_t2);
    ipc_ok = true;
    if (L4_IpcSucceeded (tag))
    {
	printf ("RCV: IPC receive incorrectly succeeded\n");
	ipc_ok = false;
    }
    else
    {
	if ((L4_ErrorCode () & 0x1) == 0 ||
	    ((L4_ErrorCode () >> 1) & 0x7) != 3)
	{
	    printf ("RCV: Incorrect error code: %s %s\n",
		    ipc_errorcode (L4_ErrorCode ()),
		    ipc_errorphase (L4_ErrorCode ()));
	    ipc_ok = false;
	}
    }
    print_result ("Receive cancelled", ipc_ok);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);

    // Cancel pagefault
    buf = (unsigned char *) get_pages (1, false);
    fp = L4_FpageAddRights(L4_Fpage ((L4_Word_t) buf, PAGE_SIZE),
			 L4_FullyAccessible);
    L4_Flush (fp);
    ipc_pf_abort_address = (L4_Word_t) buf;

    for (i = 0; i <= 63; i++)
	L4_LoadMR (i, i+1);

    *buf = 0xff;

    ipc_ok = true;
    for (i = 0; i <= 63; i++)
    {
	L4_StoreMR (i, &w);
	if (w != i + 1)
	{
	    printf ("Wrong value in MR[%d]: 0x%lx != 0x%lx\n", (int) i,
		    (long) w, (long) i+1);
	    ipc_ok = false;
	    break;
	}
    }
    print_result ("Pagefault cancelled", ipc_ok);

    // From parameter (local)
    tag = L4_Wait (&from);
    ipc_ok = true;
    if (from.raw != L4_LocalIdOf (ipc_t2).raw)
    {
	printf ("Returned Id %lx != %lx (local) [%lx (global)]\n",
		Word (from), Word (L4_LocalIdOf (ipc_t2)), Word (ipc_t2));
	ipc_ok = false;
    }
    print_result ("From parameter (local)", ipc_ok);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);

    // Get oneself killed
    L4_Set_MsgTag (L4_Niltag);
    L4_Call (L4_Pager ());
}

#if 0
__USER_TEXT static void simple_ipc_t1_g (void)
{
    // From parameter (global)
    L4_ThreadId_t from;
    ipc_ok = true;
    if (from.raw != ipc_t2.raw)
    {
	printf ("Returned Id %lx != %lx\n", Word (from), Word (ipc_t2));
	ipc_ok = false;
    }
    print_result ("From parameter (global)", ipc_ok);

    // Get oneself killed
    L4_Set_MsgTag (L4_Niltag);
    L4_Call (L4_Pager ());
}
#endif

__USER_TEXT static void simple_ipc_t2_l (void)
{
    L4_Msg_t msg;
    L4_Word_t dw;
    L4_ThreadId_t dt;
    L4_MsgTag_t tag;
        

    // Message contents
    for (L4_Word_t n = 0; n <= 63; n++)
    {
	L4_MsgClear (&msg);
	L4_Set_Label (&msg.tag, 0xf00f);
	for (L4_Word_t i = 1; i <= n; i++)
	    L4_MsgAppendWord (&msg, i);
	L4_MsgLoad (&msg);
	tag = L4_Send (ipc_t1);
        if (!L4_IpcSucceeded (tag))
        {
            printf ("Xfer %d words -- IPC failed %s %s\n", (int) n,
                    ipc_errorcode (L4_ErrorCode ()),
                    ipc_errorphase (L4_ErrorCode ()));
            ipc_ok = false;
            break;
        }

    }

    L4_Receive (ipc_t1);

    // Message contents
    for (L4_Word_t n = 0; n <= 63; n++)
    {
	L4_MsgClear (&msg);
	L4_Set_Label (&msg.tag, 0xf00d);
	for (L4_Word_t i = 1; i <= n; i++)
	    L4_MsgAppendWord (&msg, i);
	L4_MsgLoad (&msg);
	if (n == 63)
	    tag = L4_Send(ipc_t1);
	else
	    tag = L4_ReplyWait (ipc_t1, &dt);
        
        if (!L4_IpcSucceeded (tag))
        {
            printf ("Xfer %d words -- IPC failed %s %s\n", (int) n,
                    ipc_errorcode (L4_ErrorCode ()),
                    ipc_errorphase (L4_ErrorCode ()));
            ipc_ok = false;
            break;
        }
        
    }

    // Send timeout
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t1);

    // Receive timeout
    L4_Receive (ipc_t1);

    // Local destination Id
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (L4_LocalIdOf (ipc_t1));
    L4_Receive (ipc_t1);

    // Send cancel
    L4_Sleep (L4_TimePeriod (1000*1000));
    L4_ExchangeRegisters (ipc_t1, 0x6,
			  0, 0, 0, 0, L4_nilthread,
			  &dw, &dw, &dw, &dw, &dw, &dt);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t1);

    // Receive cancel
    L4_Receive(ipc_t1);
    L4_Sleep (L4_TimePeriod (1000*1000));
    L4_ExchangeRegisters (ipc_t1, 0x6,
			  0, 0, 0, 0, L4_nilthread,
			  &dw, &dw, &dw, &dw, &dw, &dt);
    L4_Receive (ipc_t1);

    // From parameter (local)
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t1);
    L4_Receive (ipc_t1);

    // Get oneself killed
    L4_Set_MsgTag (L4_Niltag);
    L4_Call (L4_Pager ());
}

#if 0
__USER_TEXT static void simple_ipc_t2_g (void)
{
    // From parameter (global)
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t1);

    // Get oneself killed
    L4_Set_MsgTag (L4_Niltag);
    L4_Call (L4_Pager ());
}
#endif


__USER_TEXT static void simple_ipc (void)
{
    printf ("\nSimple IPC test (inter-as, only untyped words)\n");
#if 0
    setup_ipc_threads (simple_ipc_t1_g, simple_ipc_t2_g, false, false, false);
#endif
    setup_ipc_threads (simple_ipc_t1_l, simple_ipc_t2_l, true, true, false);
}




/*
 * Menu definition
 */
void string_ipc (void);
void string_ipc_pf (void);
void simple_smpipc (void);
void string_smpipc_pf (void);

__USER_TEXT void all_ipc_tests (void)
{
    simple_ipc ();
    //string_ipc ();
    //string_ipc_pf ();
    //simple_smpipc ();
    //string_smpipc_pf ();
}

static struct menuitem menu_items[] = 
{
    { NULL, 		"return" },
    { simple_ipc, 	"Simple IPC" },
    { string_ipc, 
      "String copy IPC, inter address space (no pagefaults)"},
    { string_ipc_pf,
      "String copy IPC, intra address space (with pagefaults)" },
    { simple_smpipc,	"Simple SMP IPC" },
    { string_smpipc_pf,
      "String copy SMP IPC, inter address space (with pagefaults)" },
    { all_ipc_tests,	"All IPC tests" },
};

static struct menu menu = 
{
    "IPC Menu",
    0, 
    NUM_ITEMS (menu_items),
    menu_items
};


/*
 * Entry point
 */
void ipc_test (void)
{
    menu_input (&menu);
}

