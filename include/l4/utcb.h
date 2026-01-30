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
    /* Message Registers (MR) mapping with short message buffer:
     * MR0-MR7:   Hardware registers R4-R11 (ctx.regs[0-7]) - 32 bytes
     * MR8-MR39:  Short message buffer (tcb->msg_buffer[0-31]) - 128 bytes
     * MR40-MR47: UTCB overflow (mr[0-7]) - 32 bytes
     *
     * Total message capacity: 192 bytes (48 words)
     * Fastpath capacity: 160 bytes (40 words, MR0-MR39)
     */
    uint32_t mr[8]; /* MRs 40-47 (overflow beyond short buffer) */
                    /* +20w */
    uint32_t br[8];
    /* +28w */
    uint32_t reserved[4];
    /* +32w */
};

typedef struct utcb utcb_t;

#define UTCB_SIZE 128

#endif /* L4_UTCB_H_ */
