/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <debug.h>
#include <tickless_verify.h>
#include <platform/stm32f4/registers.h>
#include <platform/stm32f4/systick.h>
#include <config.h>

#define TICKLESS_VERIFY_MAX_RECORD 16

static int tickless_verify_enabled;

static int tickless_verify_n = 0;
static int tickless_verify_times = 0;

static struct {
	uint32_t ktimer_diff;
	uint32_t hwtimer_diff;
	int32_t diff;
} tickless_verify_records[TICKLESS_VERIFY_MAX_RECORD];

static uint32_t tickless_verify_start_ktimer;
static uint32_t tickless_verify_start_systick;
static uint32_t tickless_verify_start_hwtimer;

void tickless_verify_init()
{
	int i;

	if (tickless_verify_enabled == 0) {
		*RCC_APB1ENR |= 0x00000001;
		*TIM2_PSC = 0;
		*TIM2_ARR = 0xFFFFFFFF;
		*TIM2_CR1 = 0x00000001;

		for (i = 0; i < TICKLESS_VERIFY_MAX_RECORD; i++) {
			tickless_verify_records[i].diff = 0;
			tickless_verify_records[i].ktimer_diff = 0;
			tickless_verify_records[i].hwtimer_diff = 0;
		}

		tickless_verify_n = 0;
		tickless_verify_times = 0;

		tickless_verify_enabled++;
	}
}

void tickless_verify_start(uint32_t ktimer_now)
{
	tickless_verify_start_ktimer = ktimer_now;
	tickless_verify_start_systick = systick_load() - systick_now();
	tickless_verify_start_hwtimer = *TIM2_CNT;
}

void tickless_verify_stop(uint32_t ktimer_now)
{
	uint32_t systick = systick_load() - systick_now();
	uint32_t hwtimer = *TIM2_CNT;

	uint32_t ktimer_diff = (ktimer_now - tickless_verify_start_ktimer) * CONFIG_KTIMER_HEARTBEAT
							+ systick - tickless_verify_start_systick;

	uint32_t hwtimer_diff = (hwtimer - tickless_verify_start_hwtimer) * 2;

	int32_t var = hwtimer_diff - ktimer_diff;

	if (tickless_verify_n >= TICKLESS_VERIFY_MAX_RECORD)
		tickless_verify_n = 0;

	tickless_verify_records[tickless_verify_n].diff = var;
	tickless_verify_records[tickless_verify_n].ktimer_diff = ktimer_diff;
	tickless_verify_records[tickless_verify_n].hwtimer_diff = hwtimer_diff;

	tickless_verify_n++;
	tickless_verify_times++;
}

int32_t tickless_verify_stat(int *times)
{
	int32_t sum = 0;
	int i, n = tickless_verify_times;

	if (n >= TICKLESS_VERIFY_MAX_RECORD) {
		n = TICKLESS_VERIFY_MAX_RECORD;
	}

	for (i = 0; i < n; i++) {
		dbg_printf(DL_KDB, "Record: %2d: %10d - %10d = %10d\n", i, tickless_verify_records[i].hwtimer_diff, tickless_verify_records[i].ktimer_diff, tickless_verify_records[i].diff);
		sum += tickless_verify_records[i].diff;
	}

	*times = tickless_verify_times;

	return sum / n;
}
