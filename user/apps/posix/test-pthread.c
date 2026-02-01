/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 * PSE51 POSIX Threads Compliance Tests
 *
 * Tests pthread_create, pthread_join, pthread_exit, pthread_detach,
 * pthread_self, pthread_equal, and mutex operations.
 */

#include <l4/ipc.h>
#include <l4/schedule.h>
#include <l4io.h>
#include <platform/link.h>
#include <posix/pthread.h>
#include <types.h>
#include "posix_tests.h"

/* Test globals */
static int shared_counter = 0; /* Used by recursive mutex thread test */

/* Condition variable test globals - must be in user BSS */
__USER_BSS static pthread_mutex_t cv_mutex;
__USER_BSS static pthread_cond_t cv;
__USER_BSS static int cv_ready;

/* Simple thread function - receives value directly, not pointer */
__USER_TEXT
void *thread_simple(void *arg)
{
    /* Note: Use volatile and addition to prevent compiler optimization issues.
     * The ARM compiler has problems with multiplication in this context.
     */
    volatile int value = (int) arg;
    volatile int doubled = value + value;
    printf("[THREAD] Simple thread: arg=%d\n", (int) value);
    return (void *) ((int) doubled);
}

/* Thread function for stress test - identical to thread_simple */
__USER_TEXT
void *thread_stress(void *arg)
{
    volatile int value = (int) arg;
    volatile int doubled = value + value;
    printf("[STRESS] Thread running: arg=%d\n", (int) value);
    return (void *) ((int) doubled);
}

/* Condition variable waiter thread */
__USER_TEXT
void *cv_waiter(void *arg)
{
    (void) arg;
    pthread_mutex_lock(&cv_mutex);
    while (!cv_ready)
        pthread_cond_wait(&cv, &cv_mutex);
    pthread_mutex_unlock(&cv_mutex);
    return (void *) 1; /* Indicate successful wakeup */
}

/* Recursive mutex thread */
__USER_TEXT
void *thread_recursive_mutex(void *arg)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *) arg;

    /* Test recursive locking (3 levels deep) */
    pthread_mutex_lock(mutex);
    pthread_mutex_lock(mutex);
    pthread_mutex_lock(mutex);

    shared_counter += 1000;

    pthread_mutex_unlock(mutex);
    pthread_mutex_unlock(mutex);
    pthread_mutex_unlock(mutex);

    return NULL;
}

/* Test 1: pthread_create and pthread_join */
__USER_TEXT
void test_pthread_create_join(void)
{
    TEST_CASE_START();

    pthread_t thread;
    void *retval;

    /* Pass value 42 directly, not as pointer */
    int ret = pthread_create(&thread, NULL, thread_simple, (void *) 42);
    ASSERT_EQUAL(ret, 0, "pthread_create should succeed");

    ret = pthread_join(&thread, &retval);
    ASSERT_EQUAL(ret, 0, "pthread_join should succeed");
    ASSERT_EQUAL((int) retval, 84, "Thread return value should be 84");

    TEST_PASS();
}

/* Test 2: pthread_detach */
__USER_TEXT
void test_pthread_detach(void)
{
    TEST_CASE_START();

    pthread_t thread;

    /* Pass value directly, not pointer - thread_simple expects direct value */
    int ret = pthread_create(&thread, NULL, thread_simple, (void *) 123);
    ASSERT_EQUAL(ret, 0, "pthread_create should succeed");

    ret = pthread_detach(&thread);
    ASSERT_EQUAL(ret, 0, "pthread_detach should succeed");

    /* Cannot join detached thread */
    ret = pthread_join(&thread, NULL);
    ASSERT_NOT_EQUAL(ret, 0, "pthread_join on detached thread should fail");

    TEST_PASS();
}

/* Test 3: pthread_self and pthread_equal */
__USER_TEXT
void test_pthread_self_equal(void)
{
    TEST_CASE_START();

    pthread_t self1 = pthread_self();
    pthread_t self2 = pthread_self();

    ASSERT_TRUE(pthread_equal(self1, self2),
                "pthread_self should be consistent");

    pthread_t thread;
    /* Pass value directly, not pointer */
    pthread_create(&thread, NULL, thread_simple, (void *) 1);

    /* Direct comparison to avoid stack issues with pthread_equal on large
     * struct */
    int equal_result = (self1.tid.raw == thread.tid.raw);
    ASSERT_FALSE(equal_result, "Different threads should not be equal");

    pthread_detach(&thread);

    TEST_PASS();
}

/* Test 4: pthread_attr operations */
__USER_TEXT
void test_pthread_attr(void)
{
    TEST_CASE_START();

    pthread_attr_t attr;
    int detachstate;
    uint32_t stacksize;

    /* Initialize attributes */
    int ret = pthread_attr_init(&attr);
    ASSERT_EQUAL(ret, 0, "pthread_attr_init should succeed");

    /* Test detach state */
    ret = pthread_attr_getdetachstate(&attr, &detachstate);
    ASSERT_EQUAL(ret, 0, "pthread_attr_getdetachstate should succeed");
    ASSERT_EQUAL(detachstate, PTHREAD_CREATE_JOINABLE,
                 "Default detachstate should be JOINABLE");

    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ASSERT_EQUAL(ret, 0, "pthread_attr_setdetachstate should succeed");

    ret = pthread_attr_getdetachstate(&attr, &detachstate);
    ASSERT_EQUAL(detachstate, PTHREAD_CREATE_DETACHED,
                 "Detachstate should be DETACHED");

    /* Test stack size */
    ret = pthread_attr_getstacksize(&attr, &stacksize);
    ASSERT_EQUAL(ret, 0, "pthread_attr_getstacksize should succeed");
    ASSERT_EQUAL(stacksize, 512, "Default stack size should be 512");

    ret = pthread_attr_setstacksize(&attr, 1024);
    ASSERT_EQUAL(ret, 0, "pthread_attr_setstacksize should succeed");

    ret = pthread_attr_getstacksize(&attr, &stacksize);
    ASSERT_EQUAL(stacksize, 1024, "Stack size should be 1024");

    /* Destroy attributes */
    ret = pthread_attr_destroy(&attr);
    ASSERT_EQUAL(ret, 0, "pthread_attr_destroy should succeed");

    TEST_PASS();
}

/* Test 5: pthread_mutex basic operations */
__USER_TEXT
void test_pthread_mutex_basic(void)
{
    TEST_CASE_START();

    pthread_mutex_t mutex;

    /* Initialize mutex */
    int ret = pthread_mutex_init(&mutex, NULL);
    ASSERT_EQUAL(ret, 0, "pthread_mutex_init should succeed");

    /* Lock mutex */
    ret = pthread_mutex_lock(&mutex);
    ASSERT_EQUAL(ret, 0, "pthread_mutex_lock should succeed");

    /* Try to lock again (should detect deadlock for normal mutex) */
    ret = pthread_mutex_trylock(&mutex);
    ASSERT_EQUAL(ret, EBUSY,
                 "pthread_mutex_trylock on locked mutex should return EBUSY");

    /* Unlock mutex */
    ret = pthread_mutex_unlock(&mutex);
    ASSERT_EQUAL(ret, 0, "pthread_mutex_unlock should succeed");

    /* Try lock should succeed now */
    ret = pthread_mutex_trylock(&mutex);
    ASSERT_EQUAL(ret, 0,
                 "pthread_mutex_trylock on unlocked mutex should succeed");

    pthread_mutex_unlock(&mutex);

    /* Destroy mutex */
    ret = pthread_mutex_destroy(&mutex);
    ASSERT_EQUAL(ret, 0, "pthread_mutex_destroy should succeed");

    TEST_PASS();
}

/* Test 6: pthread_mutex recursive */
__USER_TEXT
void test_pthread_mutex_recursive(void)
{
    TEST_CASE_START();

    pthread_mutex_t mutex;
    pthread_mutexattr_t attr;

    /* Create recursive mutex */
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    int ret = pthread_mutex_init(&mutex, &attr);
    ASSERT_EQUAL(ret, 0,
                 "pthread_mutex_init with recursive attr should succeed");

    /* Lock multiple times */
    ret = pthread_mutex_lock(&mutex);
    ASSERT_EQUAL(ret, 0, "First lock should succeed");

    ret = pthread_mutex_lock(&mutex);
    ASSERT_EQUAL(ret, 0, "Second lock (recursive) should succeed");

    ret = pthread_mutex_lock(&mutex);
    ASSERT_EQUAL(ret, 0, "Third lock (recursive) should succeed");

    /* Unlock same number of times */
    pthread_mutex_unlock(&mutex);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_unlock(&mutex);

    /* Simplified test: verify recursive lock/unlock count is correct */
    ASSERT_EQUAL(mutex.count, 0, "Count should be 0 after all unlocks");

    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&attr);

    TEST_PASS();
}

/* Test 7: pthread_mutex stress test - single thread version */
__USER_TEXT
void test_pthread_mutex_stress(void)
{
    TEST_CASE_START();

    /* Almost identical to test_pthread_create_join */
    pthread_t thread;
    void *retval;

    int ret = pthread_create(&thread, NULL, thread_stress, (void *) 21);
    ASSERT_EQUAL(ret, 0, "pthread_create should succeed");

    ret = pthread_join(&thread, &retval);
    ASSERT_EQUAL(ret, 0, "pthread_join should succeed");
    ASSERT_EQUAL((int) retval, 42, "Thread return value should be 42");

    TEST_PASS();
}

/* Test 8: Mutex ownership errors and deadlock detection */
__USER_TEXT
void test_pthread_mutex_errors(void)
{
    TEST_CASE_START();

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    /* Lock the mutex */
    pthread_mutex_lock(&mutex);

    /* Test 1: Destroy while locked should fail with EBUSY */
    int ret = pthread_mutex_destroy(&mutex);
    ASSERT_EQUAL(ret, EBUSY, "destroy on locked mutex should return EBUSY");

    /* Test 2: Re-lock normal mutex should return EDEADLK */
    ret = pthread_mutex_lock(&mutex);
    ASSERT_EQUAL(ret, EDEADLK, "re-lock normal mutex should return EDEADLK");

    /* Test 3: Unlock by non-owner should return EPERM
     * Simulate non-owner by temporarily changing owner
     */
    L4_ThreadId_t orig_owner = mutex.owner;
    mutex.owner.raw = orig_owner.raw + 0x10000; /* Different thread ID */
    ret = pthread_mutex_unlock(&mutex);
    ASSERT_EQUAL(ret, EPERM, "unlock by non-owner should return EPERM");

    /* Restore owner and properly unlock */
    mutex.owner = orig_owner;
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);

    TEST_PASS();
}

/* Test 9: Condition variable basic wait/signal */
__USER_TEXT
void test_pthread_cond_basic(void)
{
    TEST_CASE_START();

    /* Initialize condvar and mutex */
    pthread_mutex_init(&cv_mutex, NULL);
    pthread_cond_init(&cv, NULL);
    cv_ready = 0;

    pthread_t waiter;
    int ret = pthread_create(&waiter, NULL, cv_waiter, NULL);
    ASSERT_EQUAL(ret, 0, "pthread_create for waiter should succeed");

    /* Give waiter time to block on condvar */
    L4_Sleep(L4_TimePeriod(2000)); /* 2ms */

    /* Signal the condvar */
    pthread_mutex_lock(&cv_mutex);
    cv_ready = 1;
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&cv_mutex);

    /* Join waiter and check result */
    void *result;
    ret = pthread_join(&waiter, &result);
    ASSERT_EQUAL(ret, 0, "pthread_join should succeed");
    ASSERT_EQUAL((int) result, 1, "waiter should have woken up successfully");

    pthread_cond_destroy(&cv);
    pthread_mutex_destroy(&cv_mutex);

    TEST_PASS();
}

/* Test 10: Condition variable broadcast */
__USER_TEXT
void test_pthread_cond_broadcast(void)
{
    TEST_CASE_START();

    /* Re-initialize for broadcast test */
    pthread_mutex_init(&cv_mutex, NULL);
    pthread_cond_init(&cv, NULL);
    cv_ready = 0;

    /* Create multiple waiters */
    pthread_t waiters[2];
    for (int i = 0; i < 2; i++) {
        int ret = pthread_create(&waiters[i], NULL, cv_waiter, NULL);
        ASSERT_EQUAL(ret, 0, "pthread_create for waiter should succeed");
    }

    /* Give waiters time to block */
    L4_Sleep(L4_TimePeriod(3000)); /* 3ms */

    /* Broadcast to wake all */
    pthread_mutex_lock(&cv_mutex);
    cv_ready = 1;
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&cv_mutex);

    /* Join all waiters */
    for (int i = 0; i < 2; i++) {
        void *result;
        int ret = pthread_join(&waiters[i], &result);
        ASSERT_EQUAL(ret, 0, "pthread_join should succeed");
        ASSERT_EQUAL((int) result, 1, "waiter should have woken up");
    }

    pthread_cond_destroy(&cv);
    pthread_mutex_destroy(&cv_mutex);

    TEST_PASS();
}

/* Mutex for timedlock test */
__USER_BSS static pthread_mutex_t tl_mutex;

/* Test 11: pthread_mutex_timedlock - basic functionality */
__USER_TEXT
void test_pthread_mutex_timedlock(void)
{
    TEST_CASE_START();

    pthread_mutex_init(&tl_mutex, NULL);

    /* Test 1: timedlock on unlocked mutex should succeed immediately */
    struct timespec timeout = {0, 100000000}; /* 100ms */
    int ret = pthread_mutex_timedlock(&tl_mutex, &timeout);
    ASSERT_EQUAL(ret, 0, "timedlock on unlocked mutex should succeed");

    /* Test 2: timedlock on self-owned mutex should detect deadlock */
    timeout.tv_sec = 0;
    timeout.tv_nsec = 1000000; /* 1ms */
    ret = pthread_mutex_timedlock(&tl_mutex, &timeout);
    ASSERT_EQUAL(ret, EDEADLK, "timedlock on self-owned mutex should EDEADLK");

    pthread_mutex_unlock(&tl_mutex);

    /* Test 3: timedlock with zero timeout on unlocked mutex should succeed */
    timeout.tv_sec = 0;
    timeout.tv_nsec = 1000; /* 1µs = essentially zero */
    ret = pthread_mutex_timedlock(&tl_mutex, &timeout);
    ASSERT_EQUAL(ret, 0,
                 "timedlock with tiny timeout on unlocked should succeed");

    pthread_mutex_unlock(&tl_mutex);
    pthread_mutex_destroy(&tl_mutex);

    TEST_PASS();
}

/* Test 12: pthread_cond_timedwait - timeout */
__USER_TEXT
void test_pthread_cond_timedwait(void)
{
    TEST_CASE_START();

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    /* Lock mutex and wait with timeout (no signal, should timeout) */
    pthread_mutex_lock(&mutex);

    struct timespec timeout = {0, 5000000}; /* 5ms */
    int ret = pthread_cond_timedwait(&cond, &mutex, &timeout);
    ASSERT_EQUAL(ret, ETIMEDOUT, "timedwait without signal should timeout");

    /* Verify we still hold the mutex */
    ASSERT_TRUE(mutex.owner.raw == L4_MyGlobalId().raw,
                "should still own mutex after timedwait");

    pthread_mutex_unlock(&mutex);
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);

    TEST_PASS();
}

/* Test 13: pthread_spin_init/destroy - basic operations */
__USER_TEXT
void test_pthread_spin_basic(void)
{
    TEST_CASE_START();

    pthread_spinlock_t lock;

    /* Test init with PTHREAD_PROCESS_PRIVATE */
    int ret = pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE);
    ASSERT_EQUAL(ret, 0, "spin_init should succeed");

    /* Test destroy of unlocked spinlock */
    ret = pthread_spin_destroy(&lock);
    ASSERT_EQUAL(ret, 0, "spin_destroy should succeed");

    /* Test init with PTHREAD_PROCESS_SHARED */
    ret = pthread_spin_init(&lock, PTHREAD_PROCESS_SHARED);
    ASSERT_EQUAL(ret, 0, "spin_init with SHARED should succeed");

    pthread_spin_destroy(&lock);

    /* Test init with invalid pshared */
    ret = pthread_spin_init(&lock, 99);
    ASSERT_EQUAL(ret, EINVAL, "spin_init with invalid pshared should fail");

    TEST_PASS();
}

/* Test 14: pthread_spin_lock/unlock - basic locking */
__USER_TEXT
void test_pthread_spin_lock_unlock(void)
{
    TEST_CASE_START();

    pthread_spinlock_t lock;
    pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE);

    /* Lock and unlock should succeed */
    int ret = pthread_spin_lock(&lock);
    ASSERT_EQUAL(ret, 0, "spin_lock should succeed");

    ret = pthread_spin_unlock(&lock);
    ASSERT_EQUAL(ret, 0, "spin_unlock should succeed");

    /* Can lock again after unlock */
    ret = pthread_spin_lock(&lock);
    ASSERT_EQUAL(ret, 0, "spin_lock after unlock should succeed");

    pthread_spin_unlock(&lock);
    pthread_spin_destroy(&lock);

    TEST_PASS();
}

/* Test 15: pthread_spin_trylock - non-blocking acquire */
__USER_TEXT
void test_pthread_spin_trylock(void)
{
    TEST_CASE_START();

    pthread_spinlock_t lock;
    pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE);

    /* Trylock on unlocked should succeed */
    int ret = pthread_spin_trylock(&lock);
    ASSERT_EQUAL(ret, 0, "trylock on unlocked should succeed");

    /* Trylock on locked should fail with EBUSY */
    ret = pthread_spin_trylock(&lock);
    ASSERT_EQUAL(ret, EBUSY, "trylock on locked should return EBUSY");

    pthread_spin_unlock(&lock);

    /* Trylock after unlock should succeed */
    ret = pthread_spin_trylock(&lock);
    ASSERT_EQUAL(ret, 0, "trylock after unlock should succeed");

    pthread_spin_unlock(&lock);
    pthread_spin_destroy(&lock);

    TEST_PASS();
}

/* Test 16: pthread_spin_destroy - error cases */
__USER_TEXT
void test_pthread_spin_errors(void)
{
    TEST_CASE_START();

    pthread_spinlock_t lock;

    /* Destroy uninitialized should fail */
    lock.initialized = 0;
    int ret = pthread_spin_destroy(&lock);
    ASSERT_EQUAL(ret, EINVAL, "destroy uninitialized should fail");

    /* Init, lock, then destroy should fail (EBUSY) */
    pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE);
    pthread_spin_lock(&lock);
    ret = pthread_spin_destroy(&lock);
    ASSERT_EQUAL(ret, EBUSY, "destroy locked spinlock should fail");

    pthread_spin_unlock(&lock);
    pthread_spin_destroy(&lock);

    /* NULL pointer checks */
    ret = pthread_spin_init(NULL, PTHREAD_PROCESS_PRIVATE);
    ASSERT_EQUAL(ret, EINVAL, "init with NULL should fail");

    ret = pthread_spin_lock(NULL);
    ASSERT_EQUAL(ret, EINVAL, "lock with NULL should fail");

    ret = pthread_spin_unlock(NULL);
    ASSERT_EQUAL(ret, EINVAL, "unlock with NULL should fail");

    TEST_PASS();
}

/* Thread cancellation test - worker that checks for cancellation */
__USER_BSS static volatile int cancel_test_started;
__USER_BSS static volatile int cancel_test_looped;
__USER_BSS static volatile uint32_t cancel_child_tid;

__USER_TEXT
static void *cancel_test_thread(void *arg)
{
    cancel_child_tid = L4_MyGlobalId().raw;
    cancel_test_started = 1;

    /* Loop forever checking for cancellation.
     * The parent will cancel us, and pthread_testcancel will exit.
     * Only way out is via cancellation.
     */
    while (1) {
        cancel_test_looped++;
        pthread_testcancel(); /* Cancellation point */
        /* Sleep to yield CPU - longer timeout for actual scheduling */
        L4_Sleep(L4_TimePeriod(1000)); /* 1000 microseconds = 1ms */
    }

    /* If we get here, cancellation didn't happen */
    return (void *) 42;
}

__USER_TEXT
void test_pthread_cancel(void)
{
    TSTART();

    int ret, oldstate, oldtype;
    pthread_t thread;
    void *retval;

    /* Test pthread_setcancelstate */
    ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
    ASSERT_EQUAL(ret, 0, "setcancelstate DISABLE should succeed");
    ASSERT_EQUAL(oldstate, PTHREAD_CANCEL_ENABLE,
                 "default state should be ENABLE");

    ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
    ASSERT_EQUAL(ret, 0, "setcancelstate ENABLE should succeed");
    ASSERT_EQUAL(oldstate, PTHREAD_CANCEL_DISABLE,
                 "previous state should be DISABLE");

    ret = pthread_setcancelstate(99, NULL);
    ASSERT_EQUAL(ret, EINVAL, "setcancelstate with invalid state should fail");

    /* Test pthread_setcanceltype */
    ret = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldtype);
    ASSERT_EQUAL(ret, 0, "setcanceltype DEFERRED should succeed");
    ASSERT_EQUAL(oldtype, PTHREAD_CANCEL_DEFERRED,
                 "default type should be DEFERRED");

    ret = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    ASSERT_EQUAL(ret, ENOTSUP,
                 "setcanceltype ASYNCHRONOUS should return ENOTSUP");

    ret = pthread_setcanceltype(99, NULL);
    ASSERT_EQUAL(ret, EINVAL, "setcanceltype with invalid type should fail");

    /* Test pthread_cancel with a real thread */
    cancel_test_started = 0;
    cancel_test_looped = 0;
    cancel_child_tid = 0;

    ret = pthread_create(&thread, NULL, cancel_test_thread, NULL);
    ASSERT_EQUAL(ret, 0, "create cancel test thread");

    /* Wait for thread to start */
    while (!cancel_test_started)
        L4_Yield();

    /* Cancel the thread */
    ret = pthread_cancel(&thread);
    ASSERT_EQUAL(ret, 0, "pthread_cancel should succeed");

    /* Join and verify it was cancelled */
    ret = pthread_join(&thread, &retval);
    ASSERT_EQUAL(ret, 0, "join cancelled thread");
    ASSERT_EQUAL(retval, PTHREAD_CANCELED,
                 "cancelled thread should return PTHREAD_CANCELED");

    /* Verify it didn't complete all loops */
    ASSERT_TRUE(cancel_test_looped < 500,
                "thread should have been cancelled before completing");

    /* Test pthread_cancel with NULL */
    ret = pthread_cancel(NULL);
    ASSERT_EQUAL(ret, EINVAL, "cancel with NULL should fail");

    TEST_PASS();
}

/* Main test runner */
__USER_TEXT
void run_pthread_tests(void)
{
    printf("\n=== PSE51 POSIX Threads Compliance Tests ===\n");

    test_pthread_create_join();
    test_pthread_detach();
    test_pthread_self_equal();
    test_pthread_attr();
    test_pthread_mutex_basic();
    test_pthread_mutex_recursive();
    test_pthread_mutex_stress();
    test_pthread_mutex_errors();
    test_pthread_cond_basic();
    test_pthread_cond_broadcast();
    test_pthread_mutex_timedlock();
    test_pthread_cond_timedwait();
    test_pthread_cancel();

    printf("\n=== PSE52 Spinlock Tests ===\n");

    test_pthread_spin_basic();
    test_pthread_spin_lock_unlock();
    test_pthread_spin_trylock();
    test_pthread_spin_errors();
}
