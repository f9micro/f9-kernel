/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4io.h>
#include <l4/ipc.h>
#include <thread.h>
#include <platform/link.h>

int __USER_TEXT printf(const char *format, ...)
{
	L4_Msg_t msg;
	va_list va;

	va_start(va, format);

	L4_MsgClear(&msg);
	L4_MsgAppendWord(&msg, (L4_Word_t)format);
	L4_MsgAppendWord(&msg, (L4_Word_t)&va);

	L4_MsgLoad(&msg);
	L4_Send((L4_ThreadId_t){.raw = TID_TO_GLOBALID(THREAD_LOG)});

	va_end(va);

	return 0;
}
