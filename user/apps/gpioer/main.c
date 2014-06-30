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

#define BOARD_LED_PORT GPIOD
#define BOARD_LED_NUM  4

#define BOARD_LED_PIN1 12 
#define BOARD_LED_PIN2 13 
#define BOARD_LED_PIN3 14 
#define BOARD_LED_PIN4 15 

static uint8_t board_leds[BOARD_LED_NUM] __USER_DATA;

static inline void __USER_TEXT led_init(void)
{

	board_leds[0] = BOARD_LED_PIN1;
	board_leds[1] = BOARD_LED_PIN2;
	board_leds[2] = BOARD_LED_PIN3;
	board_leds[3] = BOARD_LED_PIN4;

	for (int i = 0; i < BOARD_LED_NUM; ++i)
	{
		gpio_config_output(BOARD_LED_PORT,
			board_leds[i],
			GPIO_PUPDR_UP,
			GPIO_OSPEEDR_50M);
	}
}

static inline void __USER_TEXT leds_onoff(bool on)
{
	for (int i = 0; i < BOARD_LED_NUM; ++i)
	{
		if (on)
            gpio_out_high(BOARD_LED_PORT, board_leds[i]);
        else
            gpio_out_low(BOARD_LED_PORT, board_leds[i]);
	}
}


void __USER_TEXT gpioer_thread(void)
{
	bool flag = true;

	printf("gpioer thread: built-in leds blinking\n");
    led_init();
    while (1)
    {
            leds_onoff(flag);
            L4_Sleep(L4_TimePeriod(1000 * 1000));
            flag=!flag;
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

void __USER_TEXT button_monitor_thread(void)
{
    int count = 1;

    gpio_config_input(GPIOA, BUTTON_CUSTOM_PIN, GPIO_PUPDR_DOWN);
	printf("thread: built-in user button detection\n");
    while (1)
    {
            uint8_t state = gpio_input_bit(GPIOA, BUTTON_CUSTOM_PIN);
            if (state != 0) {
                printf("button %s %d times\n", state == 0 ? "open" : "pushed", count);
                count++;
            }
            L4_Sleep(L4_TimePeriod(1000 * 200));
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

static void __USER_TEXT main(user_struct *user)
{
	free_mem = user->fpages[0].base;

	threads[GPIOER_THREAD] = create_thread(user, gpioer_thread);
	threads[BUTTON_MONITOR_THREAD] = create_thread(user, button_monitor_thread);
}

#define DEV_SIZE 0x3c00
#define AHB1_1DEV 0x40020000

DECLARE_USER(
	0,
	gpioer,
	main,
	DECLARE_FPAGE(0x0, 2 * UTCB_SIZE + 2 * STACK_SIZE)
    /* map thread with AHB DEVICE for gpio accessing */
	DECLARE_FPAGE(AHB1_1DEV, DEV_SIZE)
);
