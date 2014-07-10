# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

chip-y = \
	lcd.o \
	spi.o \
	sdram.o \
	fmc.o \
	ltdc.o \
	dma2d.o \
	i2c.o \
	ioe.o 

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
		-f target/stm32f4x_stlink.cfg \
		-c "init" \
		-c "reset init" \
		-c "flash probe 0" \
		-c "flash info 0" \
		-c "flash write_image erase $(1) 0x08000000" \
		-c "reset run" -c "shutdown"
