/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef LIB_FIFO_H_
#define LIB_FIFO_H_

#include <types.h>

#define FIFO_OK		0x0
#define	FIFO_OVERFLOW	0x1
#define	FIFO_EMPTY	0x2

struct fifo_t {
	uint8_t *q_data;
	uint32_t q_top;
	uint32_t q_end;
	size_t   q_size;
};

uint32_t fifo_init(struct fifo_t *queue, uint8_t *addr, size_t size);

uint32_t fifo_push(struct fifo_t *queue, uint8_t el);

uint32_t fifo_state(struct fifo_t *queue);

uint32_t fifo_pop(struct fifo_t *queue, uint8_t *el);

uint32_t fifo_length(struct fifo_t *queue);

#endif /* LIB_FIFO_H_ */
