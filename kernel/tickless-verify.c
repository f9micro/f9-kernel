/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include INC_PLAT(systick.h)
#include INC_PLAT(hwtimer.h)

#include <debug.h>
#include <tickless-verify.h>

static int verify_enabled;
static int verify_started;

static int verify_n = 0;
static int verify_times = 0;

static struct {
	uint32_t ktimer_diff;
	uint32_t hwtimer_diff;
	uint32_t need;
	uint32_t count;
	uint32_t count_int;
} verify_records[TICKLESS_VERIFY_MAX_RECORD];

static uint32_t verify_start_need;
static uint32_t verify_start_ktimer;
static uint32_t verify_start_systick;
static uint32_t verify_start_hwtimer;

/* Count tickless entering times during start and stop */
static uint32_t verify_start_count;

/* Count times of tickless interrupted by not systick interrupt */
static uint32_t verify_start_count_int;

static void verify_save(uint32_t *systick, uint32_t *hwtimer)
{
	*systick = CONFIG_KTIMER_HEARTBEAT - systick_now();
	*hwtimer = hwtimer_now();
}

void tickless_verify_init()
{
	if (verify_enabled == 0) {
		hwtimer_init();

		for (int i = 0; i < TICKLESS_VERIFY_MAX_RECORD; i++) {
			verify_records[i].need = 0;
			verify_records[i].ktimer_diff = 0;
			verify_records[i].hwtimer_diff = 0;
			verify_records[i].count = 0;
			verify_records[i].count_int = 0;
		}

		verify_n = 0;
		verify_times = 0;

		verify_enabled++;
	}
}

void tickless_verify_start(uint32_t ktimer_now, uint32_t need)
{
	verify_save(&verify_start_systick, &verify_start_hwtimer);

	verify_start_need = need;
	verify_start_ktimer = ktimer_now;
	verify_start_count = 0;
	verify_start_count_int = 0;

	verify_started = verify_enabled;
}

void tickless_verify_stop(uint32_t ktimer_now)
{
	uint32_t systick;
	uint32_t hwtimer;
	uint32_t ktimer_diff;
	uint32_t hwtimer_diff;

	verify_save(&systick, &hwtimer);

	ktimer_diff = (ktimer_now - verify_start_ktimer) * CONFIG_KTIMER_HEARTBEAT
	              + systick - verify_start_systick;
	hwtimer_diff = (hwtimer - verify_start_hwtimer) * 2;

	if (!verify_started)
		return;

	if (verify_n >= TICKLESS_VERIFY_MAX_RECORD)
		verify_n = 0;

	verify_records[verify_n].need =
	    verify_start_need * CONFIG_KTIMER_HEARTBEAT -
	    verify_start_systick;
	verify_records[verify_n].ktimer_diff = ktimer_diff;
	verify_records[verify_n].hwtimer_diff = hwtimer_diff;
	verify_records[verify_n].count = verify_start_count;
	verify_records[verify_n].count_int = verify_start_count_int;

	verify_n++;
	verify_times++;
}

void tickless_verify_count()
{
	verify_start_count++;
}

void tickless_verify_count_int()
{
	verify_start_count_int++;
}

int32_t tickless_verify_stat(int *times)
{
	int32_t sum = 0;
	int n = verify_times;
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

	for (int i = 0; i < n; i++) {
		int32_t hwtimer_need_diff =
		    verify_records[i].hwtimer_diff -
		    verify_records[i].need;
#ifdef CONFIG_KDB
		int32_t hwtimer_ktimer_diff =
		    verify_records[i].hwtimer_diff -
		    verify_records[i].ktimer_diff;

		dbg_printf(DL_KDB,
		"%c Record %2d: %10d | %10d | %10d | %c%9d | %c%9d | %5d(%d)\n",
		           i == verify_n - 1 ? '*' : ' ', i,
		           verify_records[i].need,
		           verify_records[i].hwtimer_diff,
		           verify_records[i].ktimer_diff,
		           hwtimer_need_diff >= 0 ? ' ' : '-',
		           hwtimer_need_diff >= 0 ? hwtimer_need_diff : -hwtimer_need_diff,
		           hwtimer_ktimer_diff >= 0 ? ' ' : '-',
		           hwtimer_ktimer_diff >= 0 ? hwtimer_ktimer_diff : -hwtimer_ktimer_diff,
		           verify_records[i].count,
		           verify_records[i].count_int);
#endif /* CONFIG_KDB */
		sum += hwtimer_need_diff;
	}

	*times = verify_times;

	/* Tickless entering compensation */
	for (int i = 0; i < n; i++) {
		if (verify_records[i].count_int == 0) {
			int32_t diff = verify_records[i].hwtimer_diff - verify_records[i].ktimer_diff;

			if (diff > 0) {
				int new_comp = diff / verify_records[i].count;

				if (verify_records[i].count * new_comp < diff) {
					new_comp++;
				}

				if (new_comp < compensation) {
					compensation = new_comp;
				}
			}
		}
	}

	/* Tickless interrupted compensation */
	for (int i = 0; i < n; i++) {
		if (verify_records[i].count_int == 0)
			continue;

		int32_t diff = verify_records[i].hwtimer_diff -
		               verify_records[i].ktimer_diff -
		               verify_records[i].count * compensation;

		if (diff > 0) {
			int new_int_comp = diff / verify_records[i].count_int;

			if (verify_records[i].count_int * new_int_comp < diff) {
				new_int_comp++;
			}

			if (new_int_comp < int_compensation) {
				int_compensation = new_int_comp;
			}
		}
	}

	dbg_printf(DL_KDB, "Suggest Compensation: %d(%d)\n",
	           compensation, int_compensation);

	return sum / n;
}
