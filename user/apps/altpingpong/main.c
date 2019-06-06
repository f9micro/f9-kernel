/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/link.h>
#include <user_runtime.h>
#include <gpioer.h>
#include <l4io.h>
#include <l4/ipc.h>
#include <l4/utcb.h>

#define STACK_SIZE 256

enum { GPIOER_THREAD, BUTTON_MONITOR_THREAD };

static L4_ThreadId_t threads[2] __USER_DATA;

static L4_Word_t last_thread __USER_DATA;
static L4_Word_t free_mem __USER_DATA;

#define LABEL 0x1

__USER_TEXT
void gpioer_thread(void)
{
	L4_Msg_t msg;
	L4_MsgTag_t tag;

	L4_Word_t count = 0;

    printf("ping_thread(): built-in leds blinking\n");
    //led_init();

	while (1) {
		printf("ping_thread(%d)\t", count);
		L4_MsgClear(&msg);
		L4_Set_MsgLabel(&msg, LABEL);
		L4_MsgAppendWord(&msg, count++);
		L4_MsgLoad(&msg);

		tag = L4_Send_Timeout(threads[BUTTON_MONITOR_THREAD],
		                      L4_TimePeriod(1000 * 1000));

		if (!L4_IpcSucceeded(tag)) {
			printf("%p: send ipc fails\n", L4_MyGlobalId());
			printf("%p: ErrorCode = 0x%x\n", L4_MyGlobalId(), L4_ErrorCode());
		}
	}
}


/* STM32F407-Discovery
 * User Button connected on PA0 
 * as result, for this demo app, 
 * Because USART4 (PA0, PA1) is conflict, 
 * choose USART1 (PA9,PA10) or USART2 (PA2,PA3) instead.
 **/

#define BUTTON_USER_PIN 0

/* if you use external button, please 
 * update the BUTTON_CUSTOM_PIN with your own number
 **/

#define BUTTON_CUSTOM_PIN BUTTON_USER_PIN

__USER_TEXT
void button_monitor_thread(void)
{
	L4_MsgTag_t tag;
	L4_Msg_t msg;
	L4_Word_t label;
	L4_Word_t count;
	L4_Word_t u;

	while (1) {
		tag = L4_Receive_Timeout(threads[GPIOER_THREAD],
		                         L4_TimePeriod(1000 * 1000));
		L4_MsgStore(tag, &msg);
		label = L4_Label(tag);
		u = L4_UntypedWords(tag);
		count = L4_MsgWord(&msg, 0);

		if (!L4_IpcSucceeded(tag)) {
			printf("%p: recv ipc fails\n", L4_MyGlobalId());
			printf("%p: ErrorCode = 0x%x\n", L4_MyGlobalId(), L4_ErrorCode());
		}
		/* FIXME: workaround solution to avoid scheduler starvation */
		L4_Sleep(L4_TimePeriod(500 * 1000));
		printf("pong_thread %d : %d : %d\n", label, u, count);
	}
}

static void __USER_TEXT start_thread(L4_ThreadId_t t, L4_Word_t ip,
									 L4_Word_t sp, L4_Word_t stack_size)
{
	L4_Msg_t msg;

	L4_MsgClear(&msg);
	L4_MsgAppendWord(&msg, ip);
	L4_MsgAppendWord(&msg, sp);
	L4_MsgAppendWord(&msg, stack_size);
	L4_MsgLoad(&msg);

	L4_Send(t);
}

static L4_ThreadId_t __USER_TEXT create_thread(user_struct *user, void (*func)(void))
{
	L4_ThreadId_t myself = L4_MyGlobalId();
	L4_ThreadId_t child;

	child.raw = myself.raw + (++last_thread << 14);

	L4_ThreadControl(child, myself, L4_nilthread, myself, (void *) free_mem);
	free_mem += UTCB_SIZE + STACK_SIZE;

	start_thread(child, (L4_Word_t)func, free_mem, STACK_SIZE);

	return child;
}

__USER_TEXT 
static void *main(void *p)
{
	user_struct *user = (user_struct *)p;
    free_mem = user->fpages[0].base;

    threads[GPIOER_THREAD] = create_thread(user, gpioer_thread);
    threads[BUTTON_MONITOR_THREAD] = create_thread(user, button_monitor_thread);

	return 0;
}

#define DEV_SIZE 0x3c00
#define AHB1_1DEV 0x40020000

DECLARE_USER(
	0,
	altpingpong,
	main,
	DECLARE_FPAGE(0x0, 2 * UTCB_SIZE + 2 * STACK_SIZE)
    /* map thread with AHB DEVICE for gpio accessing */
	DECLARE_FPAGE(AHB1_1DEV, DEV_SIZE)
);
