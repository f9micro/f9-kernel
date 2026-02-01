/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 * PSE51 Semaphore Compliance Tests
 *
 * Tests sem_init, sem_destroy, sem_wait, sem_post, sem_trywait, sem_getvalue.
 */

#include <l4/ipc.h>
#include <l4io.h>
#include <platform/link.h>
#include <posix/pthread.h>
#include <posix/semaphore.h>
#include <types.h>
#include "posix_tests.h"

/* Test globals - must be in __USER_BSS for user thread access */
__USER_BSS static sem_t test_sem;
__USER_BSS static sem_t trywait_sem; /* Dedicated semaphore for trywait test */
__USER_BSS static sem_t block_sem;   /* Dedicated semaphore for blocking test */
__USER_BSS static int shared_resource;

/* Blocking waiter thread - blocks on semaphore until posted */
__USER_TEXT
void *sem_waiter(void *arg)
{
    (void) arg;
    sem_wait(&block_sem);
    return (void *) 1; /* Indicate successful wakeup */
}

/* Producer thread */
__USER_TEXT
void *producer_thread(void *arg)
{
    int count = *(int *) arg;

    for (int i = 0; i < count; i++) {
        sem_post(&test_sem);
        printf("[PRODUCER] Posted semaphore (iteration %d)\n", i);
    }

    return NULL;
}

/* Consumer thread */
__USER_TEXT
void *consumer_thread(void *arg)
{
    int count = *(int *) arg;

    for (int i = 0; i < count; i++) {
        sem_wait(&test_sem);
        /* Use atomic increment to prevent data race when multiple consumers run
         */
        int new_val = __atomic_add_fetch(&shared_resource, 1, __ATOMIC_SEQ_CST);
        printf("[CONSUMER] Consumed resource (total: %d)\n", new_val);
    }

    return NULL;
}

/* Test 1: sem_init and sem_destroy */
__USER_TEXT
void test_sem_init_destroy(void)
{
    TEST_CASE_START();

    sem_t sem;

    /* Initialize semaphore with count 0 */
    int ret = sem_init(&sem, 0, 0);
    ASSERT_EQUAL(ret, 0, "sem_init should succeed");

    /* Get value */
    int value;
    ret = sem_getvalue(&sem, &value);
    ASSERT_EQUAL(ret, 0, "sem_getvalue should succeed");
    ASSERT_EQUAL(value, 0, "Initial semaphore value should be 0");

    /* Destroy semaphore */
    ret = sem_destroy(&sem);
    ASSERT_EQUAL(ret, 0, "sem_destroy should succeed");

    /* Initialize with non-zero count */
    ret = sem_init(&sem, 0, 5);
    ASSERT_EQUAL(ret, 0, "sem_init with count=5 should succeed");

    ret = sem_getvalue(&sem, &value);
    ASSERT_EQUAL(value, 5, "Initial semaphore value should be 5");

    sem_destroy(&sem);

    TEST_PASS();
}

/* Test 2: sem_post and sem_wait */
__USER_TEXT
void test_sem_post_wait(void)
{
    TEST_CASE_START();

    sem_t sem;
    sem_init(&sem, 0, 0);

    /* Post to semaphore */
    int ret = sem_post(&sem);
    ASSERT_EQUAL(ret, 0, "sem_post should succeed");

    int value;
    sem_getvalue(&sem, &value);
    ASSERT_EQUAL(value, 1, "Semaphore value should be 1 after post");

    /* Wait on semaphore */
    ret = sem_wait(&sem);
    ASSERT_EQUAL(ret, 0, "sem_wait should succeed");

    sem_getvalue(&sem, &value);
    ASSERT_EQUAL(value, 0, "Semaphore value should be 0 after wait");

    sem_destroy(&sem);

    TEST_PASS();
}

/* Test 3: sem_trywait - uses static semaphore to avoid register clobbering
 *
 * Local variables on the stack have addresses computed from frame/stack
 * pointers. These pointers may be held in r4-r11, which are clobbered by L4 IPC
 * (they're global register variables for MR0-MR7). Static variables have fixed
 * addresses computed via PC-relative addressing, avoiding this issue.
 */
__USER_TEXT
void test_sem_trywait(void)
{
    TEST_CASE_START();

    /* Use static semaphore - its address is a link-time constant */
    sem_init(&trywait_sem, 0, 0);

    /* Test 1: trywait on empty should return EAGAIN */
    int ret = sem_trywait(&trywait_sem);
    ASSERT_EQUAL(ret, EAGAIN, "sem_trywait on empty should return EAGAIN");

    /* Post to make semaphore available */
    sem_post(&trywait_sem);

    /* Test 2: trywait should succeed now */
    ret = sem_trywait(&trywait_sem);
    ASSERT_EQUAL(ret, 0, "sem_trywait after post should succeed");

    /* Test 3: should be empty again */
    ret = sem_trywait(&trywait_sem);
    ASSERT_EQUAL(ret, EAGAIN, "sem_trywait after consume should return EAGAIN");

    sem_destroy(&trywait_sem);

    TEST_PASS();
}

/* Test 4: sem_getvalue */
__USER_TEXT
void test_sem_getvalue(void)
{
    TEST_CASE_START();

    sem_t sem;
    sem_init(&sem, 0, 10);

    int value;
    int ret = sem_getvalue(&sem, &value);
    ASSERT_EQUAL(ret, 0, "sem_getvalue should succeed");
    ASSERT_EQUAL(value, 10, "Initial value should be 10");

    /* Wait 3 times */
    sem_wait(&sem);
    sem_wait(&sem);
    sem_wait(&sem);

    sem_getvalue(&sem, &value);
    ASSERT_EQUAL(value, 7, "Value should be 7 after 3 waits");

    /* Post 5 times */
    for (int i = 0; i < 5; i++)
        sem_post(&sem);

    sem_getvalue(&sem, &value);
    ASSERT_EQUAL(value, 12, "Value should be 12 after 5 posts");

    sem_destroy(&sem);

    TEST_PASS();
}

/* Test 5: Producer-Consumer pattern */
__USER_TEXT
void test_sem_producer_consumer(void)
{
    TEST_CASE_START();

    sem_init(&test_sem, 0, 0);
    shared_resource = 0;

    const int ITEM_COUNT = 10;
    int producer_arg = ITEM_COUNT;
    int consumer_arg = ITEM_COUNT;

    pthread_t producer, consumer;

    /* Create producer and consumer threads */
    pthread_create(&producer, NULL, producer_thread, &producer_arg);
    pthread_create(&consumer, NULL, consumer_thread, &consumer_arg);

    /* Wait for both to complete */
    pthread_join(&producer, NULL);
    pthread_join(&consumer, NULL);

    /* Check that all items were consumed */
    ASSERT_EQUAL(shared_resource, ITEM_COUNT,
                 "All produced items should be consumed");

    int value;
    sem_getvalue(&test_sem, &value);
    ASSERT_EQUAL(value, 0, "Semaphore should be empty after consumption");

    sem_destroy(&test_sem);

    TEST_PASS();
}

/* Test 6: Multiple producers and consumers */
__USER_TEXT
void test_sem_multi_producer_consumer(void)
{
    TEST_CASE_START();

    sem_init(&test_sem, 0, 0);
    shared_resource = 0;

    const int NUM_PRODUCERS = 2;
    const int NUM_CONSUMERS = 2;
    const int ITEMS_PER_THREAD = 5;

    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int args[NUM_PRODUCERS > NUM_CONSUMERS ? NUM_PRODUCERS : NUM_CONSUMERS];

    /* Create producers */
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        args[i] = ITEMS_PER_THREAD;
        pthread_create(&producers[i], NULL, producer_thread, &args[i]);
    }

    /* Create consumers */
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        args[i] = ITEMS_PER_THREAD;
        pthread_create(&consumers[i], NULL, consumer_thread, &args[i]);
    }

    /* Join all threads */
    for (int i = 0; i < NUM_PRODUCERS; i++)
        pthread_join(&producers[i], NULL);

    for (int i = 0; i < NUM_CONSUMERS; i++)
        pthread_join(&consumers[i], NULL);

    /* Check result */
    int expected = NUM_CONSUMERS * ITEMS_PER_THREAD;
    ASSERT_EQUAL(shared_resource, expected,
                 "All items should be consumed correctly");

    sem_destroy(&test_sem);

    TEST_PASS();
}

/* Test 7: Blocking sem_wait test - verifies actual blocking semantics
 *
 * This test creates a waiter thread that blocks on a zero-count semaphore.
 * The main thread then posts after a delay. This verifies that:
 * 1. sem_wait actually blocks when count is 0
 * 2. sem_post wakes the blocked thread (tests notification path)
 */
__USER_TEXT
void test_sem_wait_blocks(void)
{
    TEST_CASE_START();

    /* Initialize with count=0 so waiter will block */
    sem_init(&block_sem, 0, 0);

    pthread_t waiter;
    int ret = pthread_create(&waiter, NULL, sem_waiter, NULL);
    ASSERT_EQUAL(ret, 0, "pthread_create for waiter should succeed");

    /* Give waiter time to start and block */
    L4_Sleep(L4_TimePeriod(3000)); /* 3ms */

    /* Verify semaphore is still 0 (waiter is blocked, not consumed) */
    int value;
    sem_getvalue(&block_sem, &value);
    ASSERT_EQUAL(value, 0, "Semaphore should still be 0 (waiter blocked)");

    /* Post to wake waiter */
    sem_post(&block_sem);

    /* Join waiter and check it woke up successfully */
    void *result;
    ret = pthread_join(&waiter, &result);
    ASSERT_EQUAL(ret, 0, "pthread_join should succeed");
    ASSERT_EQUAL((int) result, 1, "waiter should resume after post");

    sem_destroy(&block_sem);

    TEST_PASS();
}

/* Test 8: sem_post overflow handling (POSIX SEM_VALUE_MAX compliance)
 *
 * POSIX requires sem_post to return EOVERFLOW when the semaphore value
 * would exceed SEM_VALUE_MAX. This test posts repeatedly to trigger overflow.
 */
__USER_TEXT
void test_sem_overflow(void)
{
    TEST_CASE_START();

    sem_t sem;
    sem_init(&sem, 0, SEM_VALUE_MAX - 2);

    /* Post twice should succeed */
    int ret = sem_post(&sem);
    ASSERT_EQUAL(ret, 0, "sem_post near max should succeed");
    ret = sem_post(&sem);
    ASSERT_EQUAL(ret, 0, "sem_post at max should succeed");

    /* Verify we're at max */
    int value;
    sem_getvalue(&sem, &value);
    ASSERT_EQUAL(value, SEM_VALUE_MAX, "Value should be SEM_VALUE_MAX");

    /* Next post should fail with EOVERFLOW */
    ret = sem_post(&sem);
    ASSERT_EQUAL(ret, EOVERFLOW, "sem_post beyond max should return EOVERFLOW");

    /* Value should still be at max */
    sem_getvalue(&sem, &value);
    ASSERT_EQUAL(value, SEM_VALUE_MAX, "Value should still be SEM_VALUE_MAX");

    sem_destroy(&sem);

    TEST_PASS();
}

/* Main test runner */
__USER_TEXT
void run_semaphore_tests(void)
{
    printf("\n=== PSE51 Semaphore Compliance Tests ===\n");

    test_sem_init_destroy();
    test_sem_post_wait();
    test_sem_trywait();
    test_sem_getvalue();
    test_sem_wait_blocks();
    test_sem_overflow();
    test_sem_producer_consumer();
    test_sem_multi_producer_consumer();
}
