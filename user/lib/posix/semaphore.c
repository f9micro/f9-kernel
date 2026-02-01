/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/ipc.h>
#include <l4/schedule.h>
#include <l4/thread.h>
#include <l4/utcb.h>
#include <platform/link.h>
#include <posix/semaphore.h>
#include __L4_INC_ARCH(syscalls.h)

/* Semaphore implementation using direct kernel notifications (PSE51 Profile)
 *
 * This implementation eliminates pager IPC round-trips by using the kernel's
 * notification mechanism directly. Waiting threads register in the sem_t's
 * waiter list and block via L4_NotifyWait(). When sem_post() increments the
 * count, it wakes one waiter via L4_NotifyPost().
 *
 * Memory ordering:
 * - sem_trywait: Acquire barrier after successful decrement
 * - sem_post: Release barrier before increment (no acquire needed for post)
 *
 * Waiter list serialization:
 * - A spinlock (waiters_lock) protects all waiter list operations
 * - This ensures atomicity of add/remove/pop even with list shifts
 */

/* Spinlock acquire using TTAS pattern with LDREX/STREX.
 * Test-and-Test-and-Set reduces bus traffic by spinning on plain load
 * (cache-friendly) before attempting exclusive access.
 */
__USER_TEXT
static void spinlock_acquire(uint32_t *lock)
{
    uint32_t status;
    while (1) {
        /* Test: Spin on plain load until lock appears free */
        while (*(volatile uint32_t *) lock != 0) {
            /* Spin - cache-friendly, no exclusive marking */
        }

        /* Test-and-Set: Try to acquire with exclusive access */
        __asm__ __volatile__(
            "ldrex r0, [%[lock]]\n"
            "cmp r0, #0\n"
            "bne 2f\n" /* If locked (lost race), fail */
            "mov r0, #1\n"
            "strex %[status], r0, [%[lock]]\n"
            "b 3f\n"
            "2: mov %[status], #1\n" /* Mark as failed */
            "3:\n"
            : [status] "=&r"(status)
            : [lock] "r"(lock)
            : "r0", "cc", "memory");

        if (status == 0) {
            __asm__ __volatile__("dmb" ::: "memory"); /* Acquire */
            return;
        }
        /* Retry if STREX failed or lock was taken between test and TAS */
    }
}

/* Spinlock release */
__USER_TEXT
static void spinlock_release(uint32_t *lock)
{
    __asm__ __volatile__("dmb" ::: "memory"); /* Release barrier */
    *lock = 0;
}

/* Add current thread to waiter list. Returns 0 on success, -1 if full.
 * Must be called with waiters_lock held.
 */
__USER_TEXT
static int waiter_list_add_locked(sem_t *sem, L4_ThreadId_t tid)
{
    if (sem->num_waiters >= SEM_MAX_WAITERS)
        return -1; /* List full */

    sem->waiters[sem->num_waiters] = tid;
    sem->num_waiters++;
    return 0;
}

/* Remove a specific thread from waiter list. Returns 0 on success.
 * Must be called with waiters_lock held.
 */
__USER_TEXT
static int waiter_list_remove_locked(sem_t *sem, L4_ThreadId_t tid)
{
    for (uint32_t i = 0; i < sem->num_waiters; i++) {
        if (sem->waiters[i].raw == tid.raw) {
            /* Shift remaining entries down */
            for (uint32_t j = i; j < sem->num_waiters - 1; j++)
                sem->waiters[j] = sem->waiters[j + 1];
            sem->num_waiters--;
            return 0;
        }
    }
    return -1; /* Not found */
}

/* Pop first waiter from list. Returns nilthread if list empty.
 * Must be called with waiters_lock held.
 */
__USER_TEXT
static L4_ThreadId_t waiter_list_pop_locked(sem_t *sem)
{
    if (sem->num_waiters == 0)
        return L4_nilthread;

    L4_ThreadId_t tid = sem->waiters[0];

    /* Shift remaining entries down */
    for (uint32_t i = 0; i < sem->num_waiters - 1; i++)
        sem->waiters[i] = sem->waiters[i + 1];
    sem->num_waiters--;

    return tid;
}

__USER_TEXT
int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    if (!sem)
        return EINVAL;

    if (value > SEM_VALUE_MAX)
        return EINVAL;

    sem->count = value;
    sem->pshared = pshared;
    sem->waiters_lock = 0;
    sem->num_waiters = 0;
    for (int i = 0; i < SEM_MAX_WAITERS; i++)
        sem->waiters[i] = L4_nilthread;

    return 0;
}

__USER_TEXT
int sem_destroy(sem_t *sem)
{
    if (!sem)
        return EINVAL;

    /* POSIX: "The effect of destroying a semaphore upon which other threads
     * are currently blocked is undefined." Most implementations return EBUSY.
     * We follow this convention to prevent hanging waiters.
     */
    spinlock_acquire(&sem->waiters_lock);
    if (sem->num_waiters > 0) {
        spinlock_release(&sem->waiters_lock);
        return EBUSY;
    }
    sem->count = 0;
    spinlock_release(&sem->waiters_lock);

    return 0;
}

__USER_TEXT
int sem_wait(sem_t *sem)
{
    if (!sem)
        return EINVAL;

    L4_ThreadId_t self = L4_MyGlobalId();

    /* Blocking semaphore implementation using direct notifications.
     *
     * Fast path: Try atomic decrement for uncontended case.
     * Slow path: Register in waiter list and block via L4_NotifyWait.
     *
     * The retry loop handles:
     * 1. Lost wakeups (sem_post before we started waiting)
     * 2. Spurious wakeups
     * 3. Waiter list full (yield and retry)
     */
    while (1) {
        /* Fast path: try to decrement */
        if (sem_trywait(sem) == 0)
            return 0; /* Successfully decremented */

        /* Slow path: register as waiter and block */
        spinlock_acquire(&sem->waiters_lock);
        int add_result = waiter_list_add_locked(sem, self);
        spinlock_release(&sem->waiters_lock);

        if (add_result < 0) {
            /* Waiter list full - yield and retry */
            L4_Yield();
            continue;
        }

        /* Double-check before blocking (sem_post may have raced) */
        if (sem_trywait(sem) == 0) {
            /* Success! Remove ourselves from waiter list */
            spinlock_acquire(&sem->waiters_lock);
            waiter_list_remove_locked(sem, self);
            spinlock_release(&sem->waiters_lock);
            return 0;
        }

        /* Block waiting for notification */
        L4_NotifyWait(SEM_NOTIFY_BIT);

        /* Woken up - remove self from list and retry */
        spinlock_acquire(&sem->waiters_lock);
        waiter_list_remove_locked(sem, self);
        spinlock_release(&sem->waiters_lock);

        /* Retry sem_trywait to handle:
         * 1. Normal wakeup from sem_post
         * 2. Spurious wakeup
         * The loop continues until we successfully decrement
         */
    }
}

__USER_TEXT
int sem_trywait(sem_t *sem)
{
    if (!sem)
        return EINVAL;

    /* Atomic decrement using LDREX/STREX */
    register uint32_t old_count;
    register uint32_t new_count;
    register uint32_t status;

    __asm__ __volatile__(
        "1:\n"
        "ldrex %[old], [%[ptr]]\n"
        "cmp %[old], #0\n"
        "beq 2f\n"
        "sub %[new], %[old], #1\n"
        "strex %[status], %[new], [%[ptr]]\n"
        "cmp %[status], #0\n"
        "bne 1b\n"
        "dmb\n" /* Acquire barrier on success */
        "b 3f\n"
        "2:\n"
        "clrex\n"
        "mov %[status], #1\n"
        "3:\n"
        : [old] "=&r"(old_count), [new] "=&r"(new_count), [status] "=&r"(status)
        : [ptr] "r"(&sem->count)
        : "cc", "memory");

    return (status == 0) ? 0 : EAGAIN;
}

__USER_TEXT
int sem_post(sem_t *sem)
{
    if (!sem)
        return EINVAL;

    /* Release barrier before increment */
    __asm__ __volatile__("dmb" ::: "memory");

    /* Atomic increment with overflow check BEFORE store.
     * This prevents the race where concurrent sem_post calls at
     * SEM_VALUE_MAX-1 boundary could both succeed and overflow.
     */
    register uint32_t old_val;
    register uint32_t new_val;
    register uint32_t success;

    do {
        __asm__ __volatile__("ldrex %[old], [%[sem_addr]]\n"
                             : [old] "=r"(old_val)
                             : [sem_addr] "r"(&sem->count)
                             : "memory");

        /* Check overflow BEFORE attempting store */
        if (old_val >= SEM_VALUE_MAX) {
            __asm__ __volatile__("clrex" ::: "memory");
            return EOVERFLOW;
        }

        new_val = old_val + 1;

        __asm__ __volatile__("strex %[success], %[new], [%[sem_addr]]\n"
                             : [success] "=&r"(success)
                             : [new] "r"(new_val), [sem_addr] "r"(&sem->count)
                             : "memory");
    } while (success != 0);

    /* Wake one waiting thread via direct notification */
    spinlock_acquire(&sem->waiters_lock);
    L4_ThreadId_t waiter = waiter_list_pop_locked(sem);
    spinlock_release(&sem->waiters_lock);

    if (waiter.raw != L4_nilthread.raw)
        L4_NotifyPost(waiter, SEM_NOTIFY_BIT);

    return 0;
}

__USER_TEXT
int sem_getvalue(sem_t *sem, int *sval)
{
    if (!sem || !sval)
        return EINVAL;

    *sval = (int) sem->count;
    return 0;
}
