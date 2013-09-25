/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     apps/l4test/threads.cc
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
#include <l4/thread.h>
#include <l4/space.h>
#include <l4/schedule.h>
#include <l4/ipc.h>
#include <l4io.h>

#include "l4test.h"


L4_ThreadId_t __USER_TEXT
get_new_tid (void)
{
    __USER_BSS static L4_Word_t next_no = 0;

    if (next_no == 0)
	// Initialize with my thread_no + 1
	next_no = L4_ThreadNo (L4_MyGlobalId ()) + 1;

    return L4_GlobalId (next_no++, 1);
}


L4_ThreadId_t __USER_TEXT
create_thread (void (*func)(void), bool new_space, int cpu, L4_Word_t spacectrl)
{
    __USER_BSS static L4_Fpage_t kip_area, utcb_area;
    __USER_BSS static L4_Word_t utcb_base;
    __USER_BSS static bool initialized = false;
    __USER_BSS static void * kip;
    L4_ThreadId_t me;
    L4_ThreadId_t tid;
    L4_Word_t utcb_location;

    if (! initialized)
    {
    L4_Word_t ApiVersion, ApiFlags, KernelId;
	L4_ThreadId_t mylocalid;

	kip = L4_KernelInterface (&ApiVersion, &ApiFlags, &KernelId);

	// Put the kip at the same location in all address spaces
	// to make sure we can reuse the syscall jump table.
	kip_area = L4_FpageLog2 ((L4_Word_t) kip, L4_KipAreaSizeLog2 (kip));

	mylocalid = L4_MyLocalId ();
	utcb_base = *(L4_Word_t *) &mylocalid;
	utcb_base &= ~(L4_UtcbAreaSize (kip) - 1);

	// Make room for at least 1000 threads in the UTCB area
	utcb_area = L4_Fpage (utcb_base, L4_UtcbSize (kip) * 1000);

	initialized = true;
    }

    me = L4_Myself ();
    tid = get_new_tid ();

    utcb_location =
	utcb_base + L4_UtcbSize (kip) * ( L4_ThreadNo (tid) - L4_ThreadIdUserBase (kip) + 1 );

    if (new_space)
    {
	L4_Word_t control;

	// Create inactive thread
	int res = L4_ThreadControl (tid, tid, me, L4_nilthread, (void *) -1);
	if (res != 1)
	    printf ("ERROR: ThreadControl returned %d (ERR=%d)\n", res, L4_ErrorCode());

	res = L4_SpaceControl (tid, spacectrl, kip_area, utcb_area,
			       L4_nilthread, &control);
	if (res != 1)
	    printf ("ERROR: SpaceControl returned %d (ERR=%d)\n", res, L4_ErrorCode());

	// Activate thread
	res = L4_ThreadControl (tid, tid, me, me, (void *) utcb_location);
	if (res != 1)
	    printf ("ERROR: ThreadControl returned %d (ERR=%d)\n", res, L4_ErrorCode());
    }
    else
    {
	// Create active thread
	int res = L4_ThreadControl (tid, me, me, me, (void *) utcb_location);
	if (res != 1)
	    printf ("ERROR: ThreadControl returned %d (ERR=%d)\n", res, L4_ErrorCode());
    }

    if (cpu != -1)
	L4_Set_ProcessorNo (tid, cpu);

	if (func != NULL)
    	start_thread (tid, func);

    return tid;
}


L4_Word_t __USER_TEXT
kill_thread (L4_ThreadId_t tid)
{
    return L4_ThreadControl (tid, L4_nilthread, L4_nilthread,
			     L4_nilthread, (void *) -1);
}


void __USER_TEXT
start_thread (L4_ThreadId_t tid, void (*func)(void))
{
    L4_Msg_t msg;
    L4_Word_t ip, sp;

    get_startup_values (func, &ip, &sp);

    L4_MsgClear (&msg);
    L4_MsgAppendWord (&msg, ip);
    L4_MsgAppendWord (&msg, sp);
    L4_MsgLoad (&msg);

    L4_Send (tid);
}
