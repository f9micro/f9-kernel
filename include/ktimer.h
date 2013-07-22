/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef KTIMER_H_
#define KTIMER_H_

void ktimer_handler(void);

/* Returns 0 if successfully handled
 * or number ticks if need to be rescheduled
 */
typedef uint32_t (*ktimer_event_handler_t)(void *data);


typedef struct ktimer_event {
	struct ktimer_event *next;
	ktimer_event_handler_t handler;

	uint32_t delta;
	void *data;
} ktimer_event_t;

void ktimer_event_init(void);

int ktimer_event_schedule(uint32_t ticks, ktimer_event_t *kte);
int ktimer_event_create(uint32_t ticks, ktimer_event_handler_t handler, void *data);
void ktimer_event_handler(void);

#ifdef CONFIG_KTIMER_TICKLESS
void ktimer_enter_tickless();
#endif /* CONFIG_KTIMER_TICKLESS */

#endif /* KTIMER_H_ */
