/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __POSIX_TIME_H__
#define __POSIX_TIME_H__

#include <posix/sys/types.h>

/* POSIX 1003.1b-93 High Resolution Clocks and Timers */

/* Clock IDs */
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID 3

/* Timer ID type */
typedef int timer_t;

/* Signal value union */
union sigval {
    int sival_int;
    void *sival_ptr;
};

/* Signal event structure */
struct sigevent {
    int sigev_notify;
    int sigev_signo;
    union sigval sigev_value;
    void (*sigev_notify_function)(union sigval);
    void *sigev_notify_attributes;
};

/* Timer structures */
struct itimerspec {
    struct timespec it_interval; /* Timer period */
    struct timespec it_value;    /* Timer expiration */
};

/* Clock functions (1003.1b-93) */
int clock_getres(clockid_t clock_id, struct timespec *res);
int clock_gettime(clockid_t clock_id, struct timespec *tp);
int clock_settime(clockid_t clock_id, const struct timespec *tp);

/* Timer functions (1003.1b-93) */
int timer_create(clockid_t clock_id, struct sigevent *evp, timer_t *timerid);
int timer_delete(timer_t timerid);
int timer_settime(timer_t timerid,
                  int flags,
                  const struct itimerspec *value,
                  struct itimerspec *ovalue);
int timer_gettime(timer_t timerid, struct itimerspec *value);
int timer_getoverrun(timer_t timerid);

/* Nanosleep (1003.1b-93) */
int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);

/* Notification types */
#define SIGEV_NONE 0
#define SIGEV_SIGNAL 1
#define SIGEV_THREAD 2

#endif /* __POSIX_TIME_H__ */
