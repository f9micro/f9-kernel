/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/ipc.h>
#include <l4/schedule.h>
#include <l4/utcb.h>
#include <platform/link.h>
#include <posix/signal.h>
#include <posix/sys/types.h>

/* Minimal signal implementation for PSE51 POSIX_SIGNALS compliance
 *
 * This is a simplified implementation focused on:
 * - Signal set manipulation
 * - Basic signal masking (per-thread)
 * - sigwait for inter-thread notification
 *
 * Per-thread signal state is maintained in a thread-indexed table
 * to provide correct POSIX per-thread semantics.
 *
 * Known limitation: pthread_kill to a thread that exits concurrently may
 * allocate a "zombie" signal slot that is never cleaned up. This is because
 * we cannot verify thread existence from userspace. The slot leak is bounded
 * by SIGNAL_MAX_THREADS and only occurs when signaling dying threads.
 */

#include <l4/platform/syscalls.h>

/* Maximum threads for per-thread signal state */
#define SIGNAL_MAX_THREADS 16

/* Per-thread signal state structure */
struct thread_signal_state {
    L4_ThreadId_t tid;
    sigset_t sigmask;
    sigset_t pending;
};

/* Per-thread signal state table.
 * Must use __USER_BSS to place in user-accessible memory region.
 */
__USER_BSS static struct thread_signal_state thread_signals[SIGNAL_MAX_THREADS];

/* Spinlock protecting signal state table allocation */
__USER_BSS static uint32_t signal_table_lock;

/* Process-wide signal handlers (handlers are shared per POSIX) */
__USER_BSS static struct sigaction signal_handlers[32];

/* Spinlock acquire using TTAS pattern with LDREX/STREX.
 * Consistent with pthread.c spinlock implementation.
 */
__USER_TEXT
static void signal_lock_acquire(void)
{
    uint32_t status;
    while (1) {
        /* Test: Spin on plain load until lock appears free */
        while (*(volatile uint32_t *) &signal_table_lock != 0)
            L4_Yield();

        /* Test-and-Set: Try to acquire with exclusive access */
        __asm__ __volatile__(
            "ldrex r0, [%[lock]]\n"
            "cmp r0, #0\n"
            "bne 2f\n"
            "mov r0, #1\n"
            "strex %[status], r0, [%[lock]]\n"
            "b 3f\n"
            "2: mov %[status], #1\n"
            "3:\n"
            : [status] "=&r"(status)
            : [lock] "r"(&signal_table_lock)
            : "r0", "cc", "memory");

        if (status == 0) {
            __asm__ __volatile__("dmb" ::: "memory"); /* Acquire barrier */
            return;
        }
    }
}

__USER_TEXT
static void signal_lock_release(void)
{
    __asm__ __volatile__("dmb" ::: "memory"); /* Release barrier */
    signal_table_lock = 0;
}

/* Find or create per-thread signal state for current thread.
 * Thread-safe: uses spinlock to protect allocation.
 * Returns NULL if table is full (caller must handle EAGAIN).
 */
__USER_TEXT
static struct thread_signal_state *get_thread_signal_state(void)
{
    L4_ThreadId_t self = L4_MyGlobalId();
    int free_slot = -1;

    signal_lock_acquire();

    /* Find existing entry or first free slot */
    for (int i = 0; i < SIGNAL_MAX_THREADS; i++) {
        if (thread_signals[i].tid.raw == self.raw) {
            signal_lock_release();
            return &thread_signals[i];
        }
        if (free_slot < 0 && thread_signals[i].tid.raw == 0)
            free_slot = i;
    }

    /* Create new entry if slot available */
    if (free_slot >= 0) {
        thread_signals[free_slot].tid = self;
        thread_signals[free_slot].sigmask = 0;
        thread_signals[free_slot].pending = 0;
        signal_lock_release();
        return &thread_signals[free_slot];
    }

    signal_lock_release();

    /* Table full - return NULL to signal error */
    return NULL;
}

/* Deliver signal to a specific thread atomically under lock.
 * Prevents use-after-free by holding lock during find + write.
 * Creates entry for threads without signal state (POSIX requires delivery).
 * Returns 0 on success, EAGAIN if signal table is full.
 */
__USER_TEXT
static int deliver_signal_locked(L4_ThreadId_t tid, int sig)
{
    int free_slot = -1;

    signal_lock_acquire();

    /* Find existing entry or first free slot */
    for (int i = 0; i < SIGNAL_MAX_THREADS; i++) {
        if (thread_signals[i].tid.raw == tid.raw) {
            /* Use atomic OR to be consistent with other pending accesses.
             * Lock is held to prevent use-after-free from cleanup.
             */
            __atomic_fetch_or(&thread_signals[i].pending, (1U << sig),
                              __ATOMIC_RELEASE);
            signal_lock_release();
            return 0;
        }
        if (free_slot < 0 && thread_signals[i].tid.raw == 0)
            free_slot = i;
    }

    /* Create entry for thread that hasn't initialized signal state.
     * This ensures signals to valid threads are not dropped.
     */
    if (free_slot >= 0) {
        thread_signals[free_slot].tid = tid;
        thread_signals[free_slot].sigmask = 0;
        __atomic_store_n(&thread_signals[free_slot].pending, (1U << sig),
                         __ATOMIC_RELEASE);
        signal_lock_release();
        return 0;
    }

    signal_lock_release();

    /* Signal table full */
    return EAGAIN;
}

/* Release signal state slot when thread exits.
 * Called from pthread_exit() or thread cleanup.
 */
__USER_TEXT
void __signal_thread_cleanup(L4_ThreadId_t tid)
{
    signal_lock_acquire();
    for (int i = 0; i < SIGNAL_MAX_THREADS; i++) {
        if (thread_signals[i].tid.raw == tid.raw) {
            thread_signals[i].tid.raw = 0;
            thread_signals[i].sigmask = 0;
            thread_signals[i].pending = 0;
            break;
        }
    }
    signal_lock_release();
}

/* Signal set operations */

__USER_TEXT
int sigemptyset(sigset_t *set)
{
    if (!set)
        return EINVAL;

    *set = 0;
    return 0;
}

__USER_TEXT
int sigfillset(sigset_t *set)
{
    if (!set)
        return EINVAL;

    *set = ~((sigset_t) 0);
    return 0;
}

__USER_TEXT
int sigaddset(sigset_t *set, int signo)
{
    if (!set || signo < 1 || signo > 31)
        return EINVAL;

    *set |= (1U << signo);
    return 0;
}

__USER_TEXT
int sigdelset(sigset_t *set, int signo)
{
    if (!set || signo < 1 || signo > 31)
        return EINVAL;

    *set &= ~(1U << signo);
    return 0;
}

__USER_TEXT
int sigismember(const sigset_t *set, int signo)
{
    if (!set || signo < 1 || signo > 31)
        return EINVAL;

    return (*set & (1U << signo)) ? 1 : 0;
}

/* Signal mask operations */

__USER_TEXT
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oset)
{
    struct thread_signal_state *state = get_thread_signal_state();
    if (!state)
        return EAGAIN; /* Signal table full */

    if (oset)
        *oset = state->sigmask;

    if (!set)
        return 0;

    switch (how) {
    case SIG_BLOCK:
        state->sigmask |= *set;
        break;
    case SIG_UNBLOCK:
        state->sigmask &= ~(*set);
        break;
    case SIG_SETMASK:
        state->sigmask = *set;
        break;
    default:
        return EINVAL;
    }

    return 0;
}

__USER_TEXT
int sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
    /* For single-threaded compatibility */
    return pthread_sigmask(how, set, oset);
}

/* Signal action - protected by lock to prevent torn reads/writes */

__USER_TEXT
int sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
    if (sig < 1 || sig > 31)
        return EINVAL;

    if (sig == SIGKILL || sig == SIGSTOP)
        return EINVAL; /* Cannot catch or ignore SIGKILL/SIGSTOP */

    signal_lock_acquire();

    if (oact)
        *oact = signal_handlers[sig];

    if (act)
        signal_handlers[sig] = *act;

    signal_lock_release();

    return 0;
}

/* Wait for signals */

__USER_TEXT
int sigwait(const sigset_t *set, int *sig)
{
    if (!set || !sig)
        return EINVAL;

    struct thread_signal_state *state = get_thread_signal_state();
    if (!state)
        return EAGAIN; /* Signal table full */

    /* Simplified implementation: poll pending signals
     * FIXME: Full implementation would block via IPC.
     */
    int i;
    while (1) {
        for (i = 1; i <= 31; i++) {
            sigset_t mask = (1U << i);
            if ((*set & mask) &&
                (__atomic_load_n(&state->pending, __ATOMIC_ACQUIRE) & mask)) {
                /* Atomically clear the pending bit to prevent lost updates
                 * from concurrent pthread_kill.
                 */
                __atomic_fetch_and(&state->pending, ~mask, __ATOMIC_RELEASE);
                *sig = i;
                return 0;
            }
        }

        /* FIXME: Yield and retry - proper impl would block */
        L4_Sleep(L4_TimePeriod(1000)); /* 1ms yield */
    }

    return 0;
}

__USER_TEXT
int sigpending(sigset_t *set)
{
    if (!set)
        return EINVAL;

    struct thread_signal_state *state = get_thread_signal_state();
    if (!state)
        return EAGAIN; /* Signal table full */

    /* Atomic load to get consistent view of pending signals */
    *set = __atomic_load_n(&state->pending, __ATOMIC_ACQUIRE) & state->sigmask;
    return 0;
}

/* Send signals */

__USER_TEXT
int pthread_kill(pthread_t *thread, int sig)
{
    if (!thread || sig < 0 || sig > 31)
        return EINVAL;

    if (sig == 0) {
        /* POSIX: Signal 0 is for existence check only.
         * Limitation: We cannot verify thread existence from userspace,
         * so we always return success. This avoids false ESRCH for valid
         * threads that haven't initialized signal state.
         */
        return 0;
    }

    /* Deliver signal atomically under lock to prevent use-after-free. */
    return deliver_signal_locked(thread->tid, sig);
}

__USER_TEXT
int raise(int sig)
{
    if (sig < 1 || sig > 31)
        return EINVAL;

    struct thread_signal_state *state = get_thread_signal_state();
    if (!state)
        return EAGAIN; /* Signal table full */

    /* Atomic OR to prevent lost updates from concurrent pthread_kill */
    __atomic_fetch_or(&state->pending, (1U << sig), __ATOMIC_RELEASE);
    return 0;
}
