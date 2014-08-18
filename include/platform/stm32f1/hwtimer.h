/* Copyright (c) 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_STM32F1_HWTIMER_H_
#define PLATFORM_STM32F1_HWTIMER_H_

#include <stdint.h>

void hwtimer_init(void);
uint32_t hwtimer_now(void);

#endif /* PLATFORM_STM32F1_HWTIMER_H_ */
