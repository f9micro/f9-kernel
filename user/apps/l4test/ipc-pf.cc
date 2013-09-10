/*********************************************************************
 *                
 * Copyright (C) 2002, 2003, 2007,  Karlsruhe University
 *                
 * File path:     l4test/ipc-pf.cc
 * Description:   String IPC test with pagefaults
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
 * $Id: ipc-pf.cc,v 1.7 2003/09/24 19:05:54 skoglund Exp $
 *                
 ********************************************************************/
#include <l4/space.h>
#include <l4/ipc.h>
#include <l4/kip.h>
#include <l4io.h>
#include <config.h>

#include <arch.h>

#include "l4test.h"
#include "menu.h"
#include "assert.h"

void setup_ipc_threads (void (*f1)(void), void (*f2)(void),
			bool rcv_same_space, bool snd_same_space,
			bool xcpu = false);

extern L4_ThreadId_t ipc_t1;
extern L4_ThreadId_t ipc_t2;

extern L4_Word_t ipc_pf_block_address;
extern L4_Word_t ipc_pf_abort_address;

static bool snd_ok = false;
static bool rcv_ok = false;

static unsigned char * t1_buf;
static unsigned char * t2_buf;

enum access_e {
    no_access		= 0,
    read_access		= 1,
    write_access	= 2,
    rw_access		= 3,
};

enum fault_e {
    nofault,
    cutmessage,
    xfer_to_snd,
    xfer_to_rcv,
    aborted,
};

#define NOINLINE __attribute__ ((noinline))
static void setup_t1_mappings (access_e p1, access_e p2) NOINLINE;
static void setup_t2_mappings (access_e p1, access_e p2) NOINLINE;
static bool rcv_string_checks (L4_MsgTag_t tag,
			       L4_Msg_t * msg,
			       L4_Word_t cutpoint = PAGE_SIZE*2,
			       fault_e fault = nofault) NOINLINE;
static bool snd_string_checks (L4_MsgTag_t tag,
			       L4_Word_t cutpoint = PAGE_SIZE*2,
			       fault_e fault = nofault) NOINLINE;

static void string_ipc_pf_t1_1 (void)
{ 
    L4_MsgBuffer_t msgbuf;
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    L4_Set_XferTimeouts (L4_Timeouts (L4_Never, L4_Never));

    // Simple string transfer (no page faults)
    setup_t1_mappings (rw_access, rw_access);
    L4_Clear (&msgbuf);
    L4_Append (&msgbuf, L4_StringItem (PAGE_SIZE*2, t1_buf));
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg);
    print_result ("Simple string transfer (no page faults)", rcv_ok && snd_ok);

    // Single sender pagefault
    setup_t1_mappings (rw_access, rw_access);
    L4_Clear (&msgbuf);
    L4_Append (&msgbuf, L4_StringItem (PAGE_SIZE*2, t1_buf));
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg);
    print_result ("Single sender pagefault", rcv_ok && snd_ok);

    // Single receiver pagefault
    setup_t1_mappings (no_access, rw_access);
    L4_Clear (&msgbuf);
    L4_Append (&msgbuf, L4_StringItem (PAGE_SIZE*2, t1_buf));
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg);
    print_result ("Single receiver pagefault", rcv_ok && snd_ok);

    // Multiple sender and receiver pagefaults
    setup_t1_mappings (no_access, no_access);
    L4_Clear (&msgbuf);
    L4_Append (&msgbuf, L4_StringItem (PAGE_SIZE*2, t1_buf));
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg);
    print_result ("Multiple sender and receiver pagefaults", rcv_ok && snd_ok);

    // Long xfer timeouts
    L4_Set_XferTimeouts (L4_Timeouts (L4_Never, L4_TimePeriod (1000*1000*30)));
    setup_t1_mappings (rw_access, rw_access);
    L4_Clear (&msgbuf);
    L4_Append (&msgbuf, L4_StringItem (PAGE_SIZE*2, t1_buf));
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg);
    print_result ("Xfer timeouts (no timeout)", rcv_ok && snd_ok);

    // Zero xfer timeouts
    L4_Set_XferTimeouts (L4_Timeouts (L4_Never, L4_ZeroTime));
    setup_t1_mappings (rw_access, no_access);
    L4_Clear (&msgbuf);
    L4_Append (&msgbuf, L4_StringItem (PAGE_SIZE*2, t1_buf));
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);
    ipc_pf_block_address = (L4_Word_t) &t1_buf[PAGE_SIZE];

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    ipc_pf_block_address = 0;
    rcv_ok = rcv_string_checks (tag, &msg, PAGE_SIZE, xfer_to_snd);
    print_result ("Zero xfer timeouts", rcv_ok && snd_ok);

    // Sender xfer timeouts
    L4_Set_XferTimeouts (L4_Timeouts (L4_Never, L4_TimePeriod (1000*1000*30)));
    setup_t1_mappings (rw_access, rw_access);
    L4_Clear (&msgbuf);
    L4_Append (&msgbuf, L4_StringItem (PAGE_SIZE*2, t1_buf));
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, PAGE_SIZE, xfer_to_snd);
    print_result ("Sender xfer timeout", rcv_ok && snd_ok);

    // Receiver xfer timeouts
    L4_Set_XferTimeouts (L4_Timeouts (L4_Never, L4_TimePeriod (1000*1000)));
    setup_t1_mappings (rw_access, rw_access);
    L4_Clear (&msgbuf);
    L4_Append (&msgbuf, L4_StringItem (PAGE_SIZE*2, t1_buf));
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, PAGE_SIZE, xfer_to_rcv);
    print_result ("Receiver xfer timeout", rcv_ok && snd_ok);

    // Sender abort
    L4_Set_XferTimeouts (L4_Timeouts (L4_Never, L4_Never));
    setup_t1_mappings (rw_access, rw_access);
    L4_Clear (&msgbuf);
    L4_Append (&msgbuf, L4_StringItem (PAGE_SIZE*2, t1_buf));
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, PAGE_SIZE, aborted);
    print_result ("Sender abort", rcv_ok && snd_ok);

    L4_Set_XferTimeouts (L4_Timeouts (L4_Never, L4_Never));

    // Get oneself killed
    L4_Set_MsgTag (L4_Niltag);
    L4_Call (L4_Pager ());
}

static void string_ipc_pf_t1_2 (void)
{ 
    L4_MsgBuffer_t msgbuf;
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    // Receiver abort
    L4_Set_XferTimeouts (L4_Timeouts (L4_Never, L4_Never));
    setup_t1_mappings (rw_access, no_access);
    L4_Clear (&msgbuf);
    L4_Append (&msgbuf, L4_StringItem (PAGE_SIZE*2, t1_buf));
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);
    ipc_pf_abort_address = (L4_Word_t) &t1_buf[PAGE_SIZE];

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    ipc_pf_abort_address = 0;
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, PAGE_SIZE, aborted);
    print_result ("Receiver abort", rcv_ok && snd_ok);

    // Get oneself killed
    L4_Set_MsgTag (L4_Niltag);
    L4_Call (L4_Pager ());
}


static void string_ipc_pf_t2_1 (void)
{
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    L4_Set_XferTimeouts (L4_Timeouts (L4_Never, L4_Never));

    // Simple string transfer (no page faults)
    setup_t2_mappings (rw_access, rw_access);
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (PAGE_SIZE*2, t2_buf));
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag);
    L4_Receive (ipc_t1);

    // Single sender pagefault
    setup_t2_mappings (no_access, rw_access);
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (PAGE_SIZE*2, t2_buf));
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag);
    L4_Receive (ipc_t1);

    // Single receiver pagefault
    setup_t2_mappings (rw_access, rw_access);
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (PAGE_SIZE*2, t2_buf));
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag);
    L4_Receive (ipc_t1);

    // Multiple sender and receiver pagefaults
    setup_t2_mappings (no_access, no_access);
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (PAGE_SIZE*2, t2_buf));
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag);
    L4_Receive (ipc_t1);

    // Xfer timeouts (no timeout)
    L4_Set_XferTimeouts (L4_Timeouts (L4_TimePeriod (1000*1000*25), L4_Never));
    setup_t2_mappings (rw_access, no_access);
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (PAGE_SIZE*2, t2_buf));
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag);
    L4_Receive (ipc_t1);

    // Zero xfer timeout
    L4_Set_XferTimeouts (L4_Timeouts (L4_ZeroTime, L4_Never));
    setup_t2_mappings (rw_access, rw_access);
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (PAGE_SIZE*2, t2_buf));
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag, PAGE_SIZE, xfer_to_snd);
    L4_Receive (ipc_t1);

    // Sender xfer timeout
    L4_Set_XferTimeouts (L4_Timeouts (L4_TimePeriod (1000*1000), L4_Never));
    setup_t2_mappings (rw_access, no_access);
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (PAGE_SIZE*2, t2_buf));
    L4_Load (&msg);
    ipc_pf_block_address = (L4_Word_t) &t2_buf[PAGE_SIZE];

    tag = L4_Send (ipc_t1);
    ipc_pf_block_address = 0;
    snd_ok = snd_string_checks (tag, PAGE_SIZE, xfer_to_snd);
    L4_Receive (ipc_t1);

    // Receiver xfer timeout
    L4_Set_XferTimeouts (L4_Timeouts (L4_TimePeriod (1000*1000*30), L4_Never));
    setup_t2_mappings (rw_access, no_access);
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (PAGE_SIZE*2, t2_buf));
    L4_Load (&msg);
    ipc_pf_block_address = (L4_Word_t) &t2_buf[PAGE_SIZE];

    tag = L4_Send (ipc_t1);
    ipc_pf_block_address = 0;
    snd_ok = snd_string_checks (tag, PAGE_SIZE, xfer_to_rcv);
    L4_Receive (ipc_t1);

    // Sender abort
    L4_Set_XferTimeouts (L4_Timeouts (L4_Never, L4_Never));
    setup_t2_mappings (rw_access, no_access);
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (PAGE_SIZE*2, t2_buf));
    L4_Load (&msg);
    ipc_pf_abort_address = (L4_Word_t) &t2_buf[PAGE_SIZE];

    tag = L4_Send (ipc_t1);
    ipc_pf_abort_address = 0;
    snd_ok = snd_string_checks (tag, PAGE_SIZE, aborted);
    L4_Receive (ipc_t1);

    L4_Set_XferTimeouts (L4_Timeouts (L4_Never, L4_Never));

    // Get oneself killed
    L4_Set_MsgTag (L4_Niltag);
    L4_Call (L4_Pager ());
}

static void string_ipc_pf_t2_2 (void)
{
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    // Receiver abort
    L4_Set_XferTimeouts (L4_Timeouts (L4_Never, L4_Never));
    setup_t2_mappings (rw_access, rw_access);
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (PAGE_SIZE*2, t2_buf));
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag, PAGE_SIZE, aborted);
    L4_Receive (ipc_t1);

    // Get oneself killed
    L4_Set_MsgTag (L4_Niltag);
    L4_Call (L4_Pager ());
}


static bool rcv_string_checks (L4_MsgTag_t tag,
			       L4_Msg_t * msg,
			       L4_Word_t cutpoint,
			       fault_e fault)
{
    L4_StringItem_t * str;
    bool r = true;

    if (fault != nofault)
    {
	if (L4_IpcSucceeded (tag)
	    ||
	    (fault == xfer_to_snd && ((L4_ErrorCode () & 0x1) != 1 ||
				      ((L4_ErrorCode () >> 1) & 0x7) != 6))
	    ||
	    (fault == xfer_to_rcv && ((L4_ErrorCode () & 0x1) != 1 ||
				      ((L4_ErrorCode () >> 1) & 0x7) != 5))
	    ||
	    (fault == aborted && ((L4_ErrorCode () & 0x1) != 1 ||
				  ((L4_ErrorCode () >> 1) & 0x7) != 7))
	    )
	    printf ("RCV: Incorrectly reported IPC error: %s %s\n",
		    ipc_errorcode (L4_ErrorCode ()),
		    ipc_errorphase (L4_ErrorCode ())),
		r = false;

	if (fault != nofault && (L4_ErrorCode () >> 4) != cutpoint)
	    printf ("RCV: IPC incorrectly cut message at 0x%lx "
		    "(not at 0x%lx)\n",
		    (long) (L4_ErrorCode () >> 4), (long) cutpoint),
		r = false;
    }
    else if (L4_IpcFailed (tag))
	printf ("RCV: IPC error: %s %s\n",
		ipc_errorcode (L4_ErrorCode ()),
		ipc_errorphase (L4_ErrorCode ())),
	    r = false;

    if (L4_UntypedWords (tag) != 0 && L4_TypedWords (tag) != 2)
	printf ("RCV: Wrong message layout: untyped/typed=%d/%d "
		"(should be 0/2)\n",
		(int) L4_UntypedWords (tag), (int) L4_TypedWords (tag)),
	    r = false;

    str = (L4_StringItem_t *) &msg->msg[1];
    if (! L4_StringItem (str) && ! L4_Substrings (str) != 1)
	printf ("RCV: Did not receive simple string item\n"), 
	    r = false;
    
    for (unsigned int i = 0; i < PAGE_SIZE*2; i++)
    {
	unsigned int j = (i >= cutpoint) ? 0xff : i;
	if (t1_buf[i] != (unsigned char) j)
	{
	    printf ("RCV: Wrong message contents in buf[%d]: 0x%x != 0x%x\n",
		    (int) i, t1_buf[i], j);
	    r = false;
	    break;
	}
    }

    return r;
}

static bool snd_string_checks (L4_MsgTag_t tag,
			       L4_Word_t cutpoint,
			       fault_e fault)
{
    bool r = true;

    if (fault != nofault)
    {
	if (L4_IpcSucceeded (tag)
	    ||
	    (fault == xfer_to_snd && ((L4_ErrorCode () & 0x1) != 0 ||
				      ((L4_ErrorCode () >> 1) & 0x7) != 5))
	    ||
	    (fault == xfer_to_rcv && ((L4_ErrorCode () & 0x1) != 0 ||
				      ((L4_ErrorCode () >> 1) & 0x7) != 6))
	    ||
	    (fault == aborted && ((L4_ErrorCode () & 0x1) != 0 ||
				  ((L4_ErrorCode () >> 1) & 0x7) != 7))
	    )
	    printf ("SND: Incorrectly reported IPC error: %s %s\n",
		    ipc_errorcode (L4_ErrorCode ()),
		    ipc_errorphase (L4_ErrorCode ())),
		r = false;

	if (fault != nofault && (L4_ErrorCode () >> 4) != cutpoint)
	    printf ("SND: IPC incorrectly cut message at 0x%lx "
		    "(not at 0x%lx)\n",
		    (long) (L4_ErrorCode () >> 4), (long) cutpoint),
		r = false;
    }
    else if (L4_IpcFailed (tag))
	printf ("SND: IPC error: %s %s\n",
		ipc_errorcode (L4_ErrorCode ()),
		ipc_errorphase (L4_ErrorCode ())),
	    r = false;

    return r;
}

static void revoke_rights (void * addr, access_e access)
{
    L4_Fpage_t fp = L4_Fpage ((L4_Word_t) addr, PAGE_SIZE);

    switch (access)
    {
    case no_access:	fp = fp + L4_FullyAccessible; break;
    case read_access:	fp = fp + L4_Writable; break;
    case write_access:	fp = fp + L4_Readable; break;
    case rw_access:	return;
    }

    L4_Flush (fp);
}

static void setup_t1_mappings (access_e p1, access_e p2)
{
    for (unsigned int i = 0; i < PAGE_SIZE*2; i++)
	t1_buf[i] = 0xff;

    revoke_rights (&t1_buf[0], p1);
    revoke_rights (&t1_buf[PAGE_SIZE], p2);
}

static void setup_t2_mappings (access_e p1, access_e p2)
{
    for (unsigned int i = 0; i < PAGE_SIZE*2; i++)
	t2_buf[i] = i;

    revoke_rights (&t2_buf[0], p1);
    revoke_rights (&t2_buf[PAGE_SIZE], p2);
}


void
string_ipc_pf (void)
{
    printf ("\nInter addres space string copy IPC test (with pagefaults)\n");
    t1_buf = (unsigned char *) get_pages (2, false);
    t2_buf = (unsigned char *) get_pages (2, false);
    setup_ipc_threads (string_ipc_pf_t1_1, string_ipc_pf_t2_1, true, false);
    setup_ipc_threads (string_ipc_pf_t1_2, string_ipc_pf_t2_2, false, true);
}

void
string_smpipc_pf (void)
{
    printf ("\nInter address space string copy SMP IPC test "
	    "(with pagefaults)\n");

    if (L4_NumProcessors (L4_KernelInterface ()) == 1)
    {
	printf ("  [Not a multiprocessor system.]\n");
	return;
    }

    t1_buf = (unsigned char *) get_pages (2, false);
    t2_buf = (unsigned char *) get_pages (2, false);
    setup_ipc_threads (string_ipc_pf_t1_1, string_ipc_pf_t2_1, true, false, true);
    setup_ipc_threads (string_ipc_pf_t1_2, string_ipc_pf_t2_2, false, true, true);
}
