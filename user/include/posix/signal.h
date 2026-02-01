/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __POSIX_SIGNAL_H__
#define __POSIX_SIGNAL_H__

#include <posix/sys/types.h>

/* POSIX signals (PSE51 minimal set) */
#define SIGTERM 15 /* Termination request */
#define SIGKILL 9  /* Kill (cannot be caught or ignored) */
#define SIGSTOP 19 /* Stop (cannot be caught or ignored) */
#define SIGUSR1 10 /* User-defined signal 1 */
#define SIGUSR2 12 /* User-defined signal 2 */
#define SIGALRM 14 /* Alarm clock */
#define SIGINT 2   /* Interrupt */

/* Signal set type */
typedef uint32_t sigset_t;

/* Signal action structure */
struct sigaction {
    void (*sa_handler)(int); /* Signal handler function */
    sigset_t sa_mask;        /* Signals to block during handler */
    int sa_flags;            /* Flags */
};

/* Signal action flags */
#define SA_NOCLDSTOP 0x0001
#define SA_SIGINFO 0x0004
#define SA_RESTART 0x0010

/* Special handler values */
#define SIG_DFL ((void (*)(int)) 0)   /* Default action */
#define SIG_IGN ((void (*)(int)) 1)   /* Ignore signal */
#define SIG_ERR ((void (*)(int)) - 1) /* Error return */

/* Signal set operations */
int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int signo);
int sigdelset(sigset_t *set, int signo);
int sigismember(const sigset_t *set, int signo);

/* Signal mask operations */
#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

int pthread_sigmask(int how, const sigset_t *set, sigset_t *oset);
int sigprocmask(int how, const sigset_t *set, sigset_t *oset);

/* Signal action */
int sigaction(int sig, const struct sigaction *act, struct sigaction *oact);

/* Wait for signals */
int sigwait(const sigset_t *set, int *sig);
int sigpending(sigset_t *set);

/* Send signals */
int pthread_kill(pthread_t *thread, int sig);
int raise(int sig);

#endif /* __POSIX_SIGNAL_H__ */
