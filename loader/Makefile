# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

########################################################################

LINK_SCRIPT = loader.ld
CHIP := stm32f4

PROJECT_ROOT = ..
PLATFORM_SRC = $(PROJECT_ROOT)/platform
LOADER_SRC = $(PROJECT_ROOT)/loader
KERNEL_SRC = $(PROJECT_ROOT)/kernel


# platform
PLATFORM ?= stm32f4
PLATFORM_BOARD ?= discoveryf4


KERNEL_IMG_PATH = $(PROJECT_ROOT)/build/$(PLATFORM_BOARD)
KERNEL_IMG = f9.elf

# kernel lib
VPATH += $(KERNEL_SRC)/lib
SRCS += queue.c memcpy.c memset.c strcmp.c stdio.c

#board
BOARD_SRC = $(PROJECT_ROOT)/board/$(PLATFORM_BOARD)

# platform
VPATH += $(PLATFORM_SRC)
SRCS += irq.c debug_device.c

# platform/<CHIP>/
VPATH += $(PLATFORM_SRC)/$(CHIP)/
SRCS += gpio.c nvic.c rcc.c usart.c systick.c

# kernel
VPATH += $(KERNEL_SRC)/
SRCS += error.c init.c debug_uart.c debug.c

# loader source
VPATH += $(LOADER_SRC)/
SRCS += loader.c

# elf lib
VPATH += $(LOADER_SRC)/elf
SRCS += elf32.c elf64.c elf.c

# board/
VPATH += $(BOARD_SRC)/
SRCS += board.c

# all the files will be generated with this name (main.elf, main.bin, main.hex, etc)
PROJ_NAME=f9-loader
PREFIX = ./out

# cross-compiler prefix
CROSS_COMPILE := arm-none-eabi-

# no need to change anything below this line
############################################

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
STRIP = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

CFLAGS += -g3 -Wall -std=gnu99 -isystem
CFLAGS += -mlittle-endian -mcpu=cortex-m4
CFLAGS += -mthumb -mthumb-interwork -Xassembler -mimplicit-it=thumb
CFLAGS += -nostdlib -ffreestanding -fdata-sections -ffunction-sections
CFLAGS += -Wdouble-promotion -fsingle-precision-constant -fshort-double


CFLAGS += -DDEBUG
CFLAGS += -I $(PROJECT_ROOT)/include/ -I $(PROJECT_ROOT)/board/discoveryf4/ -I $(PROJECT_ROOT)/loader/include
CFLAGS += -D __PLATFORM__=$(PLATFORM)
CFLAGS += -D __BOARD_HEADER__="$(PLATFORM_BOARD).h"
CFLAGS += \
	-D __CHIP__=$(CHIP) \
	-D'INC_PLAT(x)=<platform/__CHIP__/x>' \
	-D LOADER
	
DATE := "$(shell LC_ALL=C date -u)"
CFLAGS += -D BUILD_TIME='$(DATE)'
CFLAGS += -include $(PROJECT_ROOT)/include/autoconf.h

#CFLAGS += -save-temps --verbose -Xlinker --verbose

LFLAGS= --oformat elf32-littlearm

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
	st-flash write $< 0x8000000

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
	 $(LD) $^ $(PREFIX)/kernel.o -o $@ $(LFLAGS) -T $(LINK_SCRIPT) -Map $@.map\
		`$(CROSS_COMPILE)gcc -print-libgcc-file-name`)


#
$(PREFIX)/$(PROJ_NAME).elf: $(OBJS)
	$(shell cp -f $(KERNEL_IMG_PATH)/$(KERNEL_IMG) $(PREFIX))
	$(STRIP) $(PREFIX)/$(KERNEL_IMG)
	$(LD) -T kernel.bin.lds --oformat elf32-littlearm -r -b binary $(PREFIX)/$(KERNEL_IMG) -o $(PREFIX)/kernel.o
	$(Q)$(call do_link)
	$(OBJCOPY) -O ihex $(PREFIX)/$(PROJ_NAME).elf $(PREFIX)/$(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PREFIX)/$(PROJ_NAME).elf $(PREFIX)/$(PROJ_NAME).bin
	$(OBJDUMP) -S $(PREFIX)/$(PROJ_NAME).elf > $(PREFIX)/$(PROJ_NAME).list

clean:
	$(Q)$(call do_remove, $(PREFIX))
