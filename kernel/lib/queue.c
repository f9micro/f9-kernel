/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <lib/queue.h>

static uint32_t queue_length(struct queue_t *queue)
{
	return (queue->end >= queue->top) ?
			(queue->end - queue->top) :
			(queue->size + queue->top - queue->end);
}

uint32_t queue_init(struct queue_t *queue, uint8_t *addr, size_t size)
{
	queue->top = 0;
	queue->end = 0;
	queue->size = size;
	queue->data = addr;

	return QUEUE_OK;
}

uint32_t queue_push(struct queue_t *queue, uint8_t element)
{
	if (queue_length(queue) == queue->size)
		return QUEUE_OVERFLOW;

	++queue->end;

	if (queue->end == (queue->size - 1))
		queue->end = 0;

	queue->data[queue->end] = element;

	return QUEUE_OK;
}

int queue_is_empty(struct queue_t *queue)
{
	return queue_length(queue) == 0;
}

uint32_t queue_pop(struct queue_t *queue, uint8_t *element)
{
	if (queue_length(queue) == 0)
		return QUEUE_EMPTY;

	++queue->top;

	if (queue->top == (queue->size - 1))
		queue->top = 0;

	*element = queue->data[queue->top];

	return QUEUE_OK;
}
