# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

chip-y = \
	gpio.o \
	rcc.o \
	usart.o \
	nvic.o \
	systick.o \
	hwtimer.o

# FIXME: move to toplevel definitions
define platform-flash-command
	st-flash write $1 0x8000000
endef
