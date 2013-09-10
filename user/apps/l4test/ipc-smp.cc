/*********************************************************************
 *                
 * Copyright (C) 2003, 2007, 2010,  Karlsruhe University
 *                
 * File path:     l4test/ipc-smp.cc
 * Description:   Simple SMP IPC tests
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
 * $Id: ipc-smp.cc,v 1.3 2003/09/24 19:05:54 skoglund Exp $
 *                
 ********************************************************************/
#include <l4/ipc.h>
#include <l4/space.h>
#include <l4/kip.h>
#include <l4io.h>
#include <config.h>

#include <arch.h>

#include "l4test.h"
#include "menu.h"


void setup_ipc_threads (void (*f1)(void), void (*f2)(void),
			bool rcv_same_space, bool snd_same_space,
			bool xcpu = false);

extern L4_ThreadId_t ipc_t1;
extern L4_ThreadId_t ipc_t2;

extern L4_Word_t ipc_pf_block_address;
extern L4_Word_t ipc_pf_abort_address;


static void simple_smpipc_t1 (void)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    L4_Word_t i, w;
    bool ok;

    // Correct message contents
    ok = true;
    for (L4_Word_t n = 0; n <= 63; n++)
    {
	for (L4_Word_t k = 1; k <= 63; k++)
	    L4_LoadMR (k, 0);
	tag = L4_Receive (ipc_t2);
	L4_Store (tag, &msg);
	if (L4_Label (tag) != 0xf00f)
	{
	    printf ("Xfer %d words -- wrong label: 0x%lx != 0xf00f\n",
		    (int) n, (long) L4_Label (tag));
	    ok = false;
	}
	if (! L4_IpcXcpu (tag))
	{
	    printf ("Xfer %d words -- not cross-cpu\n", (int) n);
	    ok = false;
	}
	for (i = 1; i <= n; i++)
	{
	    L4_Word_t val = L4_Get (&msg, i - 1);
	    if (val != i)
	    {
		printf ("Xfer %d words -- wrong value in MR[%d]: "
			"0x%lx != 0x%lx\n",
			(int) n, (int) i, (long) val, (long) i);
		ok = false;
		break;
	    }
	}
	L4_Set_MsgTag (L4_Niltag);
	L4_Send (ipc_t2);
    }
    print_result ("Message transfer", ok);

    // Send timeout
    L4_Set_MsgTag (L4_Niltag);
    tag = L4_Send (ipc_t2, L4_TimePeriod (1000*1000));
    ok = true;
    if (L4_IpcSucceeded (tag))
    {
	printf ("SND: IPC send incorrectly succeeded\n");
	ok = false;
    }
    else
    {
	if ((L4_ErrorCode () & 0x1) || ((L4_ErrorCode () >> 1) & 0x7) != 1)
	{
	    printf ("SND: Incorrect error code: %s %s\n",
		    ipc_errorcode (L4_ErrorCode ()),
		    ipc_errorphase (L4_ErrorCode ()));
	    ok = false;
	}
    }
    print_result ("Send timeout", ok);
    L4_Receive (ipc_t2);

    // Receive timeout
    tag = L4_Receive (ipc_t2, L4_TimePeriod (1000*1000));
    ok = true;
    if (L4_IpcSucceeded (tag))
    {
	printf ("RCV: IPC receive incorrectly succeeded\n");
	ok = false;
    }
    else
    {
	if ((L4_ErrorCode () & 0x1) == 0 ||
	    ((L4_ErrorCode () >> 1) & 0x7) != 1)
	{
	    printf ("RCV: Incorrect error code: %s %s\n",
		    ipc_errorcode (L4_ErrorCode ()),
		    ipc_errorphase (L4_ErrorCode ()));
	    ok = false;
	}
    }
    print_result ("Receive timeout", ok);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);

    // Send cancel
    L4_Set_MsgTag (L4_Niltag);
    tag = L4_Send (ipc_t2);
    ok = true;
    if (L4_IpcSucceeded (tag))
    {
	printf ("SND: IPC send incorrectly succeeded\n");
	ok = false;
    }
    else
    {
	if ((L4_ErrorCode () & 0x1) || ((L4_ErrorCode () >> 1) & 0x7) != 3)
	{
	    printf ("SND: Incorrect error code: %s %s\n",
		    ipc_errorcode (L4_ErrorCode ()),
		    ipc_errorphase (L4_ErrorCode ()));
	    ok = false;
	}
    }
    print_result ("Send cancelled", ok);
    L4_Receive (ipc_t2);

    // Receive cancel
    tag = L4_Receive (ipc_t2);
    ok = true;
    if (L4_IpcSucceeded (tag))
    {
	printf ("RCV: IPC receive incorrectly succeeded\n");
	ok = false;
    }
    else
    {
	if ((L4_ErrorCode () & 0x1) == 0 ||
	    ((L4_ErrorCode () >> 1) & 0x7) != 3)
	{
	    printf ("RCV: Incorrect error code: %s %s\n",
		    ipc_errorcode (L4_ErrorCode ()),
		    ipc_errorphase (L4_ErrorCode ()));
	    ok = false;
	}
    }
    print_result ("Receive cancelled", ok);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);

    // Cancel pagefault
    unsigned char * buf = (unsigned char *) get_pages (1, false);
    L4_Fpage_t fp = L4_Fpage ((L4_Word_t) buf, PAGE_SIZE)
	+ L4_FullyAccessible;
    L4_Flush (fp);
    ipc_pf_abort_address = (L4_Word_t) buf;

    for (i = 0; i <= 63; i++)
	L4_LoadMR (i, i+1);

    *buf = 0xff;

    ok = true;
    for (i = 0; i <= 63; i++)
    {
	L4_StoreMR (i, &w);
	if (w != i + 1)
	{
	    printf ("Wrong value in MR[%d]: 0x%lx != 0x%lx\n", (int) i,
		    (long) w, (long) i);
	    ok = false;
	    break;
	}
    }
    print_result ("Pagefault cancelled", ok);

    // Get oneself killed
    L4_Set_MsgTag (L4_Niltag);
    L4_Call (L4_Pager ());
}

static void simple_smpipc_t2 (void)
{
    L4_Msg_t msg;
    L4_Word_t dw;
    L4_ThreadId_t dt;

    // Message contents
    for (L4_Word_t n = 0; n <= 63; n++)
    {
	L4_Clear (&msg);
	L4_Set_Label (&msg, 0xf00f);
	for (L4_Word_t i = 1; i <= n; i++)
	    L4_Append (&msg, i);
	L4_Load (&msg);
	L4_Call (ipc_t1);
    }

    // Send timeout
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t1);

    // Receive timeout
    L4_Receive (ipc_t1);

    // Send cancel
    L4_Sleep (L4_TimePeriod (1000*1000));
    L4_ExchangeRegisters (ipc_t1, 0x6,
			  0, 0, 0, 0, L4_nilthread,
			  &dw, &dw, &dw, &dw, &dw, &dt);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t1);

    // Receive cancel
    L4_Sleep (L4_TimePeriod (1000*1000));
    L4_ExchangeRegisters (ipc_t1, 0x6,
			  0, 0, 0, 0, L4_nilthread,
			  &dw, &dw, &dw, &dw, &dw, &dt);
    L4_Receive (ipc_t1);

    // Get oneself killed
    L4_Set_MsgTag (L4_Niltag);
    L4_Call (L4_Pager ());
}


void simple_smpipc (void)
{
    printf ("\nSimple SMP IPC test (only untyped words)\n");
    if (L4_NumProcessors (L4_KernelInterface ()) == 1)
    {
	printf ("  [Not a multiprocessor system.]\n");
	return;
    }

    setup_ipc_threads (simple_smpipc_t1, simple_smpipc_t2,
		       true, true, true);
}
