/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef TICKLESS_VERIFY_H
#define TICKLESS_VERIFY_H

#include <stdint.h>

#define TICKLESS_VERIFY_MAX_RECORD 16

void tickless_verify_init();
void tickless_verify_start(uint32_t ktimer_now);
void tickless_verify_stop(uint32_t ktimer_now);
int32_t tickless_verify_stat(int *times);

#endif
