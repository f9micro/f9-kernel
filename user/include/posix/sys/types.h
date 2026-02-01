/* Copyright (c) 2014-2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __POSIX_TYPES_H_
#define __POSIX_TYPES_H_

#include <l4/types.h>
#include <stdint.h>

/* Time types (POSIX required)
 * tv_nsec is signed per POSIX (allows negative value validation).
 * tv_sec uses int64_t for wide range while supporting negative (pre-epoch).
 */
struct timespec {
    int64_t tv_sec;  /* Seconds */
    int32_t tv_nsec; /* Nanoseconds (signed per POSIX) */
};

/* Thread types (PSE51 Profile) */
typedef struct {
    L4_ThreadId_t tid;      /* L4 thread ID */
    L4_ThreadId_t creator;  /* Creator thread ID (for join protocol) */
    void *(*entry)(void *); /* Entry function */
    void *arg;              /* Entry argument */
    void *retval;           /* Return value from pthread_exit */
    uint32_t detached;      /* Detach state */
    uint32_t joined;        /* Join state */
    uint32_t state;         /* Thread lifecycle state */
} pthread_t;

typedef struct {
    uint32_t priority;    /* Thread priority (1-255) */
    uint32_t stack_size;  /* Stack size (default: 512 bytes) */
    uint32_t detachstate; /* PTHREAD_CREATE_DETACHED/JOINABLE */
} pthread_attr_t;

/* Mutex types (PSE51 Profile)
 * Design notes (from posix-next analysis):
 * - Use sentinel value for lazy initialization support
 * - Bitfield packing for attributes where possible
 * - Uses direct kernel notifications for blocking instead of pager IPC
 */
#define MUTEX_MAX_WAITERS 8 /* Max concurrent waiters per mutex */

typedef struct {
    uint32_t lock;         /* Lock word (0=unlocked, 1=locked) */
    L4_ThreadId_t owner;   /* Current owner thread */
    uint8_t type;          /* Mutex type (normal/recursive) */
    uint8_t initialized;   /* Initialization flag */
    uint16_t count;        /* Lock count (for recursive) */
    uint32_t waiters_lock; /* Spinlock for waiter list serialization */
    uint32_t num_waiters;  /* Number of threads in wait list */
    L4_ThreadId_t waiters[MUTEX_MAX_WAITERS]; /* Waiting thread IDs */
} pthread_mutex_t;

typedef struct {
    uint8_t type : 2;        /* PTHREAD_MUTEX_NORMAL/RECURSIVE (0-3) */
    uint8_t initialized : 1; /* Validation flag */
    uint8_t _reserved : 5;
} pthread_mutexattr_t;

/* Static initializer sentinel - enables lazy initialization
 * Usage: pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
 */
#define PTHREAD_MUTEX_INITIALIZER_MAGIC 0xDEAD
#define PTHREAD_MUTEX_INITIALIZER                                     \
    {                                                                 \
        .lock = 0, .owner = {.raw = 0}, .type = PTHREAD_MUTEX_NORMAL, \
        .initialized = 0, .count = PTHREAD_MUTEX_INITIALIZER_MAGIC,   \
        .waiters_lock = 0, .num_waiters = 0, .waiters = {             \
            {.raw = 0}                                                \
        }                                                             \
    }

/* POSIX synchronization notification bits (namespaced to avoid collision)
 * Bit 0-7: POSIX sync primitives
 * Bit 8-15: Reserved for IRQ notifications (IRQ 0-7 use these)
 * Bit 16-29: IRQ/hardware notifications (higher IRQs)
 * Bit 30-31: Timer notifications / timeouts (safe from IRQ collision)
 */
#define SEM_NOTIFY_BIT (1U << 0)         /* Semaphore wakeup */
#define POSIX_NOTIFY_MUTEX_BIT (1U << 1) /* Mutex wakeup */
#define POSIX_NOTIFY_COND_BIT (1U << 2)  /* Condition variable wakeup */
#define POSIX_NOTIFY_TIMEOUT_BIT \
    (1U << 30) /* Timed wait timeout (high bit avoids IRQ collision) */

/* Semaphore types (PSE51 Profile)
 * Uses direct kernel notifications for blocking instead of pager IPC.
 * Waiter list is embedded for fast O(1) wake operations.
 */
#define SEM_MAX_WAITERS 8 /* Max concurrent waiters per semaphore */

typedef struct {
    uint32_t count;        /* Semaphore count */
    uint32_t pshared;      /* Process-shared flag */
    uint32_t waiters_lock; /* Spinlock for waiter list serialization */
    uint32_t num_waiters;  /* Number of threads in wait list */
    L4_ThreadId_t waiters[SEM_MAX_WAITERS]; /* Waiting thread IDs */
} sem_t;

/* Condition variable types (PSE51 POSIX_THREADS_BASE - MANDATORY)
 * Uses direct kernel notifications for blocking instead of pager IPC.
 */
#define COND_MAX_WAITERS 8 /* Max concurrent waiters per condvar */

typedef struct {
    L4_Word_t wait_count;    /* Number of waiting threads */
    L4_Word_t signal_count;  /* Pending signals */
    L4_Word_t broadcast_seq; /* Broadcast sequence number */
    uint8_t initialized;     /* Initialization flag */
    uint8_t _pad[3];         /* Alignment padding */
    uint32_t waiters_lock;   /* Spinlock for waiter list serialization */
    uint32_t num_waiters;    /* Number of threads in wait list */
    L4_ThreadId_t waiters[COND_MAX_WAITERS]; /* Waiting thread IDs */
} pthread_cond_t;

typedef struct {
    uint8_t pshared : 1;     /* Process-shared attribute */
    uint8_t initialized : 1; /* Validation flag */
    uint8_t _reserved : 6;
} pthread_condattr_t;

/* Condition variable static initializer */
#define PTHREAD_COND_INITIALIZER                                            \
    {                                                                       \
        .wait_count = 0, .signal_count = 0, .broadcast_seq = 0,             \
        .initialized = 1, ._pad = {0}, .waiters_lock = 0, .num_waiters = 0, \
        .waiters = {                                                        \
            {.raw = 0}                                                      \
        }                                                                   \
    }

/* PSE52 Profile: Read-Write Lock types (POSIX_READER_WRITER_LOCKS option) */
typedef struct {
    pthread_mutex_t rd_mutex; /* Reader count protection */
    pthread_mutex_t wr_mutex; /* Writer mutual exclusion */
    uint32_t readers;         /* Active reader count */
    L4_ThreadId_t writer;     /* Current writer thread */
    uint8_t initialized;      /* Initialization flag */
} pthread_rwlock_t;

typedef struct {
    uint8_t pshared : 1;     /* Process-shared attribute */
    uint8_t initialized : 1; /* Validation flag */
    uint8_t _reserved : 6;
} pthread_rwlockattr_t;

/* RW lock static initializer */
#define PTHREAD_RWLOCK_INITIALIZER          \
    {.rd_mutex = PTHREAD_MUTEX_INITIALIZER, \
     .wr_mutex = PTHREAD_MUTEX_INITIALIZER, \
     .readers = 0,                          \
     .writer = {.raw = 0},                  \
     .initialized = 0}

/* PSE52 Profile: Barrier types (POSIX_BARRIERS option) */
typedef struct {
    uint32_t count;        /* Number of threads to synchronize */
    uint32_t waiting;      /* Current number waiting */
    uint32_t cycle;        /* Barrier cycle (for reuse) */
    pthread_mutex_t mutex; /* Internal synchronization */
    pthread_cond_t cond;   /* Wait condition */
    uint8_t initialized;   /* Initialization flag */
} pthread_barrier_t;

typedef struct {
    uint8_t pshared : 1;     /* Process-shared attribute */
    uint8_t initialized : 1; /* Validation flag */
    uint8_t _reserved : 6;
} pthread_barrierattr_t;

/* Barrier serial thread return value */
#define PTHREAD_BARRIER_SERIAL_THREAD (-1)

/* PSE52 Profile: Spinlock types (POSIX_SPIN_LOCKS option)
 * Lightweight busy-wait synchronization for short critical sections.
 * Uses TTAS (Test-and-Test-and-Set) pattern with ARM LDREX/STREX.
 */
typedef struct {
    volatile uint32_t lock; /* Lock state: 0=unlocked, 1=locked */
    uint8_t pshared;        /* Process-shared attribute */
    uint8_t initialized;    /* Initialization flag */
} pthread_spinlock_t;

/* Spinlock pshared attribute values */
#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED 1

/* Scheduling types (PSE51 Profile) */
struct sched_param {
    int sched_priority; /* Scheduling priority */
};

/* Time types for clock/timer APIs */
typedef int clockid_t;
typedef struct {
    L4_Word_t sigev_notify;            /* Notification type */
    L4_ThreadId_t sigev_notify_thread; /* Thread to notify */
} sigevent_t;

/* timer_t defined in time.h to avoid circular dependency */

/* Clock IDs */
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1

/* Scheduling policies */
#define SCHED_OTHER 0
#define SCHED_FIFO 1
#define SCHED_RR 2

/* POSIX constants */
#define PTHREAD_CREATE_DETACHED 1
#define PTHREAD_CREATE_JOINABLE 0

#define PTHREAD_MUTEX_NORMAL 0
#define PTHREAD_MUTEX_RECURSIVE 1

/* Thread cancellation constants (PSE51 POSIX_THREADS_BASE) */
#define PTHREAD_CANCEL_ENABLE 0
#define PTHREAD_CANCEL_DISABLE 1
#define PTHREAD_CANCEL_DEFERRED 0
#define PTHREAD_CANCEL_ASYNCHRONOUS 1 /* Not supported - RT hazard */
#define PTHREAD_CANCELED ((void *) -1)

/* Error codes (subset for PSE51) */
#define EINVAL 22     /* Invalid argument */
#define EBUSY 16      /* Device or resource busy */
#define EAGAIN 11     /* Try again */
#define ENOMEM 12     /* Out of memory */
#define EDEADLK 35    /* Resource deadlock would occur */
#define EPERM 1       /* Operation not permitted */
#define ESRCH 3       /* No such process */
#define EOVERFLOW 75  /* Value too large for defined data type */
#define ETIMEDOUT 110 /* Connection timed out */
#define ENOTSUP 95    /* Operation not supported */

/* Standard types */
typedef uint32_t pid_t;
typedef uint32_t mode_t;

#endif /* __POSIX_TYPES_H_ */
