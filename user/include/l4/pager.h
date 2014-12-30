/* Copyright (c) 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __PAGER_H__
#define __PAGER_H__

#define RES_FPAGE		0
#define HEAP_FPAGE		1


#define THREAD_MAX_NUM	32

/* msg tag label */
#define PAGER_REQUEST_LABEL		0xf000
#define PAGER_REPLY_LABEL		0xf001

#include <user_runtime.h>
#include <platform/link.h>

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
};

__USER_TEXT
L4_ThreadId_t pager_create_thread(void);

__USER_TEXT
L4_Word_t pager_start_thread(L4_ThreadId_t tid, void * (*thr_routine)(void *),
                             void *arg);

__USER_TEXT
void pager_thread(user_struct *user,
                  void * (*entry_main)(void *user));
#endif
