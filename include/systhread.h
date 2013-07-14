/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SYSTHREAD_H_
#define SYSTHREAD_H_

#include <thread.h>

void create_root_thread();
void create_kernel_thread();
void create_idle_thread();

void switch_to_kernel();
void set_kernel_state(thread_state_t state);

#endif /* SYSTHREAD_H_ */
