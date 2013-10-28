/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ERROR_H_
#define ERROR_H_

#include <platform/link.h>


#define assert(cond) \
	assert_impl(cond, #cond, __func__)
#define panic(...) \
	panic_impl(__VA_ARGS__)

void panic_impl(char *panicfmt, ...);
void assert_impl(int cond, const char *condstr, const char *funcname);

#endif /* ERROR_H_ */
