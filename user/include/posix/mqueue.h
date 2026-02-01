/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __POSIX_MQUEUE_H__
#define __POSIX_MQUEUE_H__

#include <posix/sys/types.h>
#include <posix/time.h>

/* POSIX 1003.1b-93 Message Passing (Message Queues) */

/* Message queue descriptor */
typedef struct {
    int mqd;           /* Queue descriptor */
    uint32_t flags;    /* O_RDONLY, O_WRONLY, O_RDWR */
    uint32_t max_msgs; /* Maximum messages in queue */
    uint32_t msg_size; /* Maximum message size */
    uint32_t cur_msgs; /* Current messages in queue */
} mqd_t;

/* Message queue attributes */
struct mq_attr {
    uint32_t mq_flags;   /* Message queue flags (O_NONBLOCK) */
    uint32_t mq_maxmsg;  /* Maximum number of messages */
    uint32_t mq_msgsize; /* Maximum message size */
    uint32_t mq_curmsgs; /* Current number of messages */
};

/* Open/close flags */
#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR 0x0002
#define O_CREAT 0x0100
#define O_EXCL 0x0200
#define O_NONBLOCK 0x0800

/* Message queue functions (1003.1b-93) */
mqd_t mq_open(const char *name, int oflag, ...);
int mq_close(mqd_t mqdes);
int mq_unlink(const char *name);

int mq_send(mqd_t mqdes,
            const char *msg_ptr,
            size_t msg_len,
            unsigned int msg_prio);
ssize_t mq_receive(mqd_t mqdes,
                   char *msg_ptr,
                   size_t msg_len,
                   unsigned int *msg_prio);

int mq_timedsend(mqd_t mqdes,
                 const char *msg_ptr,
                 size_t msg_len,
                 unsigned int msg_prio,
                 const struct timespec *abs_timeout);
ssize_t mq_timedreceive(mqd_t mqdes,
                        char *msg_ptr,
                        size_t msg_len,
                        unsigned int *msg_prio,
                        const struct timespec *abs_timeout);

int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat);
int mq_setattr(mqd_t mqdes,
               const struct mq_attr *mqstat,
               struct mq_attr *omqstat);

int mq_notify(mqd_t mqdes, const struct sigevent *notification);

/* Additional type definitions */
typedef long ssize_t;
typedef unsigned long size_t;

#endif /* __POSIX_MQUEUE_H__ */
