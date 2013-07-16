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
	uint8_t *data;		/*!< pointer to the starting position of buffer */
	uint32_t top;		/*!< queue top */
	uint32_t end;		/*!< queue end */
	size_t size;		/*!< the size of the allocated queue */
};

/**
 * fifo_init - allocates a new FIFO using a preallocated buffer
 * @param queue the preallocated buffer to be used.
 * @param addr pointer to queue data in 256-byte area
 */
uint32_t fifo_init(struct fifo_t *queue, uint8_t *addr, size_t size);

/**
 * Queue an element
 * @param queue the allocated buffer
 * @param element the value in element
 * @return FIFO_OK if succeed, otherwise FIFO_OVERFLOW if full
 */
uint32_t fifo_push(struct fifo_t *queue, uint8_t element);

/**
 * The state of FIFO
 * @return FIFO_OVERFLOW if the queue is full, FIFO_EMPTY if empty;
 * FIFO_OK for other conditions
 */
uint32_t fifo_state(struct fifo_t *queue);

/**
 * Remove the element from the queue
 * @return FIFO_OVERFLOW if the queue is full, FIFO_EMPTY if empty
 */
uint32_t fifo_pop(struct fifo_t *queue, uint8_t *element);

/**
 * Obtain the number of queued elements
 */
uint32_t fifo_length(struct fifo_t *queue);

#endif /* LIB_FIFO_H_ */
