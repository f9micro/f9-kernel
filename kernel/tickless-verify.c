/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include INC_PLAT(systick.h)
#include INC_PLAT(hwtimer.h)

#include <debug.h>
#include <tickless-verify.h>

static int tickless_verify_enabled;
static int tickless_verify_started;

static int tickless_verify_n = 0;
static int tickless_verify_times = 0;

static struct {
	uint32_t ktimer_diff;
	uint32_t hwtimer_diff;
	uint32_t need;
	uint32_t count;
	uint32_t count_int;
} tickless_verify_records[TICKLESS_VERIFY_MAX_RECORD];

static uint32_t tickless_verify_start_need;
static uint32_t tickless_verify_start_ktimer;
static uint32_t tickless_verify_start_systick;
static uint32_t tickless_verify_start_hwtimer;

/* Count tickless entering times during start and stop */
static uint32_t tickless_verify_start_count;

/* Count times of tickless interrupted by not systick interrupt */
static uint32_t tickless_verify_start_count_int;

static void tickless_verify_save(uint32_t *systick, uint32_t *hwtimer)
{
	*systick = CONFIG_KTIMER_HEARTBEAT - systick_now();
	*hwtimer = hwtimer_now();
}

void tickless_verify_init()
{
	int i;

	if (tickless_verify_enabled == 0) {
		hwtimer_init();

		for (i = 0; i < TICKLESS_VERIFY_MAX_RECORD; i++) {
			tickless_verify_records[i].need = 0;
			tickless_verify_records[i].ktimer_diff = 0;
			tickless_verify_records[i].hwtimer_diff = 0;
			tickless_verify_records[i].count = 0;
			tickless_verify_records[i].count_int = 0;
		}

		tickless_verify_n = 0;
		tickless_verify_times = 0;

		tickless_verify_enabled++;
	}
}

void tickless_verify_start(uint32_t ktimer_now, uint32_t need)
{
	tickless_verify_save(&tickless_verify_start_systick,
	                     &tickless_verify_start_hwtimer);

	tickless_verify_start_need = need;
	tickless_verify_start_ktimer = ktimer_now;
	tickless_verify_start_count = 0;
	tickless_verify_start_count_int = 0;

	tickless_verify_started = tickless_verify_enabled;
}

void tickless_verify_stop(uint32_t ktimer_now)
{
	uint32_t systick;
	uint32_t hwtimer;
	uint32_t ktimer_diff;
	uint32_t hwtimer_diff;

	tickless_verify_save(&systick, &hwtimer);

	ktimer_diff = (ktimer_now - tickless_verify_start_ktimer) * CONFIG_KTIMER_HEARTBEAT
	              + systick - tickless_verify_start_systick;
	hwtimer_diff = (hwtimer - tickless_verify_start_hwtimer) * 2;

	if (!tickless_verify_started)
		return;

	if (tickless_verify_n >= TICKLESS_VERIFY_MAX_RECORD)
		tickless_verify_n = 0;

	tickless_verify_records[tickless_verify_n].need =
	    tickless_verify_start_need * CONFIG_KTIMER_HEARTBEAT -
	    tickless_verify_start_systick;
	tickless_verify_records[tickless_verify_n].ktimer_diff = ktimer_diff;
	tickless_verify_records[tickless_verify_n].hwtimer_diff = hwtimer_diff;
	tickless_verify_records[tickless_verify_n].count = tickless_verify_start_count;
	tickless_verify_records[tickless_verify_n].count_int = tickless_verify_start_count_int;

	tickless_verify_n++;
	tickless_verify_times++;
}

void tickless_verify_count()
{
	tickless_verify_start_count++;
}

void tickless_verify_count_int()
{
	tickless_verify_start_count_int++;
}

int32_t tickless_verify_stat(int *times)
{
	int32_t sum = 0;
	int i, n = tickless_verify_times;
	uint32_t compensation = UINT32_MAX;
	uint32_t int_compensation = UINT32_MAX;

	if (n >= TICKLESS_VERIFY_MAX_RECORD) {
		n = TICKLESS_VERIFY_MAX_RECORD;
	}

#ifdef CONFIG_KDB
	dbg_printf(DL_KDB,
	           "  Record  N: %10s | %10s | %10s | %10s | %10s | %s\n",
	           "      NEED", "      REAL", "      KTIM", " REAL-NEED",
	           " REAL-KTIM", "COUNT(INT)");
#endif

	for (i = 0; i < n; i++) {
		int32_t hwtimer_need_diff =
		    tickless_verify_records[i].hwtimer_diff -
		    tickless_verify_records[i].need;
#ifdef CONFIG_KDB
		int32_t hwtimer_ktimer_diff =
		    tickless_verify_records[i].hwtimer_diff -
		    tickless_verify_records[i].ktimer_diff;

		dbg_printf(DL_KDB,
		"%c Record %2d: %10d | %10d | %10d | %c%9d | %c%9d | %5d(%d)\n",
		           i == tickless_verify_n - 1 ? '*' : ' ', i,
		           tickless_verify_records[i].need,
		           tickless_verify_records[i].hwtimer_diff,
		           tickless_verify_records[i].ktimer_diff,
		           hwtimer_need_diff >= 0 ? ' ' : '-',
		           hwtimer_need_diff >= 0 ? hwtimer_need_diff : -hwtimer_need_diff,
		           hwtimer_ktimer_diff >= 0 ? ' ' : '-',
		           hwtimer_ktimer_diff >= 0 ? hwtimer_ktimer_diff : -hwtimer_ktimer_diff,
		           tickless_verify_records[i].count,
		           tickless_verify_records[i].count_int);
#endif /* CONFIG_KDB */
		sum += hwtimer_need_diff;
	}

	*times = tickless_verify_times;

	/* Tickless entering compensation */
	for (i = 0; i < n; i++) {
		if (tickless_verify_records[i].count_int == 0) {
			int32_t diff = tickless_verify_records[i].hwtimer_diff - tickless_verify_records[i].ktimer_diff;

			if (diff > 0) {
				int new_compensation = diff / tickless_verify_records[i].count;

				if (tickless_verify_records[i].count * new_compensation < diff) {
					new_compensation++;
				}

				if (new_compensation < compensation) {
					compensation = new_compensation;
				}
			}
		}
	}

	/* Tickless interrupted compensation */
	for (i = 0; i < n; i++) {
		if (tickless_verify_records[i].count_int != 0) {
			int32_t diff = tickless_verify_records[i].hwtimer_diff - tickless_verify_records[i].ktimer_diff
			               - tickless_verify_records[i].count * compensation;

			if (diff > 0) {
				int new_int_compensation = diff / tickless_verify_records[i].count_int;

				if (tickless_verify_records[i].count_int * new_int_compensation < diff) {
					new_int_compensation++;
				}

				if (new_int_compensation < int_compensation) {
					int_compensation = new_int_compensation;
				}
			}
		}
	}

	dbg_printf(DL_KDB, "Suggest Compensation: %d(%d)\n",
	           compensation, int_compensation);

	return sum / n;
}
