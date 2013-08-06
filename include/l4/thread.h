/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef L4_THREAD_H_
#define L4_THREAD_H_

#include <platform/armv7m.h>
#include <platform/link.h>

#define L4_NILTHREAD		0
#define L4_ANYTHREAD		0xFFFFFFFF

#define DECLARE_THREAD(name, sub) \
	void name(void) __attribute__ ((naked));	\
	void __USER_TEXT name(void)			\
	{						\
		register void *kip_ptr asm ("r0");	\
		register void *utcb_ptr asm ("r1");	\
		sub(kip_ptr, utcb_ptr);			\
		while (1);				\
	}

#endif	/* L4_THREAD_H_ */
