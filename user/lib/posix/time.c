/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/ipc.h>
#include <l4/platform/syscalls.h>
#include <l4/utcb.h>
#include <platform/link.h>
#include <posix/sys/types.h>
#include <posix/time.h>

/* Time implementation for PSE51 POSIX_TIMERS compliance
 *
 * Uses L4_SystemClock() syscall for real kernel time.
 * The kernel maintains a microsecond counter from system boot.
 *
 * Clock resolution: L4_SystemClock advances at kernel tick rate.
 * With POSIX_USEC_PER_TICK=400, effective resolution is 400µs.
 */

/* Microseconds per second */
#define USEC_PER_SEC 1000000
#define NSEC_PER_USEC 1000

/* Clock resolution in nanoseconds - matches POSIX_USEC_PER_TICK (400µs) */
#define CLOCK_RESOLUTION_NS (400 * NSEC_PER_USEC)

__USER_TEXT
int clock_getres(clockid_t clock_id, struct timespec *res)
{
    if (!res)
        return EINVAL;

    switch (clock_id) {
    case CLOCK_REALTIME:
    case CLOCK_MONOTONIC:
        /* Resolution is 400 microseconds (kernel tick rate) */
        res->tv_sec = 0;
        res->tv_nsec = CLOCK_RESOLUTION_NS;
        return 0;
    default:
        return EINVAL;
    }
}

__USER_TEXT
int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    if (!tp)
        return EINVAL;

    switch (clock_id) {
    case CLOCK_REALTIME:
    case CLOCK_MONOTONIC:
        /* Use real kernel time via L4_SystemClock syscall.
         * Returns microseconds since system boot.
         */
        {
            L4_Clock_t clock = L4_SystemClock();
            uint64_t usec = clock.raw;
            tp->tv_sec = usec / USEC_PER_SEC;
            tp->tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
        }
        return 0;
    default:
        return EINVAL;
    }
}

__USER_TEXT
int clock_settime(clockid_t clock_id, const struct timespec *tp)
{
    /* Setting clock time not supported on embedded system */
    if (!tp)
        return EINVAL;

    if (clock_id == CLOCK_MONOTONIC)
        return EPERM; /* Monotonic clock cannot be set */

    /* CLOCK_REALTIME setting not implemented */
    return EPERM;
}

__USER_TEXT
int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
    if (!rqtp)
        return EINVAL;

    if (rqtp->tv_nsec >= 1000000000)
        return EINVAL;

    /* Convert timespec to microseconds for L4_Sleep.
     * Use uint64_t to prevent overflow for large tv_sec values.
     * Max representable: ~584,942 years (2^64 / 1e6 / 3600 / 24 / 365)
     */
    uint64_t total_usec =
        (uint64_t) rqtp->tv_sec * 1000000ULL + rqtp->tv_nsec / 1000;

    /* L4_Sleep takes time period in microseconds */
    while (total_usec > 0) {
        /* Sleep in chunks (max 1 second per iteration for responsiveness) */
        L4_Word_t chunk =
            (total_usec > 1000000) ? 1000000 : (L4_Word_t) total_usec;
        L4_Sleep(L4_TimePeriod(chunk));
        total_usec -= chunk;
    }

    /* Remaining time - not tracked precisely */
    if (rmtp) {
        rmtp->tv_sec = 0;
        rmtp->tv_nsec = 0;
    }

    return 0;
}

/* Timer implementation - simplified stubs for PSE51 compliance */

/* Static timer storage - limited number of timers
 * Must use __USER_BSS to place in user-accessible memory region.
 */
#define MAX_TIMERS 4
__USER_BSS static struct {
    int active;
    clockid_t clock_id;
    struct itimerspec value;
} timer_table[MAX_TIMERS];

/* Spinlock for timer slot allocation to prevent race conditions */
__USER_BSS static uint32_t timer_table_lock;

__USER_TEXT
static void timer_lock_acquire(void)
{
    while (__atomic_exchange_n(&timer_table_lock, 1, __ATOMIC_ACQUIRE))
        ;
}

__USER_TEXT
static void timer_lock_release(void)
{
    __atomic_store_n(&timer_table_lock, 0, __ATOMIC_RELEASE);
}

__USER_TEXT
int timer_create(clockid_t clock_id, struct sigevent *evp, timer_t *timerid)
{
    int i;
    int result = EAGAIN; /* No timer slots available */

    if (!timerid)
        return EINVAL;

    if (clock_id != CLOCK_REALTIME && clock_id != CLOCK_MONOTONIC)
        return EINVAL;

    /* Find free timer slot under lock to prevent race conditions */
    timer_lock_acquire();

    for (i = 0; i < MAX_TIMERS; i++) {
        if (!timer_table[i].active) {
            timer_table[i].active = 1;
            timer_table[i].clock_id = clock_id;
            timer_table[i].value.it_value.tv_sec = 0;
            timer_table[i].value.it_value.tv_nsec = 0;
            timer_table[i].value.it_interval.tv_sec = 0;
            timer_table[i].value.it_interval.tv_nsec = 0;
            *timerid = (timer_t) i;
            result = 0;
            break;
        }
    }

    timer_lock_release();
    return result;
}

__USER_TEXT
int timer_delete(timer_t timerid)
{
    int idx = (int) timerid;

    if (idx < 0 || idx >= MAX_TIMERS || !timer_table[idx].active)
        return EINVAL;

    timer_table[idx].active = 0;
    return 0;
}

__USER_TEXT
int timer_settime(timer_t timerid,
                  int flags,
                  const struct itimerspec *value,
                  struct itimerspec *ovalue)
{
    int idx = (int) timerid;

    if (idx < 0 || idx >= MAX_TIMERS || !timer_table[idx].active)
        return EINVAL;

    if (!value)
        return EINVAL;

    if (ovalue) {
        *ovalue = timer_table[idx].value;
    }

    timer_table[idx].value = *value;

    /* FIXME: Actual timer firing not implemented - would require
     * kernel timer integration or periodic polling.
     */

    return 0;
}

__USER_TEXT
int timer_gettime(timer_t timerid, struct itimerspec *value)
{
    int idx = (int) timerid;

    if (idx < 0 || idx >= MAX_TIMERS || !timer_table[idx].active)
        return EINVAL;

    if (!value)
        return EINVAL;

    *value = timer_table[idx].value;
    return 0;
}

__USER_TEXT
int timer_getoverrun(timer_t timerid)
{
    int idx = (int) timerid;

    if (idx < 0 || idx >= MAX_TIMERS || !timer_table[idx].active)
        return -1;

    /* Overrun tracking not implemented */
    return 0;
}
