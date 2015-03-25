/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef IPC_H_
#define IPC_H_

#include <l4/ipc.h>
#include <types.h>

void sys_ipc(uint32_t *param1);
uint32_t ipc_deliver(void *data);

uint32_t ipc_read_mr(tcb_t *from, int i);
void ipc_write_mr(tcb_t *to, int i, uint32_t data);

#endif /* IPC_H_ */
