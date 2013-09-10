/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     l4test/ipc-string.cc
 * Description:   String IPC test without pagefaults
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
 * $Id: ipc-string.cc,v 1.7 2003/09/24 19:05:54 skoglund Exp $
 *                
 ********************************************************************/
#include <l4/ipc.h>
#include <l4io.h>
#include <config.h>

#include <arch.h>

#include "l4test.h"
#include "menu.h"
#include "assert.h"


extern L4_ThreadId_t ipc_t1;
extern L4_ThreadId_t ipc_t2;

void setup_ipc_threads (void (*f1)(void), void (*f2)(void),
			bool rcv_same_space, bool snd_same_space,
			bool xcpu = false);

static unsigned char t1_buf[256] __attribute__ ((aligned (256)));
static unsigned char t2_buf[256] __attribute__ ((aligned (256)));

static bool snd_ok = false;
static bool rcv_ok = false;

#define NOINLINE __attribute__ ((noinline))
static void setup_strbufs (void) NOINLINE;
static bool check_strbufs (unsigned int cutpoint = 256) NOINLINE;
static bool rcv_string_checks (L4_MsgTag_t tag,
			       L4_Msg_t * msg,
			       L4_Word_t num_typed,
			       L4_Word_t cutpoint = 256,
			       bool cutmessage = false) NOINLINE;
static bool snd_string_checks (L4_MsgTag_t tag,
			       L4_Word_t cutpoint = 256,
			       bool cutmessage = false) NOINLINE;


static void string_ipc_t1 (void)
{
    L4_MsgBuffer_t msgbuf;
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    union {
	L4_Word_t spc[16];
	L4_StringItem_t str;
    };

    spc[0] = 0; // To avoid warning

    // Simple string item
    setup_strbufs ();
    L4_Clear (&msgbuf);
    L4_Append (&msgbuf, L4_StringItem (256, t1_buf));
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, 2);
    print_result ("Simple string transfer", rcv_ok && snd_ok);

    // Simple substrings
    setup_strbufs ();
    L4_Clear (&msgbuf);
    str = L4_StringItem (64, t1_buf);
    str += &t1_buf[64];
    str += &t1_buf[128];
    str += &t1_buf[192];
    L4_Append (&msgbuf, &str);
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, 5);
    print_result ("Simple substring transfer", rcv_ok && snd_ok);

    // Compound substrings
    setup_strbufs ();
    L4_Clear (&msgbuf);
    str = L4_StringItem (32, t1_buf);
    str += &t1_buf[32];
    str += &t1_buf[64];
    str += &t1_buf[96];
    str += L4_StringItem (64, &t1_buf[128]);
    str += &t1_buf[192];
    L4_Append (&msgbuf, &str);
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, 8);
    print_result ("Compound substring transfer", rcv_ok && snd_ok);

    // Multiple complex strings
    setup_strbufs ();
    L4_Clear (&msgbuf);
    str = L4_StringItem (32, t1_buf);
    str += &t1_buf[32];
    str += &t1_buf[64];
    str += &t1_buf[96];
    L4_Append (&msgbuf, &str);
    str = L4_StringItem (32, &t1_buf[128]);
    str += &t1_buf[160];
    str += L4_StringItem (64, &t1_buf[192]);
    L4_Append (&msgbuf, &str);
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, 10);
    print_result ("Multiple complex strings transfer", rcv_ok && snd_ok);

    // Scatter
    setup_strbufs ();
    L4_Clear (&msgbuf);
    str = L4_StringItem (32, t1_buf);
    str += &t1_buf[32];
    str += &t1_buf[64];
    str += &t1_buf[96];
    str += L4_StringItem (32, &t1_buf[128]);
    str += &t1_buf[160];
    str += L4_StringItem (64, &t1_buf[192]);
    L4_Append (&msgbuf, &str);
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, 2);
    print_result ("Scatter", rcv_ok && snd_ok);

    // Gather
    setup_strbufs ();
    L4_Clear (&msgbuf);
    L4_Append (&msgbuf, L4_StringItem (256, t1_buf));
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, 10);
    print_result ("Gather", rcv_ok && snd_ok);

    // Complex Scatter/Gather
    setup_strbufs ();
    L4_Clear (&msgbuf);
    str = L4_StringItem (16, t1_buf);
    str += &t1_buf[16];
    str += &t1_buf[32];
    str += L4_StringItem (32, &t1_buf[48]);
    str += &t1_buf[80];
    str += &t1_buf[112];
    str += &t1_buf[144];
    str += L4_StringItem (16, &t1_buf[176]);
    str += L4_StringItem (64, &t1_buf[192]);
    L4_Append (&msgbuf, &str);
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, 10);
    print_result ("Complex Scatter/Gather", rcv_ok && snd_ok);

    // Too long receive buffer
    setup_strbufs ();
    L4_Clear (&msgbuf);
    str = L4_StringItem (16, t1_buf);
    str += &t1_buf[16];
    str += &t1_buf[32];
    str += L4_StringItem (32, &t1_buf[48]);
    str += &t1_buf[80];
    str += &t1_buf[112];
    str += &t1_buf[144];
    str += L4_StringItem (16, &t1_buf[176]);
    str += L4_StringItem (64, &t1_buf[192]);
    L4_Append (&msgbuf, &str);
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, 2, 252);
    print_result ("Too long receive buffer", rcv_ok && snd_ok);

    // No receive buffer
    setup_strbufs ();
    L4_Accept (L4_UntypedWordsAcceptor);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, 2, 0, true);
    print_result ("No receive buffer", rcv_ok && snd_ok);

    // Missing receive buffer
    setup_strbufs ();
    L4_Clear (&msgbuf);
    L4_Append (&msgbuf, L4_StringItem (256, t1_buf));
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, 4, 128, true);
    print_result ("Missing receive buffer", rcv_ok && snd_ok);

    // Too short receive buffer
    setup_strbufs ();
    L4_Clear (&msgbuf);
    L4_Append (&msgbuf, L4_StringItem (252, t1_buf));
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, 2, 252, true);
    print_result ("Too short receive buffer", rcv_ok && snd_ok);

    // Complex cut message
    setup_strbufs ();
    L4_Clear (&msgbuf);
    str = L4_StringItem (16, t1_buf);
    str += &t1_buf[16];
    str += &t1_buf[32];
    str += &t1_buf[48];
    str += L4_StringItem (32, &t1_buf[64]);
    L4_Append (&msgbuf, &str);
    str = L4_StringItem (32, &t1_buf[96]);
    str += &t1_buf[128];
    str += &t1_buf[160];
    str += L4_StringItem (60, &t1_buf[192]);
    L4_Append (&msgbuf, &str);
    L4_Accept (L4_StringItemsAcceptor, &msgbuf);

    tag = L4_Receive (ipc_t2);
    L4_Store (tag, &msg);
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (ipc_t2);
    rcv_ok = rcv_string_checks (tag, &msg, 10, 252, true);
    print_result ("Complex cut message", rcv_ok && snd_ok);

    // Get oneself killed
    L4_Set_MsgTag (L4_Niltag);
    L4_Call (L4_Pager ());
}

static void string_ipc_t2 (void)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    union {
	L4_Word_t spc[16];
	L4_StringItem_t str;
    };

    spc[0] = 0; // To avoid warning

    // Simple string item
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (256, t2_buf));
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag);
    L4_Receive (ipc_t1);

    // Simple substrings
    L4_Clear (&msg);
    str = L4_StringItem (64, t2_buf);
    str += &t2_buf[64];
    str += &t2_buf[128];
    str += &t2_buf[192];
    L4_Append (&msg, &str);
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag);
    L4_Receive (ipc_t1);

    // Compound substrings
    L4_Clear (&msg);
    str = L4_StringItem (32, t2_buf);
    str += &t2_buf[32];
    str += &t2_buf[64];
    str += &t2_buf[96];
    str += L4_StringItem (64, &t2_buf[128]);
    str += &t2_buf[192];
    L4_Append (&msg, &str);
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag);
    L4_Receive (ipc_t1);

    // Multiple complex strings
    L4_Clear (&msg);
    str = L4_StringItem (32, t2_buf);
    str += &t2_buf[32];
    str += &t2_buf[64];
    str += &t2_buf[96];
    L4_Append (&msg, &str);
    str = L4_StringItem (32, &t2_buf[128]);
    str += &t2_buf[160];
    str += L4_StringItem (64, &t2_buf[192]);
    L4_Append (&msg, &str);
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag);
    L4_Receive (ipc_t1);

    // Scatter
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (256, t2_buf));
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag);
    L4_Receive (ipc_t1);

    // Gather
    L4_Clear (&msg);
    str = L4_StringItem (32, t2_buf);
    str += &t2_buf[32];
    str += &t2_buf[64];
    str += &t2_buf[96];
    str += L4_StringItem (32, &t2_buf[128]);
    str += &t2_buf[160];
    str += L4_StringItem (64, &t2_buf[192]);
    L4_Append (&msg, &str);
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag);
    L4_Receive (ipc_t1);

    // Complex Scatter/Gather
    L4_Clear (&msg);
    str = L4_StringItem (32, t2_buf);
    str += &t2_buf[32];
    str += &t2_buf[64];
    str += L4_StringItem (64, &t2_buf[96]);
    str += &t2_buf[160];
    str += L4_StringItem (16, &t2_buf[224]);
    str += &t2_buf[240];
    L4_Append (&msg, &str);
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag);
    L4_Receive (ipc_t1);

    // Too long receive buffer
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (252, t2_buf));
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag, 252);
    L4_Receive (ipc_t1);

    // No receive buffer
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (256, t2_buf));
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag, 0, true);
    L4_Receive (ipc_t1);

    // Missing receive buffer
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (128, t2_buf));
    L4_Append (&msg, L4_StringItem (128, &t2_buf[128]));
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag, 128, true);
    L4_Receive (ipc_t1);

    // Too short receive buffer
    L4_Clear (&msg);
    L4_Append (&msg, L4_StringItem (256, t2_buf));
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag, 252, true);
    L4_Receive (ipc_t1);

    // Complex cut message
    L4_Clear (&msg);
    str = L4_StringItem (32, t2_buf);
    str += &t2_buf[32];
    str += &t2_buf[64];
    L4_Append (&msg, &str);
    str = L4_StringItem (64, &t2_buf[96]);
    str += &t2_buf[160];
    str += L4_StringItem (16, &t2_buf[224]);
    str += &t2_buf[240];
    L4_Append (&msg, &str);
    L4_Load (&msg);

    tag = L4_Send (ipc_t1);
    snd_ok = snd_string_checks (tag, 252, true);
    L4_Receive (ipc_t1);

    // Get oneself killed
    L4_Set_MsgTag (L4_Niltag);
    L4_Call (L4_Pager ());
}

static bool rcv_string_checks (L4_MsgTag_t tag, L4_Msg_t * msg,
			       L4_Word_t num_typed,
			       L4_Word_t cutpoint,
			       bool cutmessage)
{
    union {
	L4_Word_t spc[16];
	L4_StringItem_t str;
    };
    bool r = true;
    spc[0] = 0;

    if (cutmessage)
    {
	if (L4_IpcSucceeded (tag) ||
	    (L4_ErrorCode () & 0x1) != 1 ||
	    ((L4_ErrorCode () >> 1) & 0x7) != 4)
	    printf ("RCV: Incorrectly reported IPC error: %s %s\n",
		    ipc_errorcode (L4_ErrorCode ()),
		    ipc_errorphase (L4_ErrorCode ())),
		r = false;

	if (cutmessage && (L4_ErrorCode () >> 4) != cutpoint)
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

    if (L4_UntypedWords (tag) != 0 && L4_TypedWords (tag) != num_typed)
	printf ("RCV: Wrong message layout: untyped/typed=%d/%d "
		"(should be 0/%d)\n",
		(int) L4_UntypedWords (tag), (int) L4_TypedWords (tag),
		(int) num_typed),
	    r = false;

    L4_Get (msg, 0, &str);
    if (! L4_StringItem (&str))
	printf ("RCV: Did not receive simple string item\n"), 
	    r = false;
    
    if (! check_strbufs (cutpoint))
	r = false;

    return r;
}

static bool snd_string_checks (L4_MsgTag_t tag,
			       L4_Word_t cutpoint,
			       bool cutmessage)
{
    bool r = true;

    if (cutmessage)
    {
	if (L4_IpcSucceeded (tag) ||
	    (L4_ErrorCode () & 0x1) != 0 ||
	    ((L4_ErrorCode () >> 1) & 0x7) != 4)
	    printf ("SND: Incorrectly reported IPC error: %s %s\n",
		    ipc_errorcode (L4_ErrorCode ()),
		    ipc_errorphase (L4_ErrorCode ())),
		r = false;

	if (cutmessage && (L4_ErrorCode () >> 4) != cutpoint)
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

static void setup_strbufs (void)
{
    for (unsigned int i = 0; i < 256; i++)
    {
	t1_buf[i] = 0xff;
	t2_buf[i] = i;
    }
}

static bool check_strbufs (unsigned int cutpoint)
{
    for (unsigned int i = 0; i < 256; i++)
    {
	unsigned int j = (i >= cutpoint) ? 0xff : i;
	if (t1_buf[i] != j)
	{
	    printf ("RCV: Wrong message contents in buf[%d]: 0x%x != 0x%x\n",
		    (int) i, t1_buf[i], j);
	    return false;
	}
    }
    return true;
}


void
string_ipc (void)
{
    printf ("\nIntra address space string copy IPC test (no pagefaults)\n");
    setup_ipc_threads (string_ipc_t1, string_ipc_t2, false, false);
}
