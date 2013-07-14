# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

########################################################################

LINK_SCRIPT = f9.ld

KERNEL_SRC = kernel
PLATFORM_SRC = platform
USER = user

# kernel/
VPATH = $(KERNEL_SRC)/
SRCS = debug.c error.c fpage.c init.c ipc.c kdb.c kip.c ktimer.c memory.c \
	sched.c softirq.c start.c syscall.c systhread.c thread.c

# kernel/lib/
VPATH += $(KERNEL_SRC)/lib/
SRCS += fifo.c ktable.c stdio.c

# platform
PLATFORM_INC = include/platform
PLATFORM ?= stm32f4
PLATFORM_TARGET_INC = $(PLATFORM_INC)/$(PLATFORM)
PLATFORM_BOARD ?= discoveryf4
include $(PLATFORM_SRC)/$(PLATFORM)/platform.mk

# platform/
VPATH += $(PLATFORM_SRC)/
SRCS += debug_uart.c bitops.c spinlock.c mpu.c

# user/
VPATH += $(USER)/
SRCS += root_thread.c

# all the files will be generated with this name (main.elf, main.bin, main.hex, etc)
PROJ_NAME=f9
PREFIX = ./out

# cross-compiler prefix
CROSS_COMPILE := arm-none-eabi-

# no need to change anything below this line
############################################

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

CFLAGS += -g3 -Wall -std=gnu99 -isystem
CFLAGS += -mlittle-endian -mcpu=cortex-m4
CFLAGS += -mthumb -mthumb-interwork -Xassembler -mimplicit-it=thumb
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16 -nostdlib -ffreestanding
CFLAGS += -Wdouble-promotion -fsingle-precision-constant -fshort-double

CFLAGS += -DDEBUG
CFLAGS += -I include/
CFLAGS += -D __PLATFORM__=$(PLATFORM)
CFLAGS += -D __BOARD_HEADER__="$(PLATFORM_BOARD).h"

DATE := "$(shell LC_ALL=C date -u)"
CFLAGS += -D BUILD_TIME='$(DATE)'

#CFLAGS += -save-temps --verbose -Xlinker --verbose

LFLAGS=

###################################################

OBJS = $(addprefix $(PREFIX)/, $(SRCS:.c=.o))
OBJS += $(addprefix $(PREFIX)/, $(ASM_SRCS:.S=.o))

###################################################

# verbose mode
ifeq ("$(origin V)", "command line")
  VERBOSE = $(V)
endif
ifndef VERBOSE
  VERBOSE = 0
endif
export VERBOSE

ifeq ($(VERBOSE),1)
  Q =
  print_remove =
  print_assemble =
  print_compile =
  print_link =
else
  Q = @
  print_remove =	echo '  RM	'$1;
  print_assemble =	echo '  AS	'$<;
  print_compile =	echo '  CC	'$<;
  print_link =		echo '  LINK	'$@;
endif

.PHONY: proj

all: proj

unit-tests:
	UNIT_TESTS=1 $(MAKE) -e

again: clean all

# Flash the target
flash: proj flash_command
flash_command: $(PREFIX)/$(PROJ_NAME).bin
	$(call platform-flash-command, $<)

# Create tags
ctags:
	ctags -R .

do_remove = \
	($(print_remove) \
	 rm -rf $1)

do_depend = \
	(cp $(PREFIX)/$*.d $(PREFIX)/$*.P; \
		sed -e 's/\#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
			-e '/^$$/ d' -e 's/$$/ :/' < $(PREFIX)/$*.d >> $(PREFIX)/$*.P; \
		rm -f $(PREFIX)/$*.d)

do_assemble = \
	($(print_assemble) \
	 $(CC) -MD -c $(CFLAGS) $< -o $@)

$(PREFIX)/%.o : %.S
	$(Q)$(call do_assemble)
	$(Q)$(call do_depend)

-include $(addprefix $(PREFIX)/, $(ASM_SRCS:.S=.P))

do_compile = \
	($(print_compile) \
	 $(CC) -MD -c $(CFLAGS) $< -o $@)

$(PREFIX)/%.o : %.c
	$(Q)$(call do_compile)
	$(Q)$(call do_depend)

-include $(addprefix $(PREFIX)/, $(SRCS:.c=.P))

proj: $(PREFIX) $(PREFIX)/$(PROJ_NAME).elf

$(PREFIX):
	mkdir -p $(PREFIX)

do_link = \
	($(print_link) \
	 $(LD) $^ -o $@ $(LFLAGS) -T $(LINK_SCRIPT) \
		`$(CROSS_COMPILE)gcc -print-libgcc-file-name`)

$(PREFIX)/$(PROJ_NAME).elf: $(OBJS)
	$(Q)$(call do_link)
	$(OBJCOPY) -O ihex $(PREFIX)/$(PROJ_NAME).elf $(PREFIX)/$(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PREFIX)/$(PROJ_NAME).elf $(PREFIX)/$(PROJ_NAME).bin
	$(OBJDUMP) -S $(PREFIX)/$(PROJ_NAME).elf > $(PREFIX)/$(PROJ_NAME).list

clean:
	$(Q)$(call do_remove, $(PREFIX))
