/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef LIB_QUEUE_H_
#define LIB_QUEUE_H_

#include <types.h>

#define QUEUE_OK		0x0
#define	QUEUE_OVERFLOW		0x1
#define	QUEUE_EMPTY		0x2

/**
 * Queue control block
 */
struct queue_t {
	uint8_t *data;		/*!< pointer to the starting position of buffer */
	uint32_t top;		/*!< queue top */
	uint32_t end;		/*!< queue end */
	size_t size;		/*!< the size of the allocated queue */
};

/**
 * Allocate a new queue using a preallocated buffer
 * @param queue the preallocated buffer to be used.
 * @param addr pointer to queue data in 256-byte area
 */
uint32_t queue_init(struct queue_t *queue, uint8_t *addr, size_t size);

/**
 * Push an element to the queue
 * @param queue the allocated buffer
 * @param element the value in element
 * @return QUEUE_OK if succeed, otherwise QUEUE_OVERFLOW if full
 */
uint32_t queue_push(struct queue_t *queue, uint8_t element);

/**
 * Pop the element from the queue
 * @return QUEUE_OVERFLOW if the queue is full, QUEUE_EMPTY if empty
 */
uint32_t queue_pop(struct queue_t *queue, uint8_t *element);

/**
 * Check if the queue is empty.
 * @param queue the queue control block.
 * @return 0 if the queue is not empty.
 */
int queue_is_empty(struct queue_t *queue);

/**
 * Check if the queue is full.
 * @param queue the queue control block.
 * @return 0 if the queue is not full.
 */
int queue_is_full(struct queue_t *queue);

#endif /* LIB_QUEUE_H_ */
