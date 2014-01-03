# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EXEC_FORMAT = elf32-littlearm

loader-objs := $(loader-all-y)
kernel-obj := $(out)/kernel.loader.o
CFLAGS_INCLUDE_LOADER = -I loader/include

KERNEL_OBJ_LDS = kernel.bin.lds
KLDFLAGS = \
	--oformat $(EXEC_FORMAT) \
	-r \
	-b binary

cmd_kernel_to_o = $(LD) -L loader -T $(KERNEL_OBJ_LDS) $(KLDFLAGS) $< -o $@
cmd_c_to_o_loader = $(CC) $(CFLAGS_INCLUDE_LOADER) -DLOADER $(CFLAGS) -MMD -MF $@.d -c $< -o $@
cmd_loader_elf = $(LD) --oformat $(EXEC_FORMAT) $(loader-objs) $(kernel-obj) -o $@ -L loader -T loader.ld $(LIBGCC)

.PHONY: loader
loader: $(out)/loader.bin
	mv $(out)/loader.bin $(out)/$(PROJECT).bin

$(out)/loader.bin: $(out)/loader.elf
	$(call quiet,obj_to_bin,OBJCOPY)

$(out)/loader.elf: $(kernel-obj) $(loader-objs)
	$(call quiet,loader_elf,LD     )

$(out)/kernel.loader.o: $(out)/kernel_strip.elf
	$(call quiet,kernel_to_o,LD     )

$(out)/kernel_strip.elf: $(out)/$(PROJECT).elf
	$(call quiet,strip,STRIP  )

$(out)/%.loader.o: %.c
	$(call quiet,c_to_o_loader,CC     )
