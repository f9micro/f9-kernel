/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <posix/sys/types.h>

/* POSIX semaphore limits */
#define SEM_VALUE_MAX 32767 /* Maximum semaphore value (POSIX minimum) */

/* Semaphore management (PSE51 required) */
int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_post(sem_t *sem);
int sem_getvalue(sem_t *sem, int *sval);

#endif /* __SEMAPHORE_H__ */
