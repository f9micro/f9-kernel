# F9 POSIX Compatibility Layer (PSE51 + PSE52 Partial)

## Overview

This library provides POSIX API compatibility for F9 microkernel applications,
implementing the PSE51 (POSIX Minimal Realtime System Profile) as defined in IEEE Std 1003.13-2003,
plus selected PSE52 (Realtime Controller) features.

PSE51 is designed for embedded hard real-time systems and includes:
- POSIX Threads (pthread) with condition variables
- Semaphores with notification-based blocking
- Mutexes with notification-based blocking and static initializers
- Thread scheduling (`SCHED_FIFO`, `SCHED_RR`)
- Clocks and timers (`clock_gettime`, `nanosleep`)
- Signal handling (`sigwait`, `pthread_sigmask`)

PSE52 extensions implemented:
- Read-write locks (`pthread_rwlock_*`)
- Barriers (`pthread_barrier_*`)

## Architecture

The implementation follows the seL4 ecosystem layering pattern:

```
┌────────────────────────────────────┐
│  POSIX Application Code            │
├────────────────────────────────────┤
│  libposix (PSE51 API)              │ ← This library
├────────────────────────────────────┤
│  L4 Syscalls (IPC, ThreadControl)  │
├────────────────────────────────────┤
│  F9 Microkernel                    │
└────────────────────────────────────┘
```

### Key Design Decisions

1. Thread Management via Pager IPC
   - `pthread_create()` → `pager_create_thread()` → L4 ThreadControl syscall
   - Threads share address space with creator (L4 shared AS model)
   - Stack/UTCB allocated from pager's resource pool
   - `pthread_join()` uses pager IPC (last remaining pager sync handler)

2. Mutex Implementation (Notification-Based)
   - ARM LDREX/STREX atomic operations for fast-path (uncontended case)
   - Direct kernel notification blocking via `L4_NotifyWait(POSIX_NOTIFY_MUTEX_BIT)`
   - TTAS spinlock pattern for waiter list protection
   - Deadlock detection for normal mutexes
   - Recursive mutex support (`PTHREAD_MUTEX_RECURSIVE`)
   - Static initializer support (`PTHREAD_MUTEX_INITIALIZER`)
   - Lazy initialization on first use (following posix-next pattern)

3. Semaphore Implementation (Notification-Based)
   - Atomic counter using ARM exclusive load/store for fast-path
   - Direct kernel notification blocking via `L4_NotifyWait(SEM_NOTIFY_BIT)`
   - Waiter list with spinlock protection; `sem_post()` wakes via `L4_NotifyPost()`
   - Supports `sem_wait`, `sem_post`, `sem_trywait`, `sem_getvalue`

4. Condition Variables (Sequence-Based Atomicity)
   - Full `pthread_cond_*` implementation with timed wait
   - Sequence counter (`signal_count`, `broadcast_seq`) for lost-wakeup prevention
   - `pthread_cond_wait` captures sequence before mutex release, preventing race
   - Polling-based timedwait using `L4_SystemClock` (`SYS_SYSTEM_CLOCK`)
   - Proper signal/broadcast semantics with waiter list + notification

## API Coverage

### POSIX Threads (pthread.h)

Thread Management:
- `pthread_create()` - Create new thread
- `pthread_exit()` - Terminate calling thread
- `pthread_join()` - Wait for thread termination
- `pthread_detach()` - Mark thread as detached
- `pthread_self()` - Get current thread ID
- `pthread_equal()` - Compare thread IDs

Thread Cancellation:
- `pthread_cancel()` - Request thread cancellation
- `pthread_setcancelstate()` - Set cancellation state (enable/disable)
- `pthread_setcanceltype()` - Set cancellation type (deferred only)
- `pthread_testcancel()` - Create cancellation point

Thread Attributes:
- `pthread_attr_init()` - Initialize attributes
- `pthread_attr_destroy()` - Destroy attributes
- `pthread_attr_setdetachstate()` - Set detach state
- `pthread_attr_getdetachstate()` - Get detach state
- `pthread_attr_setstacksize()` - Set stack size
- `pthread_attr_getstacksize()` - Get stack size

Mutexes:
- `pthread_mutex_init()` - Initialize mutex
- `pthread_mutex_destroy()` - Destroy mutex
- `pthread_mutex_lock()` - Lock mutex (blocking)
- `pthread_mutex_trylock()` - Try to lock mutex (non-blocking)
- `pthread_mutex_timedlock()` - Lock mutex with timeout
- `pthread_mutex_unlock()` - Unlock mutex

Mutex Attributes:
- `pthread_mutexattr_init()` - Initialize mutex attributes
- `pthread_mutexattr_destroy()` - Destroy mutex attributes
- `pthread_mutexattr_settype()` - Set mutex type (normal/recursive)
- `pthread_mutexattr_gettype()` - Get mutex type

Condition Variables:
- `pthread_cond_init()` - Initialize condition variable
- `pthread_cond_destroy()` - Destroy condition variable
- `pthread_cond_wait()` - Wait on condition (releases mutex atomically)
- `pthread_cond_timedwait()` - Wait with timeout (polling-based)
- `pthread_cond_signal()` - Wake one waiting thread
- `pthread_cond_broadcast()` - Wake all waiting threads

Condition Variable Attributes:
- `pthread_condattr_init()` - Initialize condition attributes
- `pthread_condattr_destroy()` - Destroy condition attributes

### Semaphores (semaphore.h)

- `sem_init()` - Initialize semaphore
- `sem_destroy()` - Destroy semaphore
- `sem_wait()` - Wait on semaphore (decrement, blocking)
- `sem_trywait()` - Try to wait on semaphore (non-blocking)
- `sem_post()` - Post to semaphore (increment)
- `sem_getvalue()` - Get current semaphore value

### Time and Clocks (time.h)

Clocks:
- `clock_gettime()` - Get current time from specified clock
- `clock_settime()` - Set time on specified clock
- `clock_getres()` - Get clock resolution

Sleep:
- `nanosleep()` - High-resolution sleep

Timers:
- `timer_create()` - Create a timer
- `timer_delete()` - Delete a timer
- `timer_settime()` - Arm/disarm a timer
- `timer_gettime()` - Get remaining time
- `timer_getoverrun()` - Get overrun count

Clock IDs: `CLOCK_REALTIME`, `CLOCK_MONOTONIC`

### Signals (signal.h)

Signal Set Operations:
- `sigemptyset()` - Initialize empty signal set
- `sigfillset()` - Initialize full signal set
- `sigaddset()` - Add signal to set
- `sigdelset()` - Remove signal from set
- `sigismember()` - Test signal membership

Signal Masking:
- `pthread_sigmask()` - Examine/change thread signal mask
- `sigprocmask()` - Process signal mask (single-threaded)

Signal Handling:
- `sigaction()` - Examine/change signal action
- `sigwait()` - Wait for signal from set
- `sigpending()` - Get pending signals
- `pthread_kill()` - Send signal to thread
- `raise()` - Send signal to self

Signals: `SIGTERM`, `SIGKILL`, `SIGUSR1`, `SIGUSR2`, `SIGALRM`, `SIGINT`

### Scheduling (sched.h)

Priority Functions:
- `sched_get_priority_max()` - Get maximum priority for policy
- `sched_get_priority_min()` - Get minimum priority for policy
- `sched_yield()` - Yield processor

Process Scheduling:
- `sched_getscheduler()` - Get scheduling policy
- `sched_setscheduler()` - Set scheduling policy and parameters
- `sched_getparam()` - Get scheduling parameters
- `sched_setparam()` - Set scheduling parameters

Thread Scheduling:
- `pthread_setschedparam()` - Set thread scheduling policy/priority
- `pthread_getschedparam()` - Get thread scheduling policy/priority
- `pthread_setschedprio()` - Set thread priority
- `pthread_attr_setschedpolicy()` - Set scheduling policy in attributes
- `pthread_attr_getschedpolicy()` - Get scheduling policy from attributes
- `pthread_attr_setschedparam()` - Set scheduling params in attributes
- `pthread_attr_getschedparam()` - Get scheduling params from attributes
- `pthread_attr_setinheritsched()` - Set inherit-scheduler attribute
- `pthread_attr_getinheritsched()` - Get inherit-scheduler attribute

Policies: `SCHED_FIFO`, `SCHED_RR`, `SCHED_OTHER`

### PSE52: Read-Write Locks (pthread.h)

RW Lock Management:
- `pthread_rwlock_init()` - Initialize read-write lock
- `pthread_rwlock_destroy()` - Destroy read-write lock
- `pthread_rwlock_rdlock()` - Acquire read lock (blocking)
- `pthread_rwlock_tryrdlock()` - Try to acquire read lock (non-blocking)
- `pthread_rwlock_wrlock()` - Acquire write lock (blocking)
- `pthread_rwlock_trywrlock()` - Try to acquire write lock (non-blocking)
- `pthread_rwlock_unlock()` - Release read or write lock

RW Lock Attributes:
- `pthread_rwlockattr_init()` - Initialize attributes
- `pthread_rwlockattr_destroy()` - Destroy attributes

Static initializer: `PTHREAD_RWLOCK_INITIALIZER`

### PSE52: Barriers (pthread.h)

Barrier Management:
- `pthread_barrier_init()` - Initialize barrier with thread count
- `pthread_barrier_destroy()` - Destroy barrier
- `pthread_barrier_wait()` - Wait at barrier synchronization point

Barrier Attributes:
- `pthread_barrierattr_init()` - Initialize attributes
- `pthread_barrierattr_destroy()` - Destroy attributes

Return value: One thread receives `PTHREAD_BARRIER_SERIAL_THREAD`

### Types (sys/types.h)

- `pthread_t` - Thread ID structure
- `pthread_attr_t` - Thread attributes
- `pthread_mutex_t` - Mutex structure
- `pthread_mutexattr_t` - Mutex attributes (bitfield-packed)
- `pthread_cond_t` - Condition variable structure
- `pthread_condattr_t` - Condition variable attributes (bitfield-packed)
- `pthread_rwlock_t` - Read-write lock structure (PSE52)
- `pthread_rwlockattr_t` - RW lock attributes (PSE52)
- `pthread_barrier_t` - Barrier structure (PSE52)
- `pthread_barrierattr_t` - Barrier attributes (PSE52)
- `sem_t` - Semaphore structure
- `sigset_t` - Signal set
- `struct timespec` - Time specification
- `struct sched_param` - Scheduling parameters
- `clockid_t` - Clock identifier
- `timer_t` - Timer identifier

Static Initializers:
- `PTHREAD_MUTEX_INITIALIZER` - Static mutex initialization
- `PTHREAD_COND_INITIALIZER` - Static condition variable initialization
- `PTHREAD_RWLOCK_INITIALIZER` - Static RW lock initialization

## Usage Example

### Basic Thread Creation

```c
#include <libposix/pthread.h>

void *worker_thread(void *arg)
{
    int id = *(int *)arg;
    printf("Worker %d started\n", id);
    return (void *)(id * 2);
}

int main(void)
{
    pthread_t thread;
    int thread_id = 42;
    void *retval;

    pthread_create(&thread, NULL, worker_thread, &thread_id);
    pthread_join(&thread, &retval);

    printf("Thread returned: %d\n", (int)retval);
    return 0;
}
```

### Mutex Synchronization

```c
#include <libposix/pthread.h>

pthread_mutex_t mutex;
int shared_counter = 0;

void *increment_thread(void *arg)
{
    for (int i = 0; i < 1000; i++) {
        pthread_mutex_lock(&mutex);
        shared_counter++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(void)
{
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads[4];
    for (int i = 0; i < 4; i++)
        pthread_create(&threads[i], NULL, increment_thread, NULL);

    for (int i = 0; i < 4; i++)
        pthread_join(&threads[i], NULL);

    printf("Final counter: %d\n", shared_counter);  // Should be 4000
    pthread_mutex_destroy(&mutex);
    return 0;
}
```

### Producer-Consumer with Semaphores

```c
#include <libposix/semaphore.h>
#include <libposix/pthread.h>

sem_t sem;
int buffer;

void *producer(void *arg)
{
    for (int i = 0; i < 10; i++) {
        buffer = i;
        sem_post(&sem);
    }
    return NULL;
}

void *consumer(void *arg)
{
    for (int i = 0; i < 10; i++) {
        sem_wait(&sem);
        printf("Consumed: %d\n", buffer);
    }
    return NULL;
}

int main(void)
{
    sem_init(&sem, 0, 0);

    pthread_t prod, cons;
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    pthread_join(&prod, NULL);
    pthread_join(&cons, NULL);

    sem_destroy(&sem);
    return 0;
}
```

### Read-Write Lock (PSE52)

```c
#include <libposix/pthread.h>

/* Static initialization - no init() call needed */
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
int shared_data = 0;

void *reader(void *arg)
{
    pthread_rwlock_rdlock(&rwlock);
    /* Multiple readers can access concurrently */
    printf("Read value: %d\n", shared_data);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void *writer(void *arg)
{
    pthread_rwlock_wrlock(&rwlock);
    /* Writers have exclusive access */
    shared_data++;
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}
```

### Barrier Synchronization (PSE52)

```c
#include <libposix/pthread.h>

pthread_barrier_t barrier;

void *worker(void *arg)
{
    int id = *(int *)arg;
    printf("Thread %d: Phase 1 work\n", id);

    /* All threads wait here until count reached */
    int ret = pthread_barrier_wait(&barrier);
    if (ret == PTHREAD_BARRIER_SERIAL_THREAD) {
        printf("Thread %d: I'm the serial thread!\n", id);
    }

    printf("Thread %d: Phase 2 work\n", id);
    return NULL;
}

int main(void)
{
    pthread_barrier_init(&barrier, NULL, 4);

    pthread_t threads[4];
    int ids[4] = {0, 1, 2, 3};
    for (int i = 0; i < 4; i++)
        pthread_create(&threads[i], NULL, worker, &ids[i]);

    for (int i = 0; i < 4; i++)
        pthread_join(&threads[i], NULL);

    pthread_barrier_destroy(&barrier);
    return 0;
}
```

## Compliance Testing

Comprehensive PSE51 compliance tests are available in `user/apps/posix/`:

```bash
make config
# Enable POSIX tests in configuration
make
qemu-system-arm -M netduinoplus2 -nographic -serial mon:stdio -kernel build/netduinoplus2/f9.elf
```

**Test Results:** 25 tests passing (17 pthread + 8 semaphore)

Test coverage includes:
- Thread creation, join, detach with return values
- Thread attributes (stack size, detach state)
- Thread cancellation (cancel, testcancel, setcancelstate)
- Mutex locking (normal, recursive, timed)
- Mutex error detection (deadlock, double-unlock)
- Condition variable wait/timedwait/signal/broadcast
- Spinlock init/destroy, lock/unlock, trylock, error cases
- Semaphore wait/post/trywait/getvalue
- Producer-consumer patterns
- Multi-threaded stress tests
- Notification-based blocking verification

## Limitations

1. No Thread-Local Storage (TLS)
   - `pthread_exit()` stores return value via pager IPC (not true TLS)
   - `pthread_key_*` APIs not implemented
2. No Priority Inheritance
   - Mutexes do not implement `PTHREAD_PRIO_INHERIT` protocol
   - `pthread_mutexattr_*protocol/prioceiling` return `ENOTSUP`
3. Global Signal State (Not Per-Thread)
   - `pending_signals` and `thread_sigmask` are process-global
   - PSE51 requires per-thread signal delivery
   - Fix would require kernel TLS support or per-TCB signal state
4. Thread Cancellation Limitations
   - Only `PTHREAD_CANCEL_DEFERRED` supported (asynchronous cancellation not safe for RT)
   - No cleanup handlers (`pthread_cleanup_push/pop` not implemented)
   - Cancellation points limited to `pthread_testcancel()` only
5. Timed Wait Uses Relative Interpretation
   - `pthread_cond_timedwait`: Treats abstime as relative, adds to current time
   - True CLOCK_REALTIME absolute time would require epoch reference
   - `sem_timedwait`: Not yet implemented
6. Stack Size Attribute Not Enforced
   - `pthread_attr_setstacksize()` accepted but not passed to pager
   - All threads use default stack size from pager configuration

## PSE51/PSE52 Conformance Status

| Category | Implemented | Missing/Stub |
|----------|-------------|--------------|
| Threads | create, join, detach, exit, self, equal, attr_*, cancel, setcancelstate/type, testcancel | cleanup_push/pop |
| Mutexes | init, destroy, lock, trylock, timedlock, unlock | - |
| Mutex Attrs | init, destroy, settype, gettype | setprotocol, setprioceiling (ENOTSUP) |
| Condvars | init, destroy, wait, timedwait, signal, broadcast | - |
| Condvar Attrs | init, destroy | setclock (ENOTSUP) |
| Semaphores | init, destroy, wait, trywait, post, getvalue | timedwait |
| RW Locks | init, destroy, rdlock, wrlock, tryrd/wrlock, unlock | timedrdlock, timedwrlock |
| Barriers | init, destroy, wait | - |
| Spinlocks | init, destroy, lock, trylock, unlock | - |
| Signals | sigmask, sigaction, sigwait, sigpending, raise | Per-thread state (global only) |
| TLS | - | key_create, key_delete, getspecific, setspecific |
| One-time Init | - | pthread_once |
| Scheduling | setschedparam, getschedparam, setschedprio, yield, get_priority_min/max | - |

## References

- IEEE Std 1003.13-2003 (PSE51/PSE52 Profiles)
- The Open Group Base Specifications Issue 7
- seL4 libsel4sync (semaphore/mutex reference implementation)
- Zephyr POSIX layer (externals/posix-next) - elastipool, bitfield patterns
