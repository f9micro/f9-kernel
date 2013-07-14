/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ERROR_H_
#define ERROR_H_

#include <platform/link.h>
#include <thread.h>

/**
 * Userspace errors
 */
enum user_error_t {
	UE_NO_PRIVILIGE 	= 1,
	UE_OUT_OF_MEM		= 8,

	/**
	 * ThreadControl errors
	 * see L4 X.2 R7 Reference, Page 24-25
	 *
	 * Invalid UTCB is not used because we have no Utcb Area
	 */
	UE_TC_NOT_AVAILABLE	= 2,
	UE_TC_INVAL_SPACE	= 3,
	UE_TC_INVAL_SCHED	= 4,
	UE_TC_INVAL_UTCB	= 6,		/* Not used */

	/**
	 * Ipc errors
	 * see L4 X.2 R7 Reference, Page 67-68
	 *
	 * NOTE: Message overflow may also occur if MR's are not enough
	 */

	UE_IPC_PHASE_SEND	= 0,
	UE_IPC_PHASE_RECV	= 1,

	UE_IPC_TIMEOUT		= 1 << 1,
	UE_IPC_NOT_EXIST	= 2 << 1,
	UE_IPC_CANCELED		= 3 << 1,

	UE_IPC_MSG_OVERFLOW	= 4 << 1,
	UE_IPC_XFER_TIMEOUT	= 5 << 1,
	UE_IPC_XFER_TIMEOUT2	= 6 << 1,
	UE_IPC_ABORTED		= 7 << 1
};

#define assert(cond) \
	assert_impl(cond, #cond, __func__)
#define panic(...) \
	panic_impl(__VA_ARGS__)

void set_caller_error(enum user_error_t error);
void set_user_error(tcb_t *thread, enum user_error_t error);
void panic_impl(char *panicfmt, ...);
void assert_impl(int cond, const char *condstr, const char *funcname);

#endif /* ERROR_H_ */
