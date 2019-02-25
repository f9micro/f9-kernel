/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/link.h>
#include <user_runtime.h>
#include <gpioer.h>
#include <l4/ipc.h>
#include <l4/utcb.h>
#include <l4/pager.h>
#include <l4/thread.h>
#include <l4io.h>

#define STACK_SIZE 512

enum { PING_THREAD, PONG_THREAD };

static L4_ThreadId_t threads[2] __USER_DATA;

#define LABEL 0x1

#define BOARD_LED_PORT GPIOD
#define BOARD_LED_NUM  4

#define BOARD_LED_PIN1 12 
#define BOARD_LED_PIN2 13 
#define BOARD_LED_PIN3 14 
#define BOARD_LED_PIN4 15 

static uint8_t board_leds[BOARD_LED_NUM] __USER_DATA;

__USER_TEXT
static inline void led_init(void)
{

    printf("led_init(0)\n");
    board_leds[0] = BOARD_LED_PIN1;
    board_leds[1] = BOARD_LED_PIN2;
    board_leds[2] = BOARD_LED_PIN3;
    board_leds[3] = BOARD_LED_PIN4;

    printf("led_init(1)\n");
    for (int i = 0; i < BOARD_LED_NUM; ++i)
    {
        //RCC_AHB1PeriphClockCmd(LCD_NCS_GPIO_CLK | LCD_WRX_GPIO_CLK, 1);
		printf("led_init(%d)\n", i);
        gpio_config_output(BOARD_LED_PORT,
            board_leds[i],
            GPIO_PUPDR_UP,
            GPIO_OSPEEDR_50M);
    }
    printf("led_init(9)\n");
}

__USER_TEXT
static inline void leds_onoff(bool on)
{
    for (int i = 0; i < BOARD_LED_NUM; ++i)
    {
        if (on)
            gpio_out_high(BOARD_LED_PORT, board_leds[i]);
        else
            gpio_out_low(BOARD_LED_PORT, board_leds[i]);
    }
}

#if 0
#define __L4_NUM_MRS    16
typedef unsigned long           L4_Word_t;
/*
 * Message objects
 */
typedef union {
        L4_Word_t raw[__L4_NUM_MRS];
        L4_Word_t msg[__L4_NUM_MRS];
                L4_MsgTag_t tag;
} L4_Msg_t;
#endif

__USER_TEXT
void *ping_thread(void *arg)
{
	L4_Msg_t msg;
	L4_MsgTag_t tag;

	L4_Word_t count = 0;

    bool flag = true;

    printf("ping_thread(): built-in leds blinking\n");
    led_init();

	while (1) {
		printf("ping_thread(%d)\t", count);
		L4_MsgClear(&msg);
		L4_Set_MsgLabel(&msg, LABEL);
		L4_MsgAppendWord(&msg, count++);
		L4_MsgLoad(&msg);

		tag = L4_Send_Timeout(threads[PONG_THREAD],
		                      L4_TimePeriod(1000 * 1000));

		if (!L4_IpcSucceeded(tag)) {
			printf("%p: send ipc fails\n", L4_MyGlobalId());
			printf("%p: ErrorCode = 0x%x\n", L4_MyGlobalId(), L4_ErrorCode());
		}

		leds_onoff(flag);
		L4_Sleep(L4_TimePeriod(1000 * 1000));
		flag=!flag;
	}
}

__USER_TEXT
void *pong_thread(void *arg)
{
	L4_MsgTag_t tag;
	L4_Msg_t msg;
	L4_Word_t label;
	L4_Word_t count;
	L4_Word_t u;

	while (1) {
		tag = L4_Receive_Timeout(threads[PING_THREAD],
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

__USER_TEXT
static void *main(void *user)
{
	threads[PING_THREAD] = pager_create_thread();
	threads[PONG_THREAD] = pager_create_thread();
	pager_start_thread(threads[PING_THREAD], ping_thread, NULL);
	pager_start_thread(threads[PONG_THREAD], pong_thread, NULL);
	return 0;
}

#define DEV_SIZE 0x3c00
#define AHB1_1DEV 0x40020000

DECLARE_USER(
	0,
	pingpong,
	main,
	DECLARE_FPAGE(0x0, 4 * UTCB_SIZE + 4 * STACK_SIZE)
	DECLARE_FPAGE(0x0, 512)
    DECLARE_FPAGE(AHB1_1DEV, DEV_SIZE)
);
