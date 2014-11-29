/* Copyright (c) 2001, 2002, 2003, 2008 Karlsruhe University.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __L4__IPC_H__
#define __L4__IPC_H__

#include <l4/types.h>
#include __L4_INC_ARCH(syscalls.h)
#include <l4/message.h>
#include <l4/thread.h>

/*
 * Support functions
 */
L4_INLINE L4_Bool_t L4_IpcSucceeded(L4_MsgTag_t t)
{
	return (t.X.flags & 0x8) == 0;
}

L4_INLINE L4_Bool_t L4_IpcFailed(L4_MsgTag_t t)
{
	return (t.X.flags & 0x8) != 0;
}

L4_INLINE L4_Bool_t L4_IpcPropagated(L4_MsgTag_t t)
{
	return (t.X.flags & 0x1) != 0;
}

L4_INLINE L4_Bool_t L4_IpcRedirected(L4_MsgTag_t t)
{
	return (t.X.flags & 0x2) != 0;
}

L4_INLINE L4_Bool_t L4_IpcXcpu(L4_MsgTag_t t)
{
	return (t.X.flags & 0x4) != 0;
}

L4_INLINE void L4_Set_Propagation(L4_MsgTag_t * t)
{
	t->X.flags = 1;
}

L4_INLINE L4_Word_t L4_Timeouts(L4_Time_t SndTimeout, L4_Time_t RcvTimeout)
{
	return (SndTimeout.raw << 16) + (RcvTimeout.raw);
}

/*
 * Derived functions
 */

/*
 * These are the functions derived from the two IPC syscalls.  For
 * normal C, function overloading is not supported.  Functions with
 * unique names (i.e., with the suffix inside <> appended) have
 * therefore been provided.
 *
 *   L4_Call			(to)
 *   L4_Call<_Timeouts>		(to, SndTimeout, RcvTimeout)
 *   L4_Send			(to)
 *   L4_Send<_Timeout>		(to, SndTimeout)
 *   L4_Reply			(to)
 *   L4_Receive			(from)
 *   L4_Receive<_Timeout>	(from, RcvTimeout)
 *   L4_Wait			(&from)
 *   L4_Wait<_Timeout>		(RcvTimeout, &from)
 *   L4_ReplyWait		(to, &from)
 *   L4_ReplyWait<_Timeout>	(to, RcvTimeout, &from)
 *   L4_Sleep			(time)
 *   L4_Lcall			(to)
 *   L4_Lreply_Wait		(to, &from)
 *
 */
L4_INLINE L4_MsgTag_t L4_Call_Timeouts(
		L4_ThreadId_t to,
		L4_Time_t SndTimeout,
		L4_Time_t RcvTimeout)
{
	L4_ThreadId_t dummy;
	return L4_Ipc(to, to, L4_Timeouts (SndTimeout, RcvTimeout),
			&dummy);
}

#if defined(__cplusplus)
L4_INLINE L4_MsgTag_t L4_Call(
		L4_ThreadId_t to,
		L4_Time_t SndTimeout,
		L4_Time_t RcvTimeout)
{
	return L4_Call_Timeouts (to, SndTimeout, RcvTimeout);
}
#endif

L4_INLINE L4_MsgTag_t L4_Call(L4_ThreadId_t to)
{
	return L4_Call_Timeouts(to, L4_Never, L4_Never);
}

L4_INLINE L4_MsgTag_t L4_Send_Timeout(
		L4_ThreadId_t to, L4_Time_t SndTimeout)
{
	return L4_Ipc (to, L4_nilthread,
			(L4_Word_t) SndTimeout.raw,
			(L4_ThreadId_t *) 0);
}

#if defined(__cplusplus)
L4_INLINE L4_MsgTag_t L4_Send(L4_ThreadId_t to, L4_Time_t SndTimeout)
{
	return L4_Send_Timeout(to, SndTimeout);
}
#endif

L4_INLINE L4_MsgTag_t L4_Send(L4_ThreadId_t to)
{
	return L4_Send_Timeout(to, L4_Never);
}

L4_INLINE L4_MsgTag_t L4_Reply(L4_ThreadId_t to)
{
	return L4_Send_Timeout(to, L4_ZeroTime);
}

L4_INLINE L4_MsgTag_t L4_Receive_Timeout(
		L4_ThreadId_t from,
		L4_Time_t RcvTimeout)
{
	L4_ThreadId_t dummy;
	return L4_Ipc(L4_nilthread, from, (L4_Word_t) RcvTimeout.raw, &dummy);
}

#if defined(__cplusplus)
L4_INLINE L4_MsgTag_t L4_Receive(L4_ThreadId_t from, L4_Time_t RcvTimeout)
{
	return L4_Receive_Timeout(from, RcvTimeout);
}
#endif

L4_INLINE L4_MsgTag_t L4_Receive(L4_ThreadId_t from)
{
	return L4_Receive_Timeout(from, L4_Never);
}

L4_INLINE L4_MsgTag_t L4_Wait_Timeout(
		L4_Time_t RcvTimeout,
		L4_ThreadId_t *from)
{
	return L4_Ipc(L4_nilthread, L4_anythread,
			(L4_Word_t) RcvTimeout.raw, from);
}

#if defined(__cplusplus)
L4_INLINE L4_MsgTag_t L4_Wait(L4_Time_t RcvTimeout, L4_ThreadId_t *from)
{
	return L4_Wait_Timeout(RcvTimeout, from);
}
#endif

L4_INLINE L4_MsgTag_t L4_Wait(L4_ThreadId_t *from)
{
	return L4_Wait_Timeout(L4_Never, from);
}

L4_INLINE L4_MsgTag_t L4_ReplyWait_Timeout(
		L4_ThreadId_t to,
		L4_Time_t RcvTimeout,
		L4_ThreadId_t *from)
{
	return L4_Ipc(to, L4_anythread,
			L4_Timeouts(L4_ZeroTime, RcvTimeout),
			from);
}

#if defined(__cplusplus)
L4_INLINE L4_MsgTag_t L4_ReplyWait(
		L4_ThreadId_t to,
		L4_Time_t RcvTimeout,
		L4_ThreadId_t *from)
{
	return L4_ReplyWait_Timeout(to, RcvTimeout, from);
}
#endif


L4_INLINE L4_MsgTag_t L4_ReplyWait(L4_ThreadId_t to, L4_ThreadId_t *from)
{
	return L4_ReplyWait_Timeout(to, L4_Never, from);
}

L4_INLINE void L4_Sleep(L4_Time_t t)
{
	L4_Ipc(L4_nilthread, L4_nilthread, (L4_Word_t) t.raw, NULL);
}

L4_INLINE L4_MsgTag_t L4_Lcall(L4_ThreadId_t to)
{
	L4_ThreadId_t dummy;
	return L4_Lipc(to, to, L4_Timeouts(L4_Never, L4_Never), &dummy);
}

L4_INLINE L4_MsgTag_t L4_LreplyWait(L4_ThreadId_t to, L4_ThreadId_t *from)
{
	return L4_Lipc(to, L4_anylocalthread,
			L4_Timeouts(L4_ZeroTime, L4_Never), from);
}

#endif /* !__L4__IPC_H__ */
