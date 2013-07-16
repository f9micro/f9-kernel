/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <lib/fifo.h>

uint32_t fifo_init(struct fifo_t *queue, uint8_t *addr, uint32_t size)
{
	queue->top = 0;
	queue->end = 0;
	queue->size = size;
	queue->data = addr;

	return FIFO_OK;
}

uint32_t fifo_push(struct fifo_t *queue, uint8_t element)
{
	if (fifo_length(queue) == queue->size) {
		return FIFO_OVERFLOW;
	}

	++queue->end;

	if (queue->end == (queue->size - 1))
		queue->end = 0;

	queue->data[queue->end] = element;

	return FIFO_OK;
}

uint32_t fifo_state(struct fifo_t *queue)
{
	if (fifo_length(queue) == 0) {
		return FIFO_EMPTY;
	}

	return FIFO_OK;
}

uint32_t fifo_pop(struct fifo_t *queue, uint8_t *element)
{
	if (fifo_length(queue) == 0) {
		return FIFO_EMPTY;
	}

	++queue->top;

	if (queue->top == (queue->size - 1))
		queue->top = 0;

	*element = queue->data[queue->top];

	return FIFO_OK;
}

uint32_t fifo_length(struct fifo_t *queue)
{
	return (queue->end >= queue->top) ?
			(queue->end - queue->top) :
			(queue->size + queue->top - queue->end);
}
