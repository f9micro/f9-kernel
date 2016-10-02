# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# FPU compilation flags
CFLAGS_FPU-$(CONFIG_FPU) = -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# CPU specific compilation flags
ifeq (STM32_VARIANT,f1)
CFLAGS_CPU = -mlittle-endian -mcpu=cortex-m3
else
CFLAGS_CPU = -mlittle-endian -mcpu=cortex-m4
endif
CFLAGS_CPU += -mthumb -mthumb-interwork -Xassembler -mimplicit-it=thumb
CFLAGS_CPU += -mno-sched-prolog -mno-unaligned-access
CFLAGS_CPU += -Wdouble-promotion -fsingle-precision-constant
CFLAGS_CPU += $(CFLAGS_FPU-y)

platform-y = \
	bitops.o \
	debug_device.o \
	mpu.o \
	spinlock.o \
	irq.o

platform-$(CONFIG_DEBUG_DEV_UART) += debug_uart.o
platform-$(CONFIG_DEBUG_DEV_RAM) += debug_ram.o

platform-KPROBES-$(CONFIG_KPROBES) = \
	kprobes-arch.o \
	breakpoint.o \
	breakpoint-hard.o \
	breakpoint-soft.o \
	hw_debug.o

platform-y += $(platform-KPROBES-y)

loader-platform-y = \
	irq.loader.o \
	debug_uart.loader.o \
	debug_device.loader.o
