# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

objs := $(all-y)
deps := $(objs:%.o=%.o.d)
build-utils := $(foreach u,$(build-util-bin),$(out)/util/$(u))
host-utils := $(foreach u,$(host-util-bin),$(out)/util/$(u))

# Create output directories if necessary
_dir_create := $(foreach d,$(dirs),$(shell [ -d $(out)/$(d) ] || \
	    mkdir -p $(out)/$(d)))
_dir_y_create := $(foreach d,$(dirs-y),$(shell [ -d $(out)/$(d) ] || \
	    mkdir -p $(out)/$(d)))

# Decrease verbosity unless you pass V=1
quiet = $(if $(V),,@echo '  $(2)' $(subst $(out)/,,$@) ; )$(cmd_$(1))
silent = $(if $(V),,1>/dev/null)

# commands to build all targets
cmd_obj_to_bin = $(OBJCOPY) -O binary $^ $(out)/$*.bin
cmd_elf_to_list = $(OBJDUMP) -S $< > $@
cmd_elf = $(LD) $(LDFLAGS) $(objs) -o $@ -T f9.ld \
	`$(CROSS_COMPILE)gcc -print-libgcc-file-name` \
	-Map $(out)/$*.map
cmd_c_to_o = $(CC) $(CFLAGS) -MMD -MF $@.d -c $< -o $@
cmd_c_to_build = $(BUILDCC) $(BUILD_CFLAGS) $(BUILD_LDFLAGS) \
	         -MMD -MF $@.d $< -o $@

.PHONY: all
all: $(out)/$(PROJECT).bin

$(out)/%.bin: $(out)/%.elf
	$(call quiet,obj_to_bin,OBJCOPY)

$(out)/%.list: $(out)/%.elf
	$(call quiet,elf_to_list,OBJDUMP)

$(out)/%.elf: $(objs)
	$(call quiet,elf,LD     )

$(out)/%.o:%.c
	$(call quiet,c_to_o,CC     )

$(out)/%.o:%.S
	$(call quiet,c_to_o,AS     )

$(build-utils): $(out)/%:%.c
	$(call quiet,c_to_build,BUILDCC)

.PHONY: flash
flash: $(out)/$(PROJECT).bin
	$(call platform-flash-command, $<)

.PHONY: clean
clean:
	-rm -rf $(out)

.SECONDARY:

-include $(deps)
