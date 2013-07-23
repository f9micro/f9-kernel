# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# FPU compilation flags
# FIXME: make it configurable
CONFIG_FPU=n
CFLAGS_FPU-$(CONFIG_FPU) = -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# CPU specific compilation flags
CFLAGS_CPU = -mlittle-endian -mcpu=cortex-m4
CFLAGS_CPU += -mthumb -mthumb-interwork -Xassembler -mimplicit-it=thumb
CFLAGS_CPU += -Wdouble-promotion -fsingle-precision-constant -fshort-double
CFLAGS_CPU += $(CFLAGS_FPU-y)

platform-y = \
	bitops.o \
	debug_uart.o \
	mpu.o \
	spinlock.o \
	irq.o
