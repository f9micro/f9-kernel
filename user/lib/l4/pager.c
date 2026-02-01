/* Copyright (c) 2014,2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/ipc.h>
#include <l4/kip_types.h>
#include <l4/pager.h>
#include <l4/utcb.h>
#include <l4io.h>

/* POSIX error codes for pager join protocol */
#define ESRCH 3   /* No such process/thread */
#define EAGAIN 11 /* Resource temporarily unavailable */

#define STACK_SIZE 0x200

/* Kernel requires 256-byte alignment for UTCB addresses */
#define NODE_ALIGN 256
#define NODE_SIZE_ALIGNED \
    (((UTCB_SIZE + STACK_SIZE) + NODE_ALIGN - 1) & ~(NODE_ALIGN - 1))

typedef void *thr_handler_t(void *);

struct thread_node {
    L4_Word_t base; /* stack + utcb */
    L4_ThreadId_t tid;
    void *retval;     /* Thread return value for pthread_join */
    uint8_t exited;   /* Thread has exited flag */
    uint8_t detached; /* Thread is detached (no join needed) */
};

static inline void use_thread_node(struct thread_node *node)
{
    node->base |= 0x1;
}

static inline void free_thread_node(struct thread_node *node)
{
    node->base &= ~0x1;
}

#define IS_THREAD_NODE_USED(node) ((node)->base & 0x1)

#define THREAD_NODE_BASE(node) ((node)->base & ~0x1)

struct thread_pool {
    struct thread_node *all_nodes;
    L4_Word_t node_num;
    L4_ThreadId_t pager_tid;
};

/* Wait queue for blocking synchronization (PSE51 compliance) */
#define MAX_WAITERS 16
#define MAX_SYNC_OBJECTS 8

/* Type tags for wait queue keys to prevent resource ID collisions.
 * Without tagging, a resource address could equal a thread ID, causing
 * the wrong waiter to be released.
 *
 * NOTE: WT_MUTEX, WT_SEM, WT_COND removed - sync primitives now use
 * direct kernel notifications instead of pager IPC.
 */
enum wait_type {
    WT_JOIN = 4 /* Thread join wait - still uses pager IPC */
};

#define WAIT_KEY(type, id) ((((L4_Word_t) (type)) << 28) | ((id) & 0x0FFFFFFF))

struct wait_entry {
    L4_ThreadId_t tid;     /* Waiting thread */
    L4_Word_t resource_id; /* Tagged resource key */
    uint8_t valid;         /* Entry is valid */
};

struct sync_state {
    struct wait_entry waiters[MAX_WAITERS];
    L4_Word_t waiter_count;
};

__USER_BSS static struct sync_state g_sync_state;

/* Add thread to wait queue for a resource */
__USER_TEXT
static int add_waiter(L4_Word_t resource_id, L4_ThreadId_t tid)
{
    int i;
    for (i = 0; i < MAX_WAITERS; i++) {
        if (!g_sync_state.waiters[i].valid) {
            g_sync_state.waiters[i].tid = tid;
            g_sync_state.waiters[i].resource_id = resource_id;
            g_sync_state.waiters[i].valid = 1;
            g_sync_state.waiter_count++;
            return 0;
        }
    }
    return -1; /* Wait queue full */
}

/* Find and remove first waiter for a resource */
__USER_TEXT
static L4_ThreadId_t pop_waiter(L4_Word_t resource_id)
{
    int i;
    L4_ThreadId_t nil;
    nil.raw = 0;

    for (i = 0; i < MAX_WAITERS; i++) {
        if (g_sync_state.waiters[i].valid &&
            g_sync_state.waiters[i].resource_id == resource_id) {
            L4_ThreadId_t tid = g_sync_state.waiters[i].tid;
            g_sync_state.waiters[i].valid = 0;
            g_sync_state.waiter_count--;
            return tid;
        }
    }
    return nil;
}

/* Wake all waiters for a resource (for broadcast) */
__USER_TEXT
static int wake_all_waiters(L4_Word_t resource_id) __attribute__((unused));

__USER_TEXT
static int wake_all_waiters(L4_Word_t resource_id)
{
    int i, count = 0;
    L4_Msg_t msg;

    for (i = 0; i < MAX_WAITERS; i++) {
        if (g_sync_state.waiters[i].valid &&
            g_sync_state.waiters[i].resource_id == resource_id) {
            L4_ThreadId_t tid = g_sync_state.waiters[i].tid;
            g_sync_state.waiters[i].valid = 0;
            g_sync_state.waiter_count--;

            /* Send wake-up reply */
            L4_MsgClear(&msg);
            L4_Set_Label(&msg.tag, PAGER_REPLY_LABEL);
            L4_MsgAppendWord(&msg, 0); /* Success */
            L4_MsgLoad(&msg);
            L4_Send(tid);
            count++;
        }
    }
    return count;
}

static inline int find_thread_index(struct thread_pool *pool, L4_ThreadId_t tid)
{
    /* Thread ID encoding: tid = pager_tid + (thr_idx << 14)
     * So: thr_idx = (tid - pager_tid) >> 14
     */
    return (tid.raw - (pool->pager_tid).raw) >> 14;
}

static inline L4_Word_t user_fpage_number(user_fpage_t *fpages)
{
    int num = 0;

    while (fpages[num].base != 0 || fpages[num].size != 0)
        num++;

    return num;
}

__USER_TEXT
static struct thread_pool *init_thread_pool(L4_Word_t res_base,
                                            L4_Word_t res_size,
                                            L4_Word_t heap_base,
                                            L4_Word_t heap_size)
{
    L4_Word_t node_num, i;
    L4_Word_t num1, num2;
    struct thread_pool *pool;
    struct thread_node *nodes;

    num1 =
        (heap_size - sizeof(struct thread_pool)) / sizeof(struct thread_node);
    num2 = (res_size) / NODE_SIZE_ALIGNED;

    node_num = (num1 < num2) ? num1 : num2;
    node_num = (node_num > THREAD_MAX_NUM) ? THREAD_MAX_NUM : node_num;

    pool = (struct thread_pool *) heap_base;
    nodes = (struct thread_node *) (heap_base + sizeof(struct thread_pool));

    pool->node_num = node_num;
    pool->all_nodes = nodes;
    pool->pager_tid = L4_MyGlobalId();

    if (res_base & 0x1) {
        printf("unalign res base\n");
        return NULL;
    }

    for (i = 1; i < node_num; i++) {
        nodes[i].base = res_base;
        nodes[i].tid.raw = 0;
        nodes[i].retval = (void *) 0;
        nodes[i].exited = 0;
        nodes[i].detached = 0;
        res_base += NODE_SIZE_ALIGNED;
    }

    return pool;
}

__USER_TEXT
static L4_Word_t fetch_free_thread_index(struct thread_pool *pool)
{
    int i;
    struct thread_node *node;

    for (i = 1; i < pool->node_num; i++) {
        node = &pool->all_nodes[i];
        if (!IS_THREAD_NODE_USED(node))
            break;
    }

    if (i == pool->node_num)
        return 0;

    return i;
}

__USER_TEXT
static struct thread_node *find_thread_node(struct thread_pool *pool,
                                            L4_ThreadId_t tid)
{
    int i;

    for (i = 1; i < pool->node_num; i++) {
        if (pool->all_nodes[i].tid.raw == tid.raw &&
            IS_THREAD_NODE_USED(&pool->all_nodes[i])) {
            return &pool->all_nodes[i];
        }
    }

    return NULL;
}

__USER_TEXT
static void release_thread(struct thread_pool *pool, L4_ThreadId_t tid)
{
    L4_Word_t idx;

    idx = find_thread_index(pool, tid);

    if (idx >= pool->node_num) {
        printf("tid %p not found.\n", tid);
        return;
    }

    free_thread_node(&pool->all_nodes[idx]);
    L4_ThreadControl(tid, L4_nilthread, L4_nilthread, L4_nilthread,
                     (void *) -1);
}

__USER_TEXT
void thread_container(kip_t *kip_ptr,
                      utcb_t *utcb_ptr,
                      L4_Word_t entry,
                      L4_Word_t entry_arg)
{
    L4_Msg_t msg;
    void *retval;

    /* Set current_utcb to this thread's UTCB before calling entry.
     * This ensures L4_MyGlobalId() returns the correct thread ID.
     * The kernel passes us our UTCB pointer, but the global current_utcb
     * may still point to the parent's UTCB at this point.
     * Use volatile cast and memory barrier to ensure visibility.
     */
    extern void *current_utcb;
    *(void *volatile *) &current_utcb = utcb_ptr;
    __asm__ __volatile__("dmb" ::: "memory");

    /* Execute thread function and capture return value */
    retval = ((thr_handler_t *) entry)((void *) entry_arg);

    /* Send return value to pager for pthread_join.
     * Use L4_Send (not L4_Call) since we don't need a reply.
     */
    L4_MsgClear(&msg);
    L4_Set_Label(&msg.tag, PAGER_REQUEST_LABEL);
    L4_MsgAppendWord(&msg, THREAD_FREE);
    L4_MsgAppendWord(&msg, (L4_Word_t) retval); /* Send return value */
    L4_MsgLoad(&msg);
    L4_Send(L4_Pager());

    /* Thread is done - halt forever.
     * We can't return from this function (no valid return address).
     */
    while (1)
        L4_Sleep(L4_Never);
}

__USER_TEXT
static void start_thread(L4_ThreadId_t t,
                         L4_Word_t entry,
                         L4_Word_t entry_arg,
                         L4_Word_t sp,
                         L4_Word_t stack_size)
{
    L4_Msg_t msg;

    /* CRITICAL: Do NOT call functions after L4_MsgLoad!
     * Any function call (including printf) will clobber R4-R11 (MR0-MR7)!
     */
    L4_MsgClear(&msg);
    L4_MsgAppendWord(&msg, (L4_Word_t) thread_container);
    L4_MsgAppendWord(&msg, sp);
    L4_MsgAppendWord(&msg, stack_size);
    L4_MsgAppendWord(&msg, entry);
    L4_MsgAppendWord(&msg, entry_arg);
    L4_MsgLoad(&msg);
    L4_Send(t);
}

__USER_TEXT
static L4_ThreadId_t __thread_create(struct thread_pool *pool)
{
    L4_ThreadId_t child, myself;
    L4_Word_t thr_idx;
    L4_Word_t free_mem;
    struct thread_node *node;

    thr_idx = fetch_free_thread_index(pool);
    if (thr_idx == 0) {
        printf("No free res for new thread\n");
        child.raw = 0;
        return child;
    }

    child.raw = (pool->pager_tid).raw + (thr_idx << 14);
    node = &pool->all_nodes[thr_idx];
    node->tid = child;
    node->retval = NULL;
    node->exited = 0;
    node->detached = 0;
    use_thread_node(node);

    myself = L4_MyGlobalId();
    free_mem = (L4_Word_t) THREAD_NODE_BASE(node);

    /* Create thread with shared address space (spaceid=myself).
     * Since spaceid != dest, the kernel's thread_space() will share
     * the pager's address space with the child. All fpages in the
     * pager's AS (including RES_FPAGE for stack/UTCB, UTEXT, UDATA,
     * UBSS) are automatically accessible to the child.
     *
     * Note: The kernel maps the UTCB via map_area() during ThreadControl.
     * The stack region is part of RES_FPAGE which is already in the
     * pager's AS, so no explicit mapping is needed here.
     */
    L4_ThreadControl(child, myself, L4_nilthread, myself, (void *) free_mem);

    return child;
}

__USER_TEXT
static L4_Word_t __thread_start(struct thread_pool *pool,
                                L4_ThreadId_t tid,
                                L4_Word_t entry,
                                L4_Word_t entry_arg)
{
    struct thread_node *node;
    L4_Word_t stack;

    node = find_thread_node(pool, tid);

    if (!node) {
        printf("__thread_start: node not found for %p\n", tid.raw);
        return (L4_Word_t) -1;
    }

    stack = (L4_Word_t) THREAD_NODE_BASE(node) + UTCB_SIZE + STACK_SIZE;
    /* Explicit 8-byte alignment for ARM AAPCS compliance.
     * If UTCB_SIZE or STACK_SIZE changes, alignment is preserved.
     */
    stack &= ~0x7u;
    start_thread(tid, entry, entry_arg, stack, STACK_SIZE);

    return 0;
}

__USER_TEXT
L4_ThreadId_t pager_create_thread(void)
{
    L4_ThreadId_t tid;
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    L4_ThreadId_t pager_tid = L4_Pager();

    L4_MsgClear(&msg);
    L4_Set_Label(&msg.tag, PAGER_REQUEST_LABEL);
    L4_MsgAppendWord(&msg, THREAD_CREATE);

    L4_MsgLoad(&msg);
    tag = L4_Call(pager_tid);

    if (L4_Label(tag) == PAGER_REPLY_LABEL)
        L4_StoreMR(1, &tid.raw);
    else
        tid.raw = 0;

    return tid;
}

__USER_TEXT
L4_Word_t pager_start_thread(L4_ThreadId_t tid,
                             void *(*thr_routine)(void *),
                             void *arg)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    L4_Word_t ret;

    L4_MsgClear(&msg);
    L4_Set_Label(&msg.tag, PAGER_REQUEST_LABEL);
    L4_MsgAppendWord(&msg, THREAD_START);
    L4_MsgAppendWord(&msg, (L4_Word_t) tid.raw);
    L4_MsgAppendWord(&msg, (L4_Word_t) thr_routine);
    L4_MsgAppendWord(&msg, (L4_Word_t) arg);

    L4_MsgLoad(&msg);
    tag = L4_Call(L4_Pager());

    if (L4_Label(tag) == PAGER_REPLY_LABEL) {
        L4_StoreMR(1, &ret);
    } else {
        ret = -1;
    }

    return ret;
}

__USER_TEXT
void *pager_get_thread_retval(L4_ThreadId_t tid)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    L4_Word_t retval;

    L4_MsgClear(&msg);
    L4_Set_Label(&msg.tag, PAGER_REQUEST_LABEL);
    L4_MsgAppendWord(&msg, THREAD_GET_RETVAL);
    L4_MsgAppendWord(&msg, (L4_Word_t) tid.raw);

    L4_MsgLoad(&msg);
    tag = L4_Call(L4_Pager());

    if (L4_Label(tag) == PAGER_REPLY_LABEL) {
        L4_StoreMR(1, &retval);
        return (void *) retval;
    }

    return NULL;
}

/* NOTE: pager_mutex_lock/unlock removed - mutexes now use direct kernel
 * notifications (L4_NotifyWait/L4_NotifyPost) instead of pager IPC.
 *
 * NOTE: pager_sem_wait/pager_sem_post removed - semaphores now use
 * direct kernel notifications (L4_NotifyWait/L4_NotifyPost) instead
 * of pager IPC round-trips for better performance.
 */

__USER_TEXT
int pager_thread_join(L4_ThreadId_t tid, void **retval)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    L4_Word_t status, result;

    L4_MsgClear(&msg);
    L4_Set_Label(&msg.tag, PAGER_REQUEST_LABEL);
    L4_MsgAppendWord(&msg, THREAD_JOIN_WAIT);
    L4_MsgAppendWord(&msg, tid.raw);

    L4_MsgLoad(&msg);
    tag = L4_Call(L4_Pager()); /* Blocks until thread exits */

    if (L4_Label(tag) == PAGER_REPLY_LABEL) {
        /* Protocol: Word0 = status (0=success, ESRCH/EAGAIN=error)
         *           Word1 = retval (only valid if status == 0)
         */
        L4_StoreMR(1, &status);
        L4_StoreMR(2, &result);
        if (status != 0)
            return (int) status; /* Return ESRCH or EAGAIN */
        if (retval)
            *retval = (void *) result;
        return 0;
    }

    return -1; /* IPC error */
}

__USER_TEXT
int pager_thread_detach(L4_ThreadId_t tid)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag;

    L4_MsgClear(&msg);
    L4_Set_Label(&msg.tag, PAGER_REQUEST_LABEL);
    L4_MsgAppendWord(&msg, THREAD_DETACH);
    L4_MsgAppendWord(&msg, tid.raw);

    L4_MsgLoad(&msg);
    tag = L4_Call(L4_Pager());

    if (L4_Label(tag) == PAGER_REPLY_LABEL)
        return 0;

    return -1;
}

/* NOTE: pager_cond_wait/signal/broadcast removed - condition variables now
 * use direct kernel notifications (L4_NotifyWait/L4_NotifyPost) instead of
 * pager IPC round-trips for better performance.
 */

__USER_TEXT
void pager_thread(user_struct *user, void *(*entry_main)(void *) )
{
    L4_Word_t fpage_num;
    L4_ThreadId_t main_tid;
    struct thread_pool *pool;

    fpage_num = user_fpage_number(user->fpages);

    if (fpage_num < 2) {
        /* TODO: fpage 0: stack + tcb, fpage 1: heap */
        printf("pager: There is no enough fpages\n");
        return;
    }

    pool = init_thread_pool(
        user->fpages[RES_FPAGE].base, user->fpages[RES_FPAGE].size,
        user->fpages[HEAP_FPAGE].base, user->fpages[HEAP_FPAGE].size);

    /* Create main entry thread */
    main_tid = __thread_create(pool);
    __thread_start(pool, main_tid, (L4_Word_t) entry_main, 0);

    /* Handle ipc request */
    while (1) {
        L4_Msg_t msg;
        L4_ThreadId_t request_tid;
        L4_MsgTag_t tag;
        L4_Word_t req;

        tag = L4_Wait(&request_tid);
        if (!L4_IpcSucceeded(tag)) { /* TODO: error message */
            printf("Invalid tag\n");
            continue;
        }

        if (L4_Label(tag) != PAGER_REQUEST_LABEL) {
            printf("Invalid label = %p\n", L4_Label(tag));
            continue;
        }

        L4_MsgStore(tag, &msg);
        req = L4_MsgWord(&msg, 0);


        switch (req) {
        case THREAD_CREATE: {
            L4_ThreadId_t tid;
            tid = __thread_create(pool);

            /* return tid back */
            L4_MsgClear(&msg);
            L4_Set_Label(&msg.tag, PAGER_REPLY_LABEL);
            L4_MsgAppendWord(&msg, tid.raw);
            L4_MsgLoad(&msg);
            L4_Send(request_tid);
        } break;
        case THREAD_START: {
            L4_Word_t entry;
            L4_Word_t entry_arg;
            L4_ThreadId_t tid;
            L4_Word_t ret;

            tid.raw = L4_MsgWord(&msg, 1);
            entry = L4_MsgWord(&msg, 2);
            entry_arg = L4_MsgWord(&msg, 3);

            ret = __thread_start(pool, tid, entry, entry_arg);

            L4_MsgClear(&msg);
            L4_Set_Label(&msg.tag, PAGER_REPLY_LABEL);
            L4_MsgAppendWord(&msg, ret);
            L4_MsgLoad(&msg);
            L4_Send(request_tid);
        } break;
        case THREAD_FREE: {
            L4_Word_t retval;
            struct thread_node *node;
            L4_ThreadId_t waiter;

            /* Get return value from message */
            retval = L4_MsgWord(&msg, 1);

            /* Store return value and mark thread as exited */
            node = find_thread_node(pool, request_tid);
            if (node) {
                node->retval = (void *) retval;
                node->exited = 1;
            }

            /* Wake any thread waiting in pthread_join.
             * Protocol: Word0 = status (0=success), Word1 = retval
             */
            waiter = pop_waiter(WAIT_KEY(WT_JOIN, request_tid.raw));
            if (waiter.raw != 0) {
                L4_MsgClear(&msg);
                L4_Set_Label(&msg.tag, PAGER_REPLY_LABEL);
                L4_MsgAppendWord(&msg, 0); /* Status: success */
                L4_MsgAppendWord(&msg, retval);
                L4_MsgLoad(&msg);
                L4_Send(waiter);
                /* Joiner received retval - release thread now */
                release_thread(pool, request_tid);
            } else if (node && node->detached) {
                /* Detached thread with no waiter - release immediately */
                release_thread(pool, request_tid);
            }

            /* Note: Thread used L4_Send, no reply needed.
             * Joinable threads with waiter: released after sending retval.
             * Detached threads: released above.
             * Joinable without waiter: kept for later join.
             */
        } break;
        case THREAD_WAIT:
            break;
        case THREAD_GET_RETVAL: {
            L4_ThreadId_t query_tid;
            struct thread_node *node;
            L4_Word_t retval = 0;
            int should_release = 0;

            query_tid.raw = L4_MsgWord(&msg, 1);
            node = find_thread_node(pool, query_tid);
            if (node) {
                retval = (L4_Word_t) node->retval;
                /* Mark for release if thread has exited
                 * This prevents thread pool exhaustion from
                 * repeated create/join cycles.
                 */
                if (node->exited)
                    should_release = 1;
            }

            L4_MsgClear(&msg);
            L4_Set_Label(&msg.tag, PAGER_REPLY_LABEL);
            L4_MsgAppendWord(&msg, retval);
            L4_MsgLoad(&msg);
            L4_Send(request_tid);

            /* Release thread node after reply sent
             * Only release if thread has exited (joined threads)
             */
            if (should_release)
                release_thread(pool, query_tid);
            break;
        }

            /* NOTE: MUTEX_LOCK_REQUEST/MUTEX_UNLOCK_NOTIFY cases removed.
             * Mutexes now use direct kernel notifications instead of pager IPC.
             *
             * NOTE: SEM_WAIT_REQUEST/SEM_POST_NOTIFY cases removed.
             * Semaphores now use direct kernel notifications.
             */

        case THREAD_JOIN_WAIT: {
            L4_ThreadId_t target_tid;
            struct thread_node *node;

            target_tid.raw = L4_MsgWord(&msg, 1);
            node = find_thread_node(pool, target_tid);

            if (node && node->exited) {
                /* Thread already exited - return immediately.
                 * Protocol: Word0 = status (0=success), Word1 = retval
                 */
                L4_Word_t retval = (L4_Word_t) node->retval;
                L4_MsgClear(&msg);
                L4_Set_Label(&msg.tag, PAGER_REPLY_LABEL);
                L4_MsgAppendWord(&msg, 0); /* Status: success */
                L4_MsgAppendWord(&msg, retval);
                L4_MsgLoad(&msg);
                L4_Send(request_tid);
                /* Joiner received retval - release thread now */
                release_thread(pool, target_tid);
            } else if (node) {
                /* Thread still running - add to wait queue */
                if (add_waiter(WAIT_KEY(WT_JOIN, target_tid.raw), request_tid) <
                    0) {
                    /* Wait queue full - return EAGAIN error */
                    L4_MsgClear(&msg);
                    L4_Set_Label(&msg.tag, PAGER_REPLY_LABEL);
                    L4_MsgAppendWord(&msg, EAGAIN); /* Status: resource busy */
                    L4_MsgAppendWord(&msg, 0);
                    L4_MsgLoad(&msg);
                    L4_Send(request_tid);
                }
                /* Success: don't reply - block until thread exits */
            } else {
                /* Thread not found - return ESRCH error.
                 * Protocol: Word0 = status (ESRCH), Word1 = unused
                 */
                L4_MsgClear(&msg);
                L4_Set_Label(&msg.tag, PAGER_REPLY_LABEL);
                L4_MsgAppendWord(&msg, ESRCH); /* Status: no such thread */
                L4_MsgAppendWord(&msg, 0);
                L4_MsgLoad(&msg);
                L4_Send(request_tid);
            }
        } break;

        case THREAD_DETACH: {
            L4_ThreadId_t target_tid;
            struct thread_node *node;

            target_tid.raw = L4_MsgWord(&msg, 1);
            node = find_thread_node(pool, target_tid);

            if (node) {
                node->detached = 1;
                /* If already exited, release now */
                if (node->exited) {
                    release_thread(pool, target_tid);
                }
            }

            /* Reply to caller */
            L4_MsgClear(&msg);
            L4_Set_Label(&msg.tag, PAGER_REPLY_LABEL);
            L4_MsgAppendWord(&msg, 0);
            L4_MsgLoad(&msg);
            L4_Send(request_tid);
        } break;

            /* NOTE: COND_WAIT_REQUEST/COND_SIGNAL_NOTIFY/COND_BROADCAST_NOTIFY
             * cases removed. Condition variables now use direct kernel
             * notifications instead of pager IPC.
             */
        }
    }
}
