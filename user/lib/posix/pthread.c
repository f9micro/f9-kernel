/* Copyright (c) 2014,2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/ipc.h>
#include <l4/pager.h>
#include <l4/schedule.h>
#include <l4/thread.h>
#include <l4/utcb.h>
#include <l4io.h>
#include <platform/link.h>
#include <posix/pthread.h>
#include <posix/sched.h>
#include __L4_INC_ARCH(syscalls.h)

/* Signal cleanup function - defined in signal.c */
extern void __signal_thread_cleanup(L4_ThreadId_t tid);

/* Cancel cleanup function - defined later in this file */
void __cancel_thread_cleanup(pthread_t *thread);

/* Thread lifecycle states */
#define PTHREAD_STATE_CREATED 0
#define PTHREAD_STATE_RUNNING 1
#define PTHREAD_STATE_EXITED 2

/*
 * Shared spinlock helpers for waiter list serialization
 *
 * Uses Test-and-Test-and-Set (TTAS) pattern for reduced bus traffic:
 * 1. Plain load to check if free (cache-friendly, no exclusive marking)
 * 2. Only if free, attempt LDREX/STREX sequence
 *
 * On single-core Cortex-M this is a minor optimization, but cleaner
 * and beneficial on multi-core platforms.
 */

/* Spinlock acquire using TTAS pattern with LDREX/STREX */
__USER_TEXT
static void spinlock_acquire(uint32_t *lock)
{
    uint32_t status;
    while (1) {
        /* Test: Spin on plain load until lock appears free.
         * Plain LDR is cache-friendly and doesn't mark exclusive. */
        while (*(volatile uint32_t *) lock != 0)
            ; /* Spin - on single-core this yields naturally at tick */

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
        /* STREX failed or lock was taken between test and TAS - retry */
    }
}

/* Spinlock release */
__USER_TEXT
static void spinlock_release(uint32_t *lock)
{
    __asm__ __volatile__("dmb" ::: "memory"); /* Release barrier */
    *lock = 0;
}

/* Thread attribute functions */
__USER_TEXT
int pthread_attr_init(pthread_attr_t *attr)
{
    if (!attr)
        return EINVAL;

    attr->priority = 128;   /* Default priority */
    attr->stack_size = 512; /* Default stack (512 bytes) */
    attr->detachstate = PTHREAD_CREATE_JOINABLE;

    return 0;
}

__USER_TEXT
int pthread_attr_destroy(pthread_attr_t *attr)
{
    if (!attr)
        return EINVAL;
    return 0;
}

__USER_TEXT
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
    if (!attr || (detachstate != PTHREAD_CREATE_DETACHED &&
                  detachstate != PTHREAD_CREATE_JOINABLE))
        return EINVAL;

    attr->detachstate = detachstate;
    return 0;
}

__USER_TEXT
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
    if (!attr || !detachstate)
        return EINVAL;

    *detachstate = attr->detachstate;
    return 0;
}

__USER_TEXT
int pthread_attr_setstacksize(pthread_attr_t *attr, uint32_t stacksize)
{
    if (!attr || stacksize < 256)
        return EINVAL;

    attr->stack_size = stacksize;
    return 0;
}

__USER_TEXT
int pthread_attr_getstacksize(const pthread_attr_t *attr, uint32_t *stacksize)
{
    if (!attr || !stacksize)
        return EINVAL;

    *stacksize = attr->stack_size;
    return 0;
}

/* Forward declarations for thread_ptr table functions */
static int register_thread_ptr(pthread_t *thread);
static pthread_t *get_current_thread_ptr(void);
static void unregister_thread_ptr(void);
static void *get_utcb_for_thread(pthread_t *thread);

/* Pending early cancel table.
 * Tracks pthread_t* that were cancelled before the child registered its TID.
 * When the child registers, it checks this table and applies pending cancels.
 */
#define PENDING_CANCEL_SIZE 8
__USER_BSS static pthread_t *pending_cancel[PENDING_CANCEL_SIZE];
__USER_BSS static uint32_t pending_cancel_lock;

/* Global spinlock for mutex lazy initialization (PTHREAD_MUTEX_INITIALIZER).
 * Protects check-then-initialize to prevent race between concurrent threads.
 */
__USER_BSS static uint32_t mutex_lazy_init_lock;

/* Add a pending early cancel for a pthread_t* (when child hasn't registered).
 * Returns 0 on success, EAGAIN if table is full.
 */
__USER_TEXT
static int add_pending_cancel(pthread_t *thread)
{
    int result = EAGAIN; /* Assume full until we find a slot */
    spinlock_acquire(&pending_cancel_lock);
    for (int i = 0; i < PENDING_CANCEL_SIZE; i++) {
        if ((uintptr_t) pending_cancel[i] == 0) {
            pending_cancel[i] = thread;
            result = 0;
            break;
        }
    }
    spinlock_release(&pending_cancel_lock);
    return result;
}

/* Check and consume pending early cancel for a pthread_t* */
__USER_TEXT
static int consume_pending_cancel(pthread_t *thread)
{
    int found = 0;
    spinlock_acquire(&pending_cancel_lock);
    for (int i = 0; i < PENDING_CANCEL_SIZE; i++) {
        if (pending_cancel[i] == thread) {
            pending_cancel[i] = (pthread_t *) 0;
            found = 1;
            break;
        }
    }
    spinlock_release(&pending_cancel_lock);
    return found;
}

/* Forward declaration for set_cancel_pending (defined later) */
static int set_cancel_pending(pthread_t *thread);

/* Thread wrapper function.
 * This wrapper receives pthread_t* as arg, registers the actual global ID
 * in a translation table, then calls the user's entry function.
 */
__USER_TEXT
static void *pthread_entry_wrapper(void *wrapper_arg)
{
    pthread_t *thread = (pthread_t *) wrapper_arg;

    /* Register this thread's pthread_t* using UTCB address as key.
     * This allows other code to find our pthread_t* without relying
     * on the problematic t_globalid (which reads as 0 initially).
     */
    register_thread_ptr(thread);
    thread->state = PTHREAD_STATE_RUNNING;

    /* Check for early cancel: if pthread_cancel was called before we
     * registered, consume the pending cancel and apply it now.
     */
    if (consume_pending_cancel(thread)) {
        set_cancel_pending(thread);
    }

    /* Call the actual user function */
    void *result = thread->entry(thread->arg);

    /* Clean up cancel state before unregistering to prevent table leaks */
    __cancel_thread_cleanup(thread);

    /* Unregister on normal return */
    unregister_thread_ptr();

    return result;
}

/* Thread management functions */
__USER_TEXT
int pthread_create(pthread_t *thread,
                   const pthread_attr_t *attr,
                   void *(*start_routine)(void *),
                   void *arg)
{
    if (!thread || !start_routine)
        return EINVAL;

    /* Create L4 thread via pager. */
    L4_ThreadId_t tid = pager_create_thread();
    if (tid.raw == 0)
        return EAGAIN;

    /* Initialize pthread structure.
     * Store entry and arg for the wrapper to use.
     * tid will be updated by the child to its actual global ID.
     */
    thread->tid = tid; /* Placeholder, updated by child */
    thread->entry = start_routine;
    thread->arg = arg;
    thread->retval = NULL;
    thread->detached = attr ? attr->detachstate : PTHREAD_CREATE_JOINABLE;
    thread->joined = 0;
    thread->state = PTHREAD_STATE_CREATED;
    thread->creator.raw = 0; /* Will be set by child */

    /* Start thread execution via wrapper that updates tid.
     * Pass pthread_t* as arg to the wrapper.
     */
    pager_start_thread(tid, pthread_entry_wrapper, thread);

    return 0;
}

__USER_TEXT
void pthread_exit(void *retval)
{
    /* Get current thread's pthread_t* for cleanup */
    pthread_t *self = get_current_thread_ptr();

    if (self) {
        self->state = PTHREAD_STATE_EXITED;
    }

    /* Clean up per-thread signal state to recycle slot.
     * Note: signal cleanup still uses TID, but we try to get it safely.
     */
    L4_ThreadId_t tid = L4_MyGlobalId();
    __signal_thread_cleanup(tid);

    /* Clean up per-thread cancellation state to recycle slot */
    if (self) {
        __cancel_thread_cleanup(self);
    }

    /* Unregister thread_ptr table entry */
    unregister_thread_ptr();

    /* Send exit notification to pager using same protocol as thread_container.
     * This ensures joiners are woken and thread node is properly released.
     */
    L4_Msg_t msg;
    L4_MsgClear(&msg);
    L4_Set_Label(&msg.tag, PAGER_REQUEST_LABEL);
    L4_MsgAppendWord(&msg, THREAD_FREE);
    L4_MsgAppendWord(&msg, (L4_Word_t) retval);
    L4_MsgLoad(&msg);
    L4_Send(L4_Pager());

    /* Thread will be terminated by pager - halt forever */
    while (1)
        L4_Sleep(L4_Never);
}

__USER_TEXT
int pthread_join(pthread_t *thread, void **retval)
{
    if (!thread)
        return EINVAL;

    if (thread->detached)
        return EINVAL;

    if (thread->joined)
        return EINVAL;

    /* Block until thread exits using IPC-based synchronization.
     * This is RTOS-safe: no spinning, no arbitrary delays.
     * The pager will wake us when the target thread calls THREAD_FREE.
     */
    int ret = pager_thread_join(thread->tid, retval);

    /* Only mark as joined on success, allowing retry on failure */
    if (ret == 0)
        thread->joined = 1;

    return ret;
}

__USER_TEXT
int pthread_detach(pthread_t *thread)
{
    if (!thread)
        return EINVAL;

    if (thread->detached)
        return EINVAL;

    thread->detached = 1;

    /* Notify pager so it can release thread node on exit */
    pager_thread_detach(thread->tid);

    return 0;
}

__USER_TEXT
pthread_t pthread_self(void)
{
    pthread_t self;
    self.tid = L4_MyGlobalId();
    self.creator = L4_nilthread;
    self.entry = NULL;
    self.arg = NULL;
    self.retval = NULL;
    self.detached = 0;
    self.joined = 0;
    self.state = PTHREAD_STATE_RUNNING;
    return self;
}

__USER_TEXT
int pthread_equal(pthread_t t1, pthread_t t2)
{
    return (t1.tid.raw == t2.tid.raw);
}

/* ============================================================
 * Thread Cancellation (PSE51 POSIX_THREADS_BASE)
 *
 * Only deferred cancellation is supported. Asynchronous cancellation
 * is hazardous in real-time systems (can leave locks held, data
 * structures inconsistent) and is explicitly not implemented.
 *
 * Cancellation points: pthread_testcancel(), pthread_cond_wait(),
 * pthread_cond_timedwait(), sem_wait(), nanosleep().
 * ============================================================ */

/* Maximum threads for per-thread cancellation state */
#define CANCEL_MAX_THREADS 16

/* Per-thread cancellation state structure
 * Uses UTCB address as key for lookup. This works for:
 * - Child threads created via pthread_create (have UTCB registered)
 * - Main/root thread (has valid current_utcb but no pthread_t*)
 */
struct thread_cancel_state {
    void *utcb_addr;        /* UTCB address for matching */
    pthread_t *thread_ptr;  /* Optional pthread_t* (for cleanup) */
    uint8_t cancel_state;   /* PTHREAD_CANCEL_ENABLE/DISABLE */
    uint8_t cancel_type;    /* PTHREAD_CANCEL_DEFERRED only */
    uint8_t cancel_pending; /* Cancellation requested */
};

/* Per-thread cancellation state table */
__USER_BSS static struct thread_cancel_state cancel_table[CANCEL_MAX_THREADS];

/* Spinlock protecting cancel table */
__USER_BSS static uint32_t cancel_table_lock;

/* Per-thread pthread_t pointer table (indexed by UTCB address for uniqueness).
 * This allows threads to find their pthread_t* without relying on t_globalid.
 */
#define THREAD_PTR_TABLE_SIZE 16
struct thread_ptr_entry {
    void *utcb_addr;       /* UTCB address as unique key */
    pthread_t *thread_ptr; /* Associated pthread_t* */
};
__USER_BSS static struct thread_ptr_entry
    thread_ptr_table[THREAD_PTR_TABLE_SIZE];
__USER_BSS static uint32_t thread_ptr_lock;

/* Register pthread_t* by UTCB address (called by pthread_entry_wrapper).
 * Returns 0 on success, -1 if thread_ptr_table is full.
 * When table is full, pthread_cancel and get_current_thread_ptr will fail.
 */
__USER_TEXT
static int register_thread_ptr(pthread_t *thread)
{
    extern void *current_utcb;
    int result = -1; /* Assume full until we find a slot */

    spinlock_acquire(&thread_ptr_lock);
    for (int i = 0; i < THREAD_PTR_TABLE_SIZE; i++) {
        if (thread_ptr_table[i].utcb_addr == (void *) 0) {
            thread_ptr_table[i].utcb_addr = current_utcb;
            thread_ptr_table[i].thread_ptr = thread;
            result = 0;
            break;
        }
    }
    spinlock_release(&thread_ptr_lock);

    if (result < 0) {
        /* Table full - log warning. Thread will still run but cancel won't
         * work.
         */
        printf(
            "[POSIX] WARNING: thread_ptr_table full, pthread_cancel "
            "disabled\n");
    }
    return result;
}

/* Get current thread's pthread_t* (uses UTCB address as key) */
__USER_TEXT
static pthread_t *get_current_thread_ptr(void)
{
    extern void *current_utcb;
    pthread_t *result = NULL;
    spinlock_acquire(&thread_ptr_lock);
    for (int i = 0; i < THREAD_PTR_TABLE_SIZE; i++) {
        if (thread_ptr_table[i].utcb_addr == current_utcb) {
            result = thread_ptr_table[i].thread_ptr;
            break;
        }
    }
    spinlock_release(&thread_ptr_lock);
    return result;
}

/* Unregister pthread_t* (called on thread exit) */
__USER_TEXT
static void unregister_thread_ptr(void)
{
    extern void *current_utcb;
    spinlock_acquire(&thread_ptr_lock);
    for (int i = 0; i < THREAD_PTR_TABLE_SIZE; i++) {
        if (thread_ptr_table[i].utcb_addr == current_utcb) {
            thread_ptr_table[i].utcb_addr = NULL;
            thread_ptr_table[i].thread_ptr = NULL;
            break;
        }
    }
    spinlock_release(&thread_ptr_lock);
}

/* Get UTCB address for a pthread_t* (reverse lookup in thread_ptr_table) */
__USER_TEXT
static void *get_utcb_for_thread(pthread_t *thread)
{
    void *utcb = NULL;
    spinlock_acquire(&thread_ptr_lock);
    for (int i = 0; i < THREAD_PTR_TABLE_SIZE; i++) {
        if (thread_ptr_table[i].thread_ptr == thread) {
            utcb = thread_ptr_table[i].utcb_addr;
            break;
        }
    }
    spinlock_release(&thread_ptr_lock);
    return utcb;
}

/* Find or create cancellation state for current thread.
 * Uses UTCB address as key, works for both child threads and main thread.
 */
__USER_TEXT
static struct thread_cancel_state *get_cancel_state(void)
{
    extern void *current_utcb;
    void *my_utcb = current_utcb;

    if (!my_utcb)
        return NULL;

    int free_slot = -1;

    spinlock_acquire(&cancel_table_lock);

    for (int i = 0; i < CANCEL_MAX_THREADS; i++) {
        if (cancel_table[i].utcb_addr == my_utcb) {
            spinlock_release(&cancel_table_lock);
            return &cancel_table[i];
        }
        if (free_slot < 0 && cancel_table[i].utcb_addr == (void *) 0)
            free_slot = i;
    }

    /* Create new entry with defaults */
    if (free_slot >= 0) {
        cancel_table[free_slot].utcb_addr = my_utcb;
        cancel_table[free_slot].thread_ptr = get_current_thread_ptr();
        cancel_table[free_slot].cancel_state = PTHREAD_CANCEL_ENABLE;
        cancel_table[free_slot].cancel_type = PTHREAD_CANCEL_DEFERRED;
        cancel_table[free_slot].cancel_pending = 0;
        spinlock_release(&cancel_table_lock);
        return &cancel_table[free_slot];
    }

    spinlock_release(&cancel_table_lock);
    return NULL; /* Table full */
}

/* Set pending cancellation for a thread (called by pthread_cancel).
 * Uses UTCB address as key.
 * Returns:
 *   0      - Success
 *   ESRCH  - Thread not registered yet (valid for early cancel)
 *   EAGAIN - Cancel table full (resource exhaustion)
 */
__USER_TEXT
static int set_cancel_pending(pthread_t *thread)
{
    /* Look up target thread's UTCB address */
    void *target_utcb = get_utcb_for_thread(thread);
    if (!target_utcb)
        return ESRCH; /* Thread not registered yet - early cancel case */

    int free_slot = -1;

    spinlock_acquire(&cancel_table_lock);

    for (int i = 0; i < CANCEL_MAX_THREADS; i++) {
        if (cancel_table[i].utcb_addr == target_utcb) {
            /* Use atomic store for visibility to target thread */
            __atomic_store_n(&cancel_table[i].cancel_pending, 1,
                             __ATOMIC_RELEASE);
            spinlock_release(&cancel_table_lock);
            return 0;
        }
        if (free_slot < 0 && cancel_table[i].utcb_addr == (void *) 0)
            free_slot = i;
    }

    /* Create entry for thread that hasn't initialized cancel state */
    if (free_slot >= 0) {
        cancel_table[free_slot].utcb_addr = target_utcb;
        cancel_table[free_slot].thread_ptr = thread;
        cancel_table[free_slot].cancel_state = PTHREAD_CANCEL_ENABLE;
        cancel_table[free_slot].cancel_type = PTHREAD_CANCEL_DEFERRED;
        __atomic_store_n(&cancel_table[free_slot].cancel_pending, 1,
                         __ATOMIC_RELEASE);
        spinlock_release(&cancel_table_lock);
        return 0;
    }

    spinlock_release(&cancel_table_lock);
    return EAGAIN; /* Table full */
}

/* Clean up cancellation state when thread exits.
 * Uses UTCB address lookup from thread_ptr to find the entry.
 */
__USER_TEXT
void __cancel_thread_cleanup(pthread_t *thread)
{
    void *target_utcb = get_utcb_for_thread(thread);

    spinlock_acquire(&cancel_table_lock);
    for (int i = 0; i < CANCEL_MAX_THREADS; i++) {
        /* Match by UTCB if available, or by thread_ptr as fallback */
        if ((target_utcb && cancel_table[i].utcb_addr == target_utcb) ||
            (!target_utcb && cancel_table[i].thread_ptr == thread)) {
            cancel_table[i].utcb_addr = NULL;
            cancel_table[i].thread_ptr = NULL;
            cancel_table[i].cancel_state = 0;
            cancel_table[i].cancel_type = 0;
            cancel_table[i].cancel_pending = 0;
            break;
        }
    }
    spinlock_release(&cancel_table_lock);
}

__USER_TEXT
int pthread_cancel(pthread_t *thread)
{
    if (!thread)
        return EINVAL;

    /* Check if thread already exited */
    if (thread->joined || thread->state == PTHREAD_STATE_EXITED) {
        return ESRCH;
    }

    /* Try to set cancel pending directly using pthread_t*.
     * If thread hasn't registered yet (ESRCH), queue for early cancel.
     * If cancel table is full (EAGAIN), report the error.
     */
    int result = set_cancel_pending(thread);
    if (result == ESRCH) {
        /* Thread not registered yet - queue for early cancel.
         * The wrapper will apply it when the thread starts.
         */
        if (thread->state == PTHREAD_STATE_CREATED) {
            return add_pending_cancel(thread);
        }
        return ESRCH;
    }
    return result;
}

__USER_TEXT
int pthread_setcancelstate(int state, int *oldstate)
{
    if (state != PTHREAD_CANCEL_ENABLE && state != PTHREAD_CANCEL_DISABLE)
        return EINVAL;

    struct thread_cancel_state *cs = get_cancel_state();
    if (!cs)
        return EAGAIN;

    if (oldstate)
        *oldstate = cs->cancel_state;

    cs->cancel_state = state;
    return 0;
}

__USER_TEXT
int pthread_setcanceltype(int type, int *oldtype)
{
    /* Only deferred cancellation is supported.
     * Asynchronous cancellation is hazardous in RT systems.
     */
    if (type != PTHREAD_CANCEL_DEFERRED) {
        if (type == PTHREAD_CANCEL_ASYNCHRONOUS)
            return ENOTSUP; /* Explicitly not supported */
        return EINVAL;
    }

    struct thread_cancel_state *cs = get_cancel_state();
    if (!cs)
        return EAGAIN;

    if (oldtype)
        *oldtype = cs->cancel_type;

    cs->cancel_type = type;
    return 0;
}

__USER_TEXT
void pthread_testcancel(void)
{
    struct thread_cancel_state *cs = get_cancel_state();
    if (!cs)
        return;

    /* Check if cancellation is pending and enabled.
     * Use atomic load for visibility from pthread_cancel in other thread.
     */
    if (__atomic_load_n(&cs->cancel_pending, __ATOMIC_ACQUIRE) &&
        cs->cancel_state == PTHREAD_CANCEL_ENABLE) {
        /* Clean up cancel state before exiting */
        __cancel_thread_cleanup(cs->thread_ptr);
        pthread_exit(PTHREAD_CANCELED);
    }
}

/* Mutex attribute functions */
__USER_TEXT
int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    if (!attr)
        return EINVAL;

    attr->type = PTHREAD_MUTEX_NORMAL;
    attr->initialized = 1;
    return 0;
}

__USER_TEXT
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    if (!attr || !attr->initialized)
        return EINVAL;
    attr->initialized = 0;
    return 0;
}

__USER_TEXT
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    if (!attr || !attr->initialized)
        return EINVAL;
    if (type != PTHREAD_MUTEX_NORMAL && type != PTHREAD_MUTEX_RECURSIVE)
        return EINVAL;

    attr->type = type;
    return 0;
}

__USER_TEXT
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    if (!attr || !type || !attr->initialized)
        return EINVAL;

    *type = attr->type;
    return 0;
}

/* Mutex management functions */

/* Mutex waiter list helpers (spinlock-protected) */

/* Add current thread to mutex waiter list. Returns 0 on success, -1 if full. */
__USER_TEXT
static int mutex_waiter_add(pthread_mutex_t *m, L4_ThreadId_t tid)
{
    spinlock_acquire(&m->waiters_lock);
    if (m->num_waiters >= MUTEX_MAX_WAITERS) {
        spinlock_release(&m->waiters_lock);
        return -1; /* List full */
    }
    m->waiters[m->num_waiters] = tid;
    m->num_waiters++;
    spinlock_release(&m->waiters_lock);
    return 0;
}

/* Remove a specific thread from mutex waiter list. */
__USER_TEXT
static void mutex_waiter_remove(pthread_mutex_t *m, L4_ThreadId_t tid)
{
    spinlock_acquire(&m->waiters_lock);
    for (uint32_t i = 0; i < m->num_waiters; i++) {
        if (m->waiters[i].raw == tid.raw) {
            /* Shift remaining entries down */
            for (uint32_t j = i; j < m->num_waiters - 1; j++)
                m->waiters[j] = m->waiters[j + 1];
            m->num_waiters--;
            break;
        }
    }
    spinlock_release(&m->waiters_lock);
}

/* Pop first waiter from mutex list. Returns nilthread if list empty. */
__USER_TEXT
static L4_ThreadId_t mutex_waiter_pop(pthread_mutex_t *m)
{
    L4_ThreadId_t tid = L4_nilthread;
    spinlock_acquire(&m->waiters_lock);
    if (m->num_waiters > 0) {
        tid = m->waiters[0];
        /* Shift remaining entries down */
        for (uint32_t i = 0; i < m->num_waiters - 1; i++)
            m->waiters[i] = m->waiters[i + 1];
        m->num_waiters--;
    }
    spinlock_release(&m->waiters_lock);
    return tid;
}

/* Internal: Initialize mutex if using static initializer.
 * Uses global spinlock to prevent race between concurrent lazy inits.
 */
__USER_TEXT
static int pthread_mutex_lazy_init(pthread_mutex_t *mutex)
{
    /* Fast path: already initialized */
    if (mutex->initialized)
        return 0;

    /* Slow path: check sentinel and initialize under lock.
     * Global spinlock prevents TOCTOU race between check and init.
     */
    spinlock_acquire(&mutex_lazy_init_lock);

    /* Double-check after acquiring lock (another thread may have initialized)
     */
    if (mutex->count == PTHREAD_MUTEX_INITIALIZER_MAGIC &&
        !mutex->initialized) {
        mutex->lock = 0;
        mutex->owner.raw = 0;
        mutex->type = PTHREAD_MUTEX_NORMAL;
        mutex->count = 0;
        mutex->waiters_lock = 0;
        mutex->num_waiters = 0;
        for (int i = 0; i < MUTEX_MAX_WAITERS; i++)
            mutex->waiters[i] = L4_nilthread;
        /* Memory barrier before setting initialized flag */
        __atomic_thread_fence(__ATOMIC_RELEASE);
        mutex->initialized = 1;
    }

    spinlock_release(&mutex_lazy_init_lock);
    return mutex->initialized ? 0 : EINVAL;
}

__USER_TEXT
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    if (!mutex)
        return EINVAL;

    /* Validate attr if provided */
    if (attr && !attr->initialized)
        return EINVAL;

    mutex->lock = 0;
    mutex->owner.raw = 0;
    mutex->type = attr ? attr->type : PTHREAD_MUTEX_NORMAL;
    mutex->count = 0;
    mutex->waiters_lock = 0;
    mutex->num_waiters = 0;
    for (int i = 0; i < MUTEX_MAX_WAITERS; i++)
        mutex->waiters[i] = L4_nilthread;
    mutex->initialized = 1;

    return 0;
}

__USER_TEXT
int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    if (!mutex)
        return EINVAL;

    /* Check if initialized (explicit or via lazy init) */
    if (!mutex->initialized)
        return EINVAL;

    if (mutex->lock)
        return EBUSY;

    mutex->initialized = 0;
    return 0;
}

__USER_TEXT
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    if (!mutex)
        return EINVAL;

    /* Support PTHREAD_MUTEX_INITIALIZER lazy initialization */
    if (!mutex->initialized) {
        int ret = pthread_mutex_lazy_init(mutex);
        if (ret != 0)
            return ret;
    }

    L4_ThreadId_t self = L4_MyGlobalId();

    /* Recursive mutex: allow re-locking by same thread */
    if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
        if (mutex->owner.raw == self.raw) {
            mutex->count++;
            return 0;
        }
    } else {
        /* Deadlock detection for normal mutex */
        if (mutex->owner.raw == self.raw)
            return EDEADLK;
    }

    /* Blocking mutex acquisition using direct notifications.
     *
     * Fast path: Try atomic acquisition for uncontended case.
     * Slow path: Register in waiter list and block via L4_NotifyWait.
     *
     * The retry loop handles:
     * 1. Lost wakeups (mutex_unlock before we started waiting)
     * 2. Spurious wakeups
     * 3. Waiter list full (yield and retry)
     */
    while (1) {
        /* Fast path: try to acquire with atomic operation */
        if (pthread_mutex_trylock(mutex) == 0)
            return 0;

        /* Slow path: register as waiter and block */
        if (mutex_waiter_add(mutex, self) < 0) {
            /* Waiter list full - yield and retry */
            L4_Yield();
            continue;
        }

        /* Double-check before blocking (unlock may have raced) */
        if (pthread_mutex_trylock(mutex) == 0) {
            /* Success! Remove ourselves from waiter list */
            mutex_waiter_remove(mutex, self);
            return 0;
        }

        /* Block waiting for notification */
        L4_NotifyWait(POSIX_NOTIFY_MUTEX_BIT);

        /* Woken up - remove self from list and retry */
        mutex_waiter_remove(mutex, self);

        /* Retry trylock to handle:
         * 1. Normal wakeup from mutex_unlock
         * 2. Spurious wakeup
         * The loop continues until we successfully acquire
         */
    }
}

__USER_TEXT
int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    if (!mutex)
        return EINVAL;

    /* Support PTHREAD_MUTEX_INITIALIZER lazy initialization */
    if (!mutex->initialized) {
        int ret = pthread_mutex_lazy_init(mutex);
        if (ret != 0)
            return ret;
    }

    register int result = 1;
    L4_ThreadId_t self = L4_MyGlobalId();

    /* Recursive mutex: allow re-locking */
    if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
        if (mutex->owner.raw == self.raw) {
            mutex->count++;
            return 0;
        }
    }

    /* ARM LDREX/STREX atomic operation with acquire barrier.
     * Use ITE (If-Then-Else) to always write result:
     * - If lock==0: STREX stores 1, r0 gets STREX result (0=success, 1=fail)
     * - If lock!=0: r0 gets 1 (failure, mutex was locked)
     *
     * DMB ISH after successful lock provides acquire semantics:
     * critical section memory accesses won't be reordered before lock.
     */
    __asm__ __volatile__(
        "mov r1, #1\n"
        "mov r2, %[mutex]\n"
        "ldrex r0, [r2]\n"       /* Load exclusive: r0 = lock value */
        "cmp r0, #0\n"           /* Check if unlocked */
        "ite eq\n"               /* If-Then-Else block */
        "strexeq r0, r1, [r2]\n" /* Then: store 1 if unlocked, r0=STREX result
                                  */
        "movne r0, #1\n"         /* Else: set r0=1 (already locked) */
        "cmp r0, #0\n"           /* Check if we acquired the lock */
        "it eq\n"
        "dmbeq\n"             /* Acquire barrier on success */
        "mov %[result], r0\n" /* Always store result */
        : [result] "=r"(result)
        : [mutex] "r"(&mutex->lock)
        : "r0", "r1", "r2", "cc", "memory");

    if (result == 0) {
        mutex->owner = self;
        mutex->count = 1;
    }

    return result ? EBUSY : 0;
}

__USER_TEXT
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    if (!mutex)
        return EINVAL;

    /* Must be initialized to unlock */
    if (!mutex->initialized)
        return EINVAL;

    L4_ThreadId_t self = L4_MyGlobalId();

    /* Check ownership */
    if (mutex->owner.raw != self.raw)
        return EPERM;

    /* Recursive mutex: decrement count */
    if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
        if (mutex->count > 1) {
            mutex->count--;
            return 0;
        }
    }

    /* Release lock with release barrier.
     * DMB ISH ensures all critical section memory accesses complete
     * before the lock becomes visible as released.
     */
    mutex->count = 0;
    mutex->owner.raw = 0;
    __asm__ __volatile__("dmb" ::: "memory");
    mutex->lock = 0;

    /* Wake one waiting thread via direct notification */
    L4_ThreadId_t waiter = mutex_waiter_pop(mutex);
    if (waiter.raw != L4_nilthread.raw) {
        L4_NotifyPost(waiter, POSIX_NOTIFY_MUTEX_BIT);
    }

    return 0;
}

/*
 * Timed wait helpers
 *
 * Tick rate: STM32F4 @ 168MHz with KTIMER_HEARTBEAT=65536:
 *   168000000 / 65536 ≈ 2563 ticks/sec ≈ 390 µs/tick
 *
 * For portability, we use 2500 ticks/sec (400 µs/tick).
 * All calculations use 32-bit arithmetic to avoid libgcc dependencies.
 */
#define POSIX_TICKS_PER_SEC 2500U
#define POSIX_USEC_PER_TICK 400U /* 1000000 / 2500 = 400 µs/tick */

/* Convert timespec to microseconds (32-bit arithmetic, no libgcc). */
__USER_TEXT
static uint64_t timespec_to_us(const struct timespec *ts)
{
    if (!ts)
        return 0;

    /* Clamp seconds to reasonable range */
    uint32_t secs = (ts->tv_sec > 4000000) ? 4000000 : (uint32_t) ts->tv_sec;

    /* Convert to microseconds */
    uint64_t usec = (uint64_t) secs * 1000000UL;

    /* Add nanoseconds converted to microseconds */
    uint32_t nsec = (uint32_t) ts->tv_nsec;
    usec += nsec / 1000;

    return usec;
}

/* Convert absolute timespec deadline to relative ticks.
 * POSIX specifies abstime as an absolute deadline (e.g., from clock_gettime).
 * We compute: relative_timeout = abstime - now
 * Returns 0 if deadline has already passed.
 */
__USER_TEXT
static L4_Word_t abstime_to_relative_ticks(const struct timespec *abstime)
{
    if (!abstime)
        return 0;

    /* Get current time in microseconds */
    L4_Clock_t now = L4_SystemClock();
    uint64_t now_us = now.raw;

    /* Convert abstime to microseconds */
    uint64_t deadline_us = timespec_to_us(abstime);

    /* If deadline has passed, return 0 (immediate timeout) */
    if (deadline_us <= now_us)
        return 0;

    /* Compute relative timeout in microseconds */
    uint64_t relative_us = deadline_us - now_us;

    /* Convert to ticks (400 µs per tick) */
    L4_Word_t ticks = (L4_Word_t) (relative_us / POSIX_USEC_PER_TICK);

    /* Ensure at least 1 tick for non-zero timeout */
    if (ticks == 0 && relative_us > 0)
        ticks = 1;

    return ticks;
}

__USER_TEXT
int pthread_mutex_timedlock(pthread_mutex_t *mutex,
                            const struct timespec *abstime)
{
    if (!mutex)
        return EINVAL;

    if (!abstime)
        return EINVAL;

    /* Support PTHREAD_MUTEX_INITIALIZER lazy initialization */
    if (!mutex->initialized) {
        int ret = pthread_mutex_lazy_init(mutex);
        if (ret != 0)
            return ret;
    }

    L4_ThreadId_t self = L4_MyGlobalId();

    /* Recursive mutex: allow re-locking by same thread */
    if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
        if (mutex->owner.raw == self.raw) {
            mutex->count++;
            return 0;
        }
    } else {
        /* Deadlock detection for normal mutex */
        if (mutex->owner.raw == self.raw)
            return EDEADLK;
    }

    /* Fast path: try immediate acquisition */
    if (pthread_mutex_trylock(mutex) == 0) {
        L4_NotifyClear(POSIX_NOTIFY_TIMEOUT_BIT);
        return 0;
    }

    /* Convert absolute deadline to relative ticks.
     * POSIX: abstime is an absolute deadline, not a duration.
     */
    L4_Word_t timeout_ticks = abstime_to_relative_ticks(abstime);
    if (timeout_ticks == 0)
        return ETIMEDOUT; /* Deadline passed = immediate fail */

    /* Slow path: timed wait using notification-based blocking
     *
     * Pattern:
     * 1. Register in waiter list
     * 2. Schedule timeout timer with L4_TimerNotify
     * 3. Wait on (MUTEX_BIT | TIMEOUT_BIT)
     * 4. Check which bit woke us:
     *    - MUTEX_BIT: try to acquire
     *    - TIMEOUT_BIT: return ETIMEDOUT
     * 5. Handle spurious wakes and retries
     */
    while (timeout_ticks > 0) {
        /* Clear any stale timeout bits from previous iterations.
         * A timer from a previous iteration may have fired late;
         * without clearing, we'd see the stale bit immediately.
         */
        L4_NotifyClear(POSIX_NOTIFY_TIMEOUT_BIT);

        /* Register as waiter */
        if (mutex_waiter_add(mutex, self) < 0) {
            /* Waiter list full - yield and retry */
            L4_Yield();
            continue;
        }

        /* Double-check before blocking */
        if (pthread_mutex_trylock(mutex) == 0) {
            mutex_waiter_remove(mutex, self);
            return 0;
        }

        /* Schedule one-shot timeout timer */
        L4_Word_t timer =
            L4_TimerNotify(timeout_ticks, POSIX_NOTIFY_TIMEOUT_BIT, 0);
        if (timer == 0) {
            /* Timer creation failed (pool exhausted) - treat as timeout */
            mutex_waiter_remove(mutex, self);
            return ETIMEDOUT;
        }

        /* Block waiting for mutex notification or timeout */
        L4_Word_t bits =
            L4_NotifyWait(POSIX_NOTIFY_MUTEX_BIT | POSIX_NOTIFY_TIMEOUT_BIT);

        /* Remove from waiter list */
        mutex_waiter_remove(mutex, self);

        /* Check for timeout */
        if (bits & POSIX_NOTIFY_TIMEOUT_BIT) {
            /* Try one more time in case of race */
            if (pthread_mutex_trylock(mutex) == 0) {
                /* Clear timeout bit before returning */
                L4_NotifyClear(POSIX_NOTIFY_TIMEOUT_BIT);
                return 0;
            }
            return ETIMEDOUT;
        }

        /* Woken by mutex notification - try to acquire */
        if (pthread_mutex_trylock(mutex) == 0) {
            /* Clear any pending timeout bit to prevent stale notifications.
             * The timer may still fire later; without clearing, the next
             * timed wait would see the stale TIMEOUT_BIT and return
             * immediately.
             */
            L4_NotifyClear(POSIX_NOTIFY_TIMEOUT_BIT);
            return 0;
        }

        /* Spurious wake or lost race - recompute remaining timeout from
         * deadline */
        timeout_ticks = abstime_to_relative_ticks(abstime);
    }

    return ETIMEDOUT;
}

/* Condition variable implementation - POSIX_THREADS_BASE mandatory */

/* Condition variable waiter list helpers (spinlock-protected) */

/* Add current thread to condvar waiter list. Returns 0 on success, -1 if full.
 */
__USER_TEXT
static int cond_waiter_add(pthread_cond_t *c, L4_ThreadId_t tid)
{
    spinlock_acquire(&c->waiters_lock);
    if (c->num_waiters >= COND_MAX_WAITERS) {
        spinlock_release(&c->waiters_lock);
        return -1; /* List full */
    }
    c->waiters[c->num_waiters] = tid;
    c->num_waiters++;
    spinlock_release(&c->waiters_lock);
    return 0;
}

/* Remove a specific thread from condvar waiter list. */
__USER_TEXT
static void cond_waiter_remove(pthread_cond_t *c, L4_ThreadId_t tid)
{
    spinlock_acquire(&c->waiters_lock);
    for (uint32_t i = 0; i < c->num_waiters; i++) {
        if (c->waiters[i].raw == tid.raw) {
            /* Shift remaining entries down */
            for (uint32_t j = i; j < c->num_waiters - 1; j++)
                c->waiters[j] = c->waiters[j + 1];
            c->num_waiters--;
            break;
        }
    }
    spinlock_release(&c->waiters_lock);
}

/* Pop first waiter from condvar list. Returns nilthread if list empty. */
__USER_TEXT
static L4_ThreadId_t cond_waiter_pop(pthread_cond_t *c)
{
    L4_ThreadId_t tid = L4_nilthread;
    spinlock_acquire(&c->waiters_lock);
    if (c->num_waiters > 0) {
        tid = c->waiters[0];
        /* Shift remaining entries down */
        for (uint32_t i = 0; i < c->num_waiters - 1; i++)
            c->waiters[i] = c->waiters[i + 1];
        c->num_waiters--;
    }
    spinlock_release(&c->waiters_lock);
    return tid;
}

__USER_TEXT
int pthread_condattr_init(pthread_condattr_t *attr)
{
    if (!attr)
        return EINVAL;

    attr->pshared = 0;
    attr->initialized = 1;
    return 0;
}

__USER_TEXT
int pthread_condattr_destroy(pthread_condattr_t *attr)
{
    if (!attr || !attr->initialized)
        return EINVAL;
    attr->initialized = 0;
    return 0;
}

__USER_TEXT
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
    if (!cond)
        return EINVAL;

    /* Validate attr if provided */
    if (attr && !attr->initialized)
        return EINVAL;

    cond->wait_count = 0;
    cond->signal_count = 0;
    cond->broadcast_seq = 0;
    cond->initialized = 1;
    cond->waiters_lock = 0;
    cond->num_waiters = 0;
    for (int i = 0; i < COND_MAX_WAITERS; i++)
        cond->waiters[i] = L4_nilthread;

    return 0;
}

__USER_TEXT
int pthread_cond_destroy(pthread_cond_t *cond)
{
    if (!cond)
        return EINVAL;

    if (!cond->initialized)
        return EINVAL;

    /* Check for waiters - cannot destroy in use */
    if (cond->wait_count > 0)
        return EBUSY;

    cond->initialized = 0;
    return 0;
}

__USER_TEXT
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    if (!cond || !mutex)
        return EINVAL;

    L4_ThreadId_t self = L4_MyGlobalId();

    /* Verify we own the mutex */
    if (mutex->owner.raw != self.raw)
        return EPERM;

    /* Capture signal/broadcast state BEFORE releasing mutex.
     * This closes the atomicity gap: if signal/broadcast occurs between
     * mutex release and blocking, we detect it via changed counters.
     *
     * Race scenario without this:
     * 1. Thread A: releases mutex
     * 2. Thread B: acquires mutex, signals, releases mutex
     * 3. Thread A: blocks on NotifyWait - signal already sent, lost!
     *
     * With sequence check:
     * 1. Thread A: captures seq, releases mutex
     * 2. Thread B: signals (increments counter, sends notification)
     * 3. Thread A: checks seq changed -> doesn't block, or wakes immediately
     */
    L4_Word_t initial_signal = cond->signal_count;
    L4_Word_t initial_broadcast = cond->broadcast_seq;

    cond->wait_count++;

    /* Register in waiter list before releasing mutex */
    while (cond_waiter_add(cond, self) < 0) {
        /* Waiter list full - yield and retry */
        L4_Yield();
    }

    /* Release the mutex */
    pthread_mutex_unlock(mutex);

    /* Wait loop with sequence-based spurious wakeup detection.
     * Continue waiting only if no signal/broadcast occurred.
     */
    while (cond->signal_count == initial_signal &&
           cond->broadcast_seq == initial_broadcast) {
        /* Block until signaled via direct notification */
        L4_NotifyWait(POSIX_NOTIFY_COND_BIT);
    }

    /* Remove from waiter list */
    cond_waiter_remove(cond, self);
    cond->wait_count--;

    /* Re-acquire mutex before returning (POSIX requirement) */
    pthread_mutex_lock(mutex);

    return 0;
}

__USER_TEXT
int pthread_cond_timedwait(pthread_cond_t *cond,
                           pthread_mutex_t *mutex,
                           const struct timespec *abstime)
{
    if (!cond || !mutex || !abstime)
        return EINVAL;

    L4_ThreadId_t self = L4_MyGlobalId();

    /* Verify we own the mutex */
    if (mutex->owner.raw != self.raw)
        return EPERM;

    /* POSIX: abstime is an absolute deadline (e.g., from clock_gettime).
     * Convert directly to microseconds - do NOT add to current time.
     */
    uint64_t deadline_us = timespec_to_us(abstime);

    /* Check if deadline has already passed */
    L4_Clock_t now = L4_SystemClock();
    if (deadline_us <= now.raw)
        return ETIMEDOUT; /* Deadline already passed */

    /* Capture initial signal/broadcast state for polling detection.
     * If these values change, we know a signal/broadcast occurred.
     */
    L4_Word_t initial_signal = cond->signal_count;
    L4_Word_t initial_broadcast = cond->broadcast_seq;

    /* Atomically release mutex and block on condition.
     * POSIX requires these to be atomic.
     */
    cond->wait_count++;

    /* Register in waiter list before releasing mutex */
    while (cond_waiter_add(cond, self) < 0) {
        /* Waiter list full - yield and retry */
        L4_Yield();
    }

    /* Release the mutex */
    pthread_mutex_unlock(mutex);

    /* Polling-based timed wait algorithm:
     *
     * Instead of relying on timer notifications (which have QEMU issues),
     * we poll with short sleep slices and check:
     * 1. If signal_count or broadcast_seq changed → signaled
     * 2. If current time >= deadline → timeout
     *
     * This is deterministic and works on both QEMU and real hardware.
     * Sleep slice of 1ms provides reasonable responsiveness.
     */
    int result = ETIMEDOUT;

    while (1) {
        /* Check if signaled (signal_count or broadcast_seq changed) */
        if (cond->signal_count != initial_signal ||
            cond->broadcast_seq != initial_broadcast) {
            result = 0; /* Signaled! */
            break;
        }

        /* Check timeout */
        L4_Clock_t now = L4_SystemClock();
        if (now.raw >= deadline_us)
            break; /* Timeout */

        /* Sleep for 1ms slice, then check again */
        L4_Sleep(L4_TimePeriod(1000));
    }

    /* Remove from waiter list */
    cond_waiter_remove(cond, self);
    cond->wait_count--;

    /* Re-acquire mutex before returning (POSIX requirement) */
    pthread_mutex_lock(mutex);

    return result;
}

__USER_TEXT
int pthread_cond_signal(pthread_cond_t *cond)
{
    if (!cond)
        return EINVAL;

    /* Increment signal count for polling-based timedwait detection */
    cond->signal_count++;

    /* Wake one waiting thread via direct notification */
    L4_ThreadId_t waiter = cond_waiter_pop(cond);
    if (waiter.raw != L4_nilthread.raw)
        L4_NotifyPost(waiter, POSIX_NOTIFY_COND_BIT);

    return 0;
}

__USER_TEXT
int pthread_cond_broadcast(pthread_cond_t *cond)
{
    if (!cond)
        return EINVAL;

    /* Increment broadcast sequence for polling-based timedwait detection */
    cond->broadcast_seq++;

    /* Wake all waiting threads via direct notification */
    L4_ThreadId_t waiter;
    while ((waiter = cond_waiter_pop(cond)).raw != L4_nilthread.raw)
        L4_NotifyPost(waiter, POSIX_NOTIFY_COND_BIT);

    return 0;
}

/* Scheduling API - POSIX_PRIORITY_SCHEDULING (PSE51 mandatory) */

__USER_TEXT
int sched_get_priority_max(int policy)
{
    switch (policy) {
    case SCHED_FIFO:
    case SCHED_RR:
        return SCHED_PRIORITY_MAX;
    case SCHED_OTHER:
        return 0; /* Normal scheduling has no priority range */
    default:
        return -1;
    }
}

__USER_TEXT
int sched_get_priority_min(int policy)
{
    switch (policy) {
    case SCHED_FIFO:
    case SCHED_RR:
        return SCHED_PRIORITY_MIN;
    case SCHED_OTHER:
        return 0;
    default:
        return -1;
    }
}

/* sched_yield is defined as static inline in sched.h to avoid
 * symbol conflict with kernel's sched_yield. User-space version
 * uses L4_Sleep for voluntary preemption.
 */

__USER_TEXT
int sched_getscheduler(pid_t pid)
{
    /* F9 uses priority-based preemptive scheduling (FIFO within priority) */
    (void) pid;
    return SCHED_FIFO;
}

__USER_TEXT
int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param)
{
    (void) pid;
    (void) param;

    /* Only SCHED_FIFO supported on F9 microkernel */
    if (policy != SCHED_FIFO && policy != SCHED_RR)
        return EINVAL;

    /* Setting would require kernel support - accept but don't change */
    return 0;
}

__USER_TEXT
int sched_getparam(pid_t pid, struct sched_param *param)
{
    if (!param)
        return EINVAL;

    (void) pid;
    /* Return default priority - actual priority stored in TCB */
    param->sched_priority = 128;
    return 0;
}

__USER_TEXT
int sched_setparam(pid_t pid, const struct sched_param *param)
{
    if (!param)
        return EINVAL;

    (void) pid;

    if (param->sched_priority < SCHED_PRIORITY_MIN ||
        param->sched_priority > SCHED_PRIORITY_MAX)
        return EINVAL;

    /* Setting priority would require pager/kernel IPC */
    return 0;
}

/* Thread scheduling attributes */

__USER_TEXT
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
    if (!attr)
        return EINVAL;

    if (policy != SCHED_OTHER && policy != SCHED_FIFO && policy != SCHED_RR)
        return EINVAL;

    /* Store in attr - will use when creating thread */
    /* Note: attr struct would need schedpolicy field for full impl */
    (void) policy;
    return 0;
}

__USER_TEXT
int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
    if (!attr || !policy)
        return EINVAL;

    *policy = SCHED_FIFO; /* F9 default */
    return 0;
}

__USER_TEXT
int pthread_attr_setschedparam(pthread_attr_t *attr,
                               const struct sched_param *param)
{
    if (!attr || !param)
        return EINVAL;

    if (param->sched_priority < SCHED_PRIORITY_MIN ||
        param->sched_priority > SCHED_PRIORITY_MAX)
        return EINVAL;

    attr->priority = param->sched_priority;
    return 0;
}

__USER_TEXT
int pthread_attr_getschedparam(const pthread_attr_t *attr,
                               struct sched_param *param)
{
    if (!attr || !param)
        return EINVAL;

    param->sched_priority = attr->priority;
    return 0;
}

__USER_TEXT
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched)
{
    if (!attr)
        return EINVAL;

    if (inheritsched != PTHREAD_INHERIT_SCHED &&
        inheritsched != PTHREAD_EXPLICIT_SCHED)
        return EINVAL;

    /* Would need inheritsched field in attr for full impl */
    (void) inheritsched;
    return 0;
}

__USER_TEXT
int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched)
{
    if (!attr || !inheritsched)
        return EINVAL;

    *inheritsched = PTHREAD_INHERIT_SCHED; /* Default */
    return 0;
}

/* Thread scheduling parameter functions */

__USER_TEXT
int pthread_setschedparam(pthread_t *thread,
                          int policy,
                          const struct sched_param *param)
{
    if (!thread || !param)
        return EINVAL;

    if (policy != SCHED_OTHER && policy != SCHED_FIFO && policy != SCHED_RR)
        return EINVAL;

    if (param->sched_priority < SCHED_PRIORITY_MIN ||
        param->sched_priority > SCHED_PRIORITY_MAX)
        return EINVAL;

    /* Would require pager IPC to change thread priority */
    (void) thread;
    return 0;
}

__USER_TEXT
int pthread_getschedparam(pthread_t *thread,
                          int *policy,
                          struct sched_param *param)
{
    if (!thread || !policy || !param)
        return EINVAL;

    *policy = SCHED_FIFO;
    param->sched_priority = 128; /* Default - would query pager for actual */
    return 0;
}

__USER_TEXT
int pthread_setschedprio(pthread_t *thread, int prio)
{
    if (!thread)
        return EINVAL;

    if (prio < SCHED_PRIORITY_MIN || prio > SCHED_PRIORITY_MAX)
        return EINVAL;

    /* Would require pager IPC to change priority */
    return 0;
}

/*
 * PSE52 Profile: Read-Write Locks (POSIX_READER_WRITER_LOCKS option)
 *
 * Implementation uses two mutexes:
 * - rd_mutex: protects reader count
 * - wr_mutex: exclusive writer access
 *
 * Readers: multiple concurrent allowed
 * Writers: exclusive access, waits for all readers to finish
 */

__USER_TEXT
int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
    if (!attr)
        return EINVAL;

    attr->pshared = 0;
    attr->initialized = 1;
    return 0;
}

__USER_TEXT
int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
{
    if (!attr || !attr->initialized)
        return EINVAL;
    attr->initialized = 0;
    return 0;
}

__USER_TEXT
int pthread_rwlock_init(pthread_rwlock_t *rwlock,
                        const pthread_rwlockattr_t *attr)
{
    if (!rwlock)
        return EINVAL;

    if (attr && !attr->initialized)
        return EINVAL;

    int ret;

    ret = pthread_mutex_init(&rwlock->rd_mutex, NULL);
    if (ret != 0)
        return ret;

    ret = pthread_mutex_init(&rwlock->wr_mutex, NULL);
    if (ret != 0) {
        pthread_mutex_destroy(&rwlock->rd_mutex);
        return ret;
    }

    rwlock->readers = 0;
    rwlock->writer.raw = 0;
    rwlock->initialized = 1;

    return 0;
}

__USER_TEXT
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
    if (!rwlock || !rwlock->initialized)
        return EINVAL;

    /* Cannot destroy if in use */
    if (rwlock->readers > 0 || rwlock->writer.raw != 0)
        return EBUSY;

    pthread_mutex_destroy(&rwlock->rd_mutex);
    pthread_mutex_destroy(&rwlock->wr_mutex);
    rwlock->initialized = 0;

    return 0;
}

__USER_TEXT
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
    if (!rwlock || !rwlock->initialized)
        return EINVAL;

    /* Acquire writer mutex to block new writers */
    pthread_mutex_lock(&rwlock->wr_mutex);

    /* Increment reader count */
    pthread_mutex_lock(&rwlock->rd_mutex);
    rwlock->readers++;
    pthread_mutex_unlock(&rwlock->rd_mutex);

    /* Release writer mutex - readers can coexist */
    pthread_mutex_unlock(&rwlock->wr_mutex);

    return 0;
}

__USER_TEXT
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{
    if (!rwlock || !rwlock->initialized)
        return EINVAL;

    /* Try to acquire writer mutex */
    if (pthread_mutex_trylock(&rwlock->wr_mutex) != 0)
        return EBUSY;

    /* Increment reader count */
    pthread_mutex_lock(&rwlock->rd_mutex);
    rwlock->readers++;
    pthread_mutex_unlock(&rwlock->rd_mutex);

    /* Release writer mutex */
    pthread_mutex_unlock(&rwlock->wr_mutex);

    return 0;
}

__USER_TEXT
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
    if (!rwlock || !rwlock->initialized)
        return EINVAL;

    /* Acquire writer mutex for exclusive access */
    pthread_mutex_lock(&rwlock->wr_mutex);

    /* Wait for all readers to finish */
    while (rwlock->readers > 0) {
        pthread_mutex_unlock(&rwlock->wr_mutex);
        L4_Sleep(L4_TimePeriod(1000)); /* Yield, check again */
        pthread_mutex_lock(&rwlock->wr_mutex);
    }

    rwlock->writer = L4_MyGlobalId();
    return 0;
}

__USER_TEXT
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{
    if (!rwlock || !rwlock->initialized)
        return EINVAL;

    /* Try to acquire writer mutex */
    if (pthread_mutex_trylock(&rwlock->wr_mutex) != 0)
        return EBUSY;

    /* Check for active readers */
    if (rwlock->readers > 0) {
        pthread_mutex_unlock(&rwlock->wr_mutex);
        return EBUSY;
    }

    rwlock->writer = L4_MyGlobalId();
    return 0;
}

__USER_TEXT
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
    if (!rwlock || !rwlock->initialized)
        return EINVAL;

    L4_ThreadId_t self = L4_MyGlobalId();

    /* Check if we're the writer */
    if (rwlock->writer.raw == self.raw) {
        rwlock->writer.raw = 0;
        pthread_mutex_unlock(&rwlock->wr_mutex);
        return 0;
    }

    /* Otherwise, we're a reader */
    pthread_mutex_lock(&rwlock->rd_mutex);
    if (rwlock->readers > 0)
        rwlock->readers--;
    pthread_mutex_unlock(&rwlock->rd_mutex);

    return 0;
}

/*
 * PSE52 Profile: Barriers (POSIX_BARRIERS option)
 *
 * Barrier synchronization point for multiple threads.
 * All threads block until count threads have called barrier_wait().
 */

__USER_TEXT
int pthread_barrierattr_init(pthread_barrierattr_t *attr)
{
    if (!attr)
        return EINVAL;

    attr->pshared = 0;
    attr->initialized = 1;
    return 0;
}

__USER_TEXT
int pthread_barrierattr_destroy(pthread_barrierattr_t *attr)
{
    if (!attr || !attr->initialized)
        return EINVAL;
    attr->initialized = 0;
    return 0;
}

__USER_TEXT
int pthread_barrier_init(pthread_barrier_t *barrier,
                         const pthread_barrierattr_t *attr,
                         unsigned int count)
{
    if (!barrier || count == 0)
        return EINVAL;

    if (attr && !attr->initialized)
        return EINVAL;

    int ret;

    ret = pthread_mutex_init(&barrier->mutex, NULL);
    if (ret != 0)
        return ret;

    ret = pthread_cond_init(&barrier->cond, NULL);
    if (ret != 0) {
        pthread_mutex_destroy(&barrier->mutex);
        return ret;
    }

    barrier->count = count;
    barrier->waiting = 0;
    barrier->cycle = 0;
    barrier->initialized = 1;

    return 0;
}

__USER_TEXT
int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
    if (!barrier || !barrier->initialized)
        return EINVAL;

    /* Cannot destroy if threads are waiting */
    if (barrier->waiting > 0)
        return EBUSY;

    pthread_cond_destroy(&barrier->cond);
    pthread_mutex_destroy(&barrier->mutex);
    barrier->initialized = 0;

    return 0;
}

__USER_TEXT
int pthread_barrier_wait(pthread_barrier_t *barrier)
{
    if (!barrier || !barrier->initialized)
        return EINVAL;

    pthread_mutex_lock(&barrier->mutex);

    uint32_t my_cycle = barrier->cycle;
    barrier->waiting++;

    if (barrier->waiting == barrier->count) {
        /* Last thread to arrive - release all */
        barrier->waiting = 0;
        barrier->cycle++;
        pthread_cond_broadcast(&barrier->cond);
        pthread_mutex_unlock(&barrier->mutex);
        return PTHREAD_BARRIER_SERIAL_THREAD;
    }

    /* Wait for other threads */
    while (my_cycle == barrier->cycle)
        pthread_cond_wait(&barrier->cond, &barrier->mutex);

    pthread_mutex_unlock(&barrier->mutex);
    return 0;
}

/* PSE52: Spinlock implementation (POSIX_SPIN_LOCKS option)
 *
 * Lightweight busy-wait synchronization using TTAS (Test-and-Test-and-Set):
 * 1. Plain load spin until lock appears free (cache-friendly)
 * 2. Attempt LDREX/STREX atomic acquire only when free
 *
 * Appropriate for very short critical sections where blocking overhead
 * exceeds the expected spin time. NOT for long critical sections.
 */

__USER_TEXT
int pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
    if (!lock)
        return EINVAL;

    if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED)
        return EINVAL;

    lock->lock = 0;
    lock->pshared = (uint8_t) pshared;
    lock->initialized = 1;

    return 0;
}

__USER_TEXT
int pthread_spin_destroy(pthread_spinlock_t *lock)
{
    if (!lock || !lock->initialized)
        return EINVAL;

    /* Cannot destroy if locked */
    if (lock->lock != 0)
        return EBUSY;

    lock->initialized = 0;
    return 0;
}

__USER_TEXT
int pthread_spin_lock(pthread_spinlock_t *lock)
{
    if (!lock || !lock->initialized)
        return EINVAL;

    /* TTAS pattern: spin on plain load, then attempt atomic acquire */
    uint32_t status;
    while (1) {
        /* Test: Spin on plain load until lock appears free */
        while (*(volatile uint32_t *) &lock->lock != 0) {
            /* Busy wait - yields naturally at tick on single-core */
        }

        /* Test-and-Set: Attempt atomic acquire with LDREX/STREX
         * Note: STREX Rd, Rt, [Rn] requires Rd != Rt, so we use r0 for value */
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
            : [lock] "r"(&lock->lock)
            : "r0", "cc", "memory");

        if (status == 0) {
            __asm__ __volatile__("dmb" ::: "memory"); /* Acquire barrier */
            return 0;
        }
        /* STREX failed or lock was taken - retry */
    }
}

__USER_TEXT
int pthread_spin_trylock(pthread_spinlock_t *lock)
{
    if (!lock || !lock->initialized)
        return EINVAL;

    /* Single attempt to acquire lock
     * Note: STREX Rd, Rt, [Rn] requires Rd != Rt, so we use r0 for value */
    uint32_t status;
    __asm__ __volatile__(
        "ldrex r0, [%[lock]]\n"
        "cmp r0, #0\n"
        "bne 1f\n" /* Already locked */
        "mov r0, #1\n"
        "strex %[status], r0, [%[lock]]\n"
        "b 2f\n"
        "1: mov %[status], #1\n" /* Mark as failed (lock held) */
        "2:\n"
        : [status] "=&r"(status)
        : [lock] "r"(&lock->lock)
        : "r0", "cc", "memory");

    if (status == 0) {
        __asm__ __volatile__("dmb" ::: "memory"); /* Acquire barrier */
        return 0;
    }

    return EBUSY;
}

__USER_TEXT
int pthread_spin_unlock(pthread_spinlock_t *lock)
{
    if (!lock || !lock->initialized)
        return EINVAL;

    /* Release barrier before clearing lock */
    __asm__ __volatile__("dmb" ::: "memory");
    lock->lock = 0;

    return 0;
}
