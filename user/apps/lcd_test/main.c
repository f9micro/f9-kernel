/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/link.h>
#include <user_runtime.h>
#include <l4/ipc.h>
#include <l4/utcb.h>
#include <l4/pager.h>
#include <l4io.h>
#include INC_PLAT(lcd.h)
#include INC_PLAT(ltdc.h)

#define STACK_SIZE 512
__USER_TEXT
static void *main(void *arg)
{

	printf("lcd main\n");
	/* testing lcd */
	lcd_init();
	lcd_layer_init();
	ltdc_layer_cmd(LTDC_Layer1, 1);
	ltdc_layer_cmd(LTDC_Layer2, 1);
	ltdc_reload_config(LTDC_IMReload);
	ltdc_cmd(1);
	lcd_set_layer(LCD_FOREGROUND_LAYER);
	lcd_clear(LCD_COLOR_WHITE);
	
	lcd_set_back_color(0xCE79);
	lcd_set_text_color(LCD_COLOR_BLACK);
	lcd_draw_rect(20, 30, 20, 40);

	while (1)
		L4_Sleep(L4_Never);
}

DECLARE_USER(
	4,
	lcd_test,
	main,
	DECLARE_FPAGE(0x0, 2 * (UTCB_SIZE + STACK_SIZE))
	DECLARE_FPAGE(0x0, 512)
	DECLARE_FPAGE(0x40005000, 0x1000)
	DECLARE_FPAGE(0x40015000, 0x0c00)
	DECLARE_FPAGE(0x40020000, 0x3c00)
	DECLARE_FPAGE(0x40028000, 0x8000)
	DECLARE_FPAGE(0x42470000, 0x0c00)
	DECLARE_FPAGE(0x40016800, 0x0c00)
	DECLARE_FPAGE(0xA0000000, 0x1000)
	DECLARE_FPAGE(0xD0000000, 0xA0000)
	DECLARE_FPAGE(0xD00A0000, 0xA0000)
);
