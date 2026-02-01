/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef L4_UTCB_H_
#define L4_UTCB_H_

#include <types.h>

struct utcb {
    /* +0w */
    l4_thread_t t_globalid;
    uint32_t processor_no;
    uint32_t user_defined_handle; /* NOT used by kernel */
    l4_thread_t t_pager;
    /* +4w */
    uint32_t exception_handler;
    uint32_t flags; /* COP/PREEMPT flags (not used) */
    uint32_t xfer_timeouts;
    uint32_t error_code;
    /* +8w */
    l4_thread_t intended_receiver;
    l4_thread_t sender;
    uint32_t thread_word_1;
    uint32_t thread_word_2;
    /* +12w */
    /* Message Registers (MR) storage:
     *
     * User-space perspective (via L4_LoadMR/L4_StoreMR):
     * - MR0-MR7:   mr_low[0-7] (UTCB storage, marshaled to R4-R11 by L4_Ipc)
     * - MR8-MR39:  tcb->msg_buffer[0-31] (kernel copies to receiver)
     * - MR40-MR47: mr[0-7] (UTCB overflow)
     *
     * Kernel perspective (ctx.regs[] = saved R4-R11):
     * - On SVC entry: kernel reads R4-R11 from exception frame
     * - On SVC exit: kernel restores R4-R11 to exception frame
     * - L4_Ipc loads mr_low→R4-R11 before SVC, stores after
     *
     * This decouples MRs from physical registers, preventing corruption
     * when C functions are called between L4_LoadMR and L4_Ipc.
     */
    uint32_t mr_low[8]; /* MRs 0-7 (user-space cache, R4-R11 equivalent) */
    /* +20w */
    uint32_t mr[8]; /* MRs 40-47 (overflow beyond short buffer) */
    /* +28w */
    uint32_t br[8];
    /* +36w */
};

typedef struct utcb utcb_t;

#define UTCB_SIZE 144

#endif /* L4_UTCB_H_ */
