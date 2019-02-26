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

/* Changed stack size from 256 to 512. */
#define STACK_SIZE 512

enum { GPIOER_THREAD, BUTTON_MONITOR_THREAD };

static L4_ThreadId_t threads[2] __USER_DATA;

/* Remove last_thread and free_mem as they are no longer 
   required to launch and manage the threads. */
#define BOARD_LED_PORT GPIOD
#define BOARD_LED_NUM  4

#define BOARD_LED_PIN1 12 
#define BOARD_LED_PIN2 13 
#define BOARD_LED_PIN3 14 
#define BOARD_LED_PIN4 15 

static uint8_t board_leds[BOARD_LED_NUM] __USER_DATA;
static int count __USER_DATA;


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

static inline void __USER_TEXT leds_onoff(int count)
{
	for (int i = 0; i < BOARD_LED_NUM; ++i)
	{
        if ((count % 4) == i)
            gpio_out_high(BOARD_LED_PORT, board_leds[i]);
        else
            gpio_out_low(BOARD_LED_PORT, board_leds[i]);
	}
}

/* Thread function signature changed to confirm to pager_start_thread(). */
__USER_TEXT
void *gpioer_thread(void *arg)
{
	printf("gpioer thread: built-in leds blinking\n");
    led_init();
    while (1)
    {
			printf("gpioer thread: built-in leds blinking - count %d\n", count);
            leds_onoff(count++);
            L4_Sleep(L4_TimePeriod(500 * 1000));
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

/* Thread function signature changed to confirm to pager_start_thread(). */
__USER_TEXT
void *button_monitor_thread(void *arg)
{
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

/* main() signature changed. */
__USER_TEXT
static void *main(void *user)
{
	count = 0;

	threads[GPIOER_THREAD] = pager_create_thread();
	threads[BUTTON_MONITOR_THREAD] = pager_create_thread();

	pager_start_thread(threads[GPIOER_THREAD], gpioer_thread, NULL);
	pager_start_thread(threads[BUTTON_MONITOR_THREAD], button_monitor_thread, NULL);

	/* Return statement required. */
	return 0;
}

/* Function start_thread() no longer required because we are using pager_start_thread(). */

/* Function create_thread() no longer required because we are using pager_start_thread(). */

#define DEV_SIZE 0x3c00
#define AHB1_1DEV 0x40020000

DECLARE_USER(
	0,
	gpioer,
	main,
	/* was DECLARE_FPAGE(0x0, 2 * UTCB_SIZE + 2 * STACK_SIZE) */
	DECLARE_FPAGE(0x0, 6 * UTCB_SIZE + 6 * STACK_SIZE)
	/* Added this next line. */
	DECLARE_FPAGE(0x0, 512)
    /* map thread with AHB DEVICE for gpio accessing */
	DECLARE_FPAGE(AHB1_1DEV, DEV_SIZE)
);
