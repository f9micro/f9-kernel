# Copyright (c) 2014 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

platform-common-y = \
	gpio-$(STM32_VARIANT).o \
	rcc.o \
	mpu.o \
	nvic.o \
	systick.o \
	hwtimer.o \
	usart.o
