/* Copyright (c) 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __PAGER_H__
#define __PAGER_H__

#define RES_FPAGE 0
#define HEAP_FPAGE 1


#define THREAD_MAX_NUM 32

/* msg tag label */
#define PAGER_REQUEST_LABEL 0xf000
#define PAGER_REPLY_LABEL 0xf001

#include <platform/link.h>
#include <user_runtime.h>

enum {
    PAGER_REQ_TYPE,
    PAGER_REQ_ARG0,
    PAGER_REQ_ARG1,
    PAGER_REQ_ARG2,
    PAGER_REQ_ARG3,
    PAGER_REQ_ARG4,
    PAGER_REQ_ARG5,
    PAGER_REQ_ARG6,
    PAGER_REQ_ARG7,
};

enum {
    THREAD_CREATE,
    THREAD_START,
    THREAD_CREATE_START,
    THREAD_FREE,
    THREAD_WAIT,
    THREAD_GET_RETVAL, /* Query thread return value for pthread_join */
    THREAD_DETACH,     /* Mark thread as detached (releases node on exit) */
    THREAD_JOIN_WAIT,  /* Block until thread exits */
    /* NOTE: MUTEX_LOCK_REQUEST, MUTEX_UNLOCK_NOTIFY, COND_WAIT_REQUEST,
     * COND_SIGNAL_NOTIFY, COND_BROADCAST_NOTIFY removed - sync primitives
     * now use direct kernel notifications instead of pager IPC.
     */
};

__USER_TEXT
L4_ThreadId_t pager_create_thread(void);

__USER_TEXT
L4_Word_t pager_start_thread(L4_ThreadId_t tid,
                             void *(*thr_routine)(void *),
                             void *arg);

__USER_TEXT
void *pager_get_thread_retval(L4_ThreadId_t tid);

/* Thread lifecycle APIs */
__USER_TEXT
int pager_thread_join(L4_ThreadId_t tid, void **retval);

__USER_TEXT
int pager_thread_detach(L4_ThreadId_t tid);

/* NOTE: pager_mutex_lock/unlock, pager_cond_wait/signal/broadcast removed.
 * Sync primitives now use direct kernel notifications (L4_NotifyWait/Post)
 * instead of pager IPC round-trips for better performance.
 */

__USER_TEXT
void pager_thread(user_struct *user, void *(*entry_main)(void *user));
#endif
