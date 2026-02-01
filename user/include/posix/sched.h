/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __POSIX_SCHED_H__
#define __POSIX_SCHED_H__

#include <l4/ipc.h>
#include <posix/sys/types.h>

/* Scheduling policies */
#define SCHED_OTHER 0 /* Normal scheduling */
#define SCHED_FIFO 1  /* First-in-first-out scheduling */
#define SCHED_RR 2    /* Round-robin scheduling */

/* Priority range for SCHED_FIFO and SCHED_RR */
#define SCHED_PRIORITY_MIN 1
#define SCHED_PRIORITY_MAX 255

/* Scheduling parameter structure - defined in sys/types.h */

/* Get priority range */
int sched_get_priority_max(int policy);
int sched_get_priority_min(int policy);

/* Scheduler yield - static inline to avoid symbol conflict with kernel
 * Uses L4 IPC timeout for user-space voluntary preemption.
 */
static inline int sched_yield(void)
{
    L4_Sleep(L4_TimePeriod(0));
    return 0;
}
int sched_getscheduler(pid_t pid);
int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param);
int sched_getparam(pid_t pid, struct sched_param *param);
int sched_setparam(pid_t pid, const struct sched_param *param);

/* Thread scheduling (pthread extensions) */
int pthread_setschedparam(pthread_t *thread,
                          int policy,
                          const struct sched_param *param);
int pthread_getschedparam(pthread_t *thread,
                          int *policy,
                          struct sched_param *param);
int pthread_setschedprio(pthread_t *thread, int prio);

/* Thread scheduling attributes */
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy);
int pthread_attr_setschedparam(pthread_attr_t *attr,
                               const struct sched_param *param);
int pthread_attr_getschedparam(const pthread_attr_t *attr,
                               struct sched_param *param);
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched);

/* Inherit scheduling constants */
#define PTHREAD_INHERIT_SCHED 0
#define PTHREAD_EXPLICIT_SCHED 1

#endif /* __POSIX_SCHED_H__ */
