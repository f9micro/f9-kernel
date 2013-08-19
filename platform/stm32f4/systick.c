/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/stm32f4/systick.h>
#include <platform/cortex_m.h>

void init_systick(uint32_t tick_reload, uint32_t tick_next_reload)
{
	/* 250us at 168Mhz */
	*SYSTICK_RELOAD = tick_reload;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTL = 0x00000007;

	if (tick_next_reload)
		*SYSTICK_RELOAD = tick_next_reload;
}

void systick_disable()
{
	*SYSTICK_CTL = 0x00000000;
}

uint32_t systick_now()
{
	return *SYSTICK_VAL;
}

uint32_t systick_flag_count()
{
	return (*SYSTICK_CTL & (1 << 16)) >> 16;
}
