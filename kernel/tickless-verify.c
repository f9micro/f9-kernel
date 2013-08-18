/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include INC_PLAT(systick.h)
#include INC_PLAT(hwtimer.h)

#include <debug.h>
#include <tickless_verify.h>

static int tickless_verify_enabled;
static int tickless_verify_started;

static int tickless_verify_n = 0;
static int tickless_verify_times = 0;

static struct {
	uint32_t ktimer_diff;
	uint32_t hwtimer_diff;
	uint32_t need;
} tickless_verify_records[TICKLESS_VERIFY_MAX_RECORD];

static uint32_t tickless_verify_start_need;
static uint32_t tickless_verify_start_ktimer;
static uint32_t tickless_verify_start_systick;
static uint32_t tickless_verify_start_hwtimer;

void tickless_verify_init()
{
	int i;

	if (tickless_verify_enabled == 0) {
		hwtimer_init();

		for (i = 0; i < TICKLESS_VERIFY_MAX_RECORD; i++) {
			tickless_verify_records[i].need = 0;
			tickless_verify_records[i].ktimer_diff = 0;
			tickless_verify_records[i].hwtimer_diff = 0;
		}

		tickless_verify_n = 0;
		tickless_verify_times = 0;

		tickless_verify_enabled++;
	}
}

void tickless_verify_start(uint32_t ktimer_now, uint32_t need)
{
	tickless_verify_start_need = need;
	tickless_verify_start_ktimer = ktimer_now;
	tickless_verify_start_systick = CONFIG_KTIMER_HEARTBEAT - systick_now();
	tickless_verify_start_hwtimer = hwtimer_now();

	tickless_verify_started = tickless_verify_enabled;
}

void tickless_verify_stop(uint32_t ktimer_now)
{
	uint32_t systick = CONFIG_KTIMER_HEARTBEAT - systick_now();
	uint32_t hwtimer = hwtimer_now();

	uint32_t ktimer_diff = (ktimer_now - tickless_verify_start_ktimer) * CONFIG_KTIMER_HEARTBEAT
							+ systick - tickless_verify_start_systick;

	uint32_t hwtimer_diff = (hwtimer - tickless_verify_start_hwtimer) * 2;

	if (!tickless_verify_started)
		return;

	if (tickless_verify_n >= TICKLESS_VERIFY_MAX_RECORD)
		tickless_verify_n = 0;

	tickless_verify_records[tickless_verify_n].need =
		tickless_verify_start_need * CONFIG_KTIMER_HEARTBEAT - tickless_verify_start_systick;
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

	dbg_printf(DL_KDB, "  Record  N: %10s | %10s | %10s | %10s | %10s\n",
		"      NEED", "      REAL", "      KTIM", " REAL-NEED", " REAL-KTIM");

	for (i = 0; i < n; i++) {
		int32_t hwtimer_need_diff =
			tickless_verify_records[i].hwtimer_diff - tickless_verify_records[i].need;
		int32_t hwtimer_ktimer_diff =
			tickless_verify_records[i].hwtimer_diff - tickless_verify_records[i].ktimer_diff;

		dbg_printf(DL_KDB, "%c Record %2d: %10d | %10d | %10d | %c%9d | %c%9d\n",
			i == tickless_verify_n - 1 ? '*' : ' ', i,
			tickless_verify_records[i].need,
			tickless_verify_records[i].hwtimer_diff,
			tickless_verify_records[i].ktimer_diff,
			hwtimer_need_diff >= 0 ? ' ' : '-',
			hwtimer_need_diff >= 0 ? hwtimer_need_diff : -hwtimer_need_diff,
			hwtimer_ktimer_diff >= 0 ? ' ' : '-',
			hwtimer_ktimer_diff >= 0 ? hwtimer_ktimer_diff : -hwtimer_ktimer_diff);
		sum += hwtimer_need_diff;
	}

	*times = tickless_verify_times;

	return sum / n;
}
