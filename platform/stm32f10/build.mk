# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

chip-y = \
	gpio.o \
	rcc.o \
	usart.o \
	nvic.o \
	systick.o \
	hwtimer.o \
	mpu.o

loader-chip-y = \
	gpio.loader.o \
	rcc.loader.o \
	usart.loader.o \
	nvic.loader.o \
	systick.loader.o

# FIXME: move to toplevel definitions
platform-flash-command = \
	st-flash write $(1) 0x8000000 || \
	openocd -f interface/stlink-v2.cfg \
		-f target/stm32f10x_stlink.cfg \
		-c "init" \
		-c "reset init" \
		-c "flash probe 0" \
		-c "flash info 0" \
		-c "flash write_image erase $(1) 0x08000000" \
		-c "reset run" -c "shutdown"
