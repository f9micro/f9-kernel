/* Copyright (c) 2013 The F9 Microkernel. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __USER_PLATFORM_ARCH_H__
#define __USER_PLATFORM_ARCH_H__

#define ARCH_NAME               "ARM-CM4"

#define PAGE_BITS		(8)
#define PAGE_SIZE		(1 << PAGE_BITS)
#define MAX_MEM			(4L*256L)

#define STACK_PAGES             1

#endif /* __USER_PLATFORM_ARCH_H__ */
