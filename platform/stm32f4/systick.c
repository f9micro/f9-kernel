/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/stm32f4/systick.h>
#include <platform/cortex_m.h>

void init_systick(uint32_t tick_reload)
{
	/* 250us at 168Mhz */
	*SYSTICK_RELOAD = tick_reload;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTL = 0x00000007;
}
