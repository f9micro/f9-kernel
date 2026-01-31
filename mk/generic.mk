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

symmap_obj-list = $(symmap_obj-list-y)

# Decrease verbosity unless you pass V=1
quiet = $(if $(V),,@echo '  $(2)' $(subst $(out)/,,$@) ; )$(cmd_$(1))
silent = $(if $(V),,1>/dev/null)

# commands to build all targets
cmd_obj_to_bin = $(OBJCOPY) -O binary $< $@
cmd_elf_to_list = $(OBJDUMP) -S $< > $@
cmd_elf = $(LD) $(LDFLAGS) $(objs) -o $@ \
	-L platform -T $(CHIP)/$(F9_LD_FILE) $(LIBGCC)
cmd_strip = $(STRIP) $< -o $@
cmd_elf_relink = $(LD) $(LDFLAGS) $(objs) $(symmap_obj-list) -o $@ \
	-L platform -T $(CHIP)/$(F9_LD_FILE) $(LIBGCC) \
	-Map $(out)/$*.map
cmd_c_to_o = $(CC) $(CFLAGS) -MMD -MF $@.d -c $< -o $@
cmd_c_to_o_user = $(CC) $(CFLAGS_INCLUDE_USER) $(CFLAGS) $(CFLAGS_USER) -MMD -MF $@.d -c $< -o $@
cmd_c_to_build = $(BUILDCC) $(BUILD_CFLAGS) $(BUILD_LDFLAGS) \
	         -MMD -MF $@.d $< -o $@
cmd_bin = cat $^ > $@

# Kconfiglib (Python-based Kconfig tools)
KCONFIG_DIR := tools/kconfig
KCONFIG := Kconfig
KCONFIGLIB_REPO := https://github.com/sysprog21/Kconfiglib

# Discover available boards (directories with defconfig)
BOARDS := $(notdir $(patsubst %/defconfig,%,$(wildcard board/*/defconfig)))

# Kconfiglib tool paths
MENUCONFIG := $(KCONFIG_DIR)/menuconfig.py
DEFCONFIG := $(KCONFIG_DIR)/defconfig.py
GENCONFIG := $(KCONFIG_DIR)/genconfig.py
OLDCONFIG := $(KCONFIG_DIR)/oldconfig.py
SAVEDEFCONFIG := $(KCONFIG_DIR)/savedefconfig.py

# Kconfiglib commands
cmd_menuconfig = KCONFIG_CONFIG=$(CONFIG) $(PYTHON) $(MENUCONFIG) $(KCONFIG)
cmd_genconfig = KCONFIG_CONFIG=$(CONFIG) $(PYTHON) $(GENCONFIG) --header-path include/autoconf.h $(KCONFIG)
cmd_board_defconfig = KCONFIG_CONFIG=$(CONFIG) $(PYTHON) $(DEFCONFIG) --kconfig $(KCONFIG) board/$*/defconfig
cmd_oldconfig = KCONFIG_CONFIG=$(CONFIG) $(PYTHON) $(OLDCONFIG) $(KCONFIG)
cmd_savedefconfig = KCONFIG_CONFIG=$(CONFIG) $(PYTHON) $(SAVEDEFCONFIG) --kconfig $(KCONFIG) --out board/$(BOARD)/defconfig

# Auto-regenerate autoconf.h when .config changes
# Error handling: Remove partial file on failure to force retry on next build
include/autoconf.h: $(CONFIG) $(GENCONFIG)
	@echo "  GENCONFIG include/autoconf.h (config changed)"
	@$(cmd_genconfig) || { rm -f include/autoconf.h; \
		echo "ERROR: Failed to generate autoconf.h"; false; }

.PHONY: bare
bare: $(out)/$(PROJECT).bin

$(out)/%.bin: $(out)/%.elf.bin $(bin-list)
	$(call quiet,bin,CAT    )

$(out)/%.elf.bin: $(out)/%.elf
	$(call quiet,obj_to_bin,OBJCOPY)

$(out)/%.list: $(out)/%.elf
	$(call quiet,elf_to_list,OBJDUMP)

$(out)/%.elf: $(out)/f9_nosym.elf $(symmap_obj-list)
	$(call quiet,elf_relink,LD     )

# Build depends on valid autoconf.h
$(out)/f9_nosym.elf: $(objs) include/autoconf.h
	$(call quiet,elf,LD     )

# All compilation depends on autoconf.h being up-to-date
$(out)/user/%.o:user/%.c include/autoconf.h
	$(call quiet,c_to_o_user,CC     )

$(out)/%.o:%.c include/autoconf.h
	$(call quiet,c_to_o,CC     )

$(out)/%.o:%.S include/autoconf.h
	$(call quiet,c_to_o,AS     )

$(build-utils): $(out)/%:%.c
	$(call quiet,c_to_build,BUILDCC)

.PHONY: flash
flash: $(out)/$(PROJECT).bin
	$(call platform-flash-command, $<)

.PHONY: clean
clean:
	-rm -rf $(out)

.PHONY: distclean
distclean: clean
	-rm -rf $(out_host) $(KCONFIG_DIR)
	-rm -f $(CONFIG) $(CONFIG).old include/autoconf.h

# QEMU emulation for netduinoplus2
.PHONY: qemu
qemu: $(out)/$(PROJECT).bin
	-killall -q qemu-system-arm
	$(QEMU) -M netduinoplus2 -nographic -kernel $(out)/$(PROJECT).elf -serial mon:stdio

# QEMU with GDB debugging
# Terminal 1: make qemu-gdb
# Terminal 2: make gdb-attach
.PHONY: qemu-gdb
qemu-gdb: $(out)/$(PROJECT).bin
	@echo "Starting QEMU with GDB server on port 1234..."
	@echo "In another terminal, run: make gdb-attach"
	@echo "Press Ctrl+C to exit"
	-killall -q qemu-system-arm
	$(QEMU) -M netduinoplus2 -nographic -kernel $(out)/$(PROJECT).elf -s -S

.PHONY: gdb-attach
gdb-attach: $(out)/$(PROJECT).elf
	@echo "Connecting to QEMU GDB server..."
	arm-none-eabi-gdb $(out)/$(PROJECT).elf \
		-ex "target remote :1234" \
		-ex "layout src" \
		-ex "layout regs"

# QEMU automated testing
# Usage: make run-tests              (test suite)
#        make run-tests FAULT=mpu    (MPU fault test)
#        make run-tests FAULT=canary (stack canary test)
.PHONY: run-tests
run-tests:
ifeq ($(FAULT),mpu)
	@echo "Building with FAULT_TYPE=1 (MPU)..."
	@$(MAKE) clean $(silent)
	@$(MAKE) FAULT_TYPE=1 $(out)/$(PROJECT).elf $(silent)
	@echo "Running MPU fault test under QEMU..."
	@python3 scripts/qemu-test.py $(out)/$(PROJECT).elf --fault -t 30
else ifeq ($(FAULT),canary)
	@echo "Building with FAULT_TYPE=2 (canary)..."
	@$(MAKE) clean $(silent)
	@$(MAKE) FAULT_TYPE=2 $(out)/$(PROJECT).elf $(silent)
	@echo "Running stack canary fault test under QEMU..."
	@python3 scripts/qemu-test.py $(out)/$(PROJECT).elf --fault -t 30
else
	@echo "Running test suite under QEMU..."
	@$(MAKE) $(out)/$(PROJECT).elf $(silent)
	@python3 scripts/qemu-test.py $(out)/$(PROJECT).elf -t 45
endif

# Compile-only build for hardware code paths (catches syntax errors)
# This builds for STM32F4 Discovery (no CONFIG_QEMU), forcing compilation of
# all hardware-specific test code that normally skips in QEMU builds.
# Does NOT run tests - just ensures hardware code compiles.
.PHONY: compile-hw-tests
compile-hw-tests:
	@echo "Compile-only check for hardware test paths..."
	@if [ ! -f .config ]; then \
		echo "Error: No .config found. Run 'make config' first."; \
		exit 1; \
	fi
	@echo "  Saving current config..."
	@cp .config .config.bak
	@echo "  Switching to discoveryf4 (hardware target)..."
	@$(MAKE) discoveryf4_defconfig $(silent)
	@echo "  Building hardware test code..."
	@$(MAKE) $(out)/$(PROJECT).elf $(silent)
	@echo "  Restoring original config..."
	@mv .config.bak .config
	@echo "  Cleaning build artifacts..."
	@$(MAKE) clean $(silent)
	@echo "✓ Hardware test code compiles successfully"

# Kconfiglib download target
$(KCONFIG_DIR)/kconfiglib.py:
	@echo "  CLONE   Kconfiglib"
	@git clone --depth=1 $(KCONFIGLIB_REPO) $(KCONFIG_DIR)

$(MENUCONFIG) $(DEFCONFIG) $(GENCONFIG) $(OLDCONFIG) $(SAVEDEFCONFIG): $(KCONFIG_DIR)/kconfiglib.py

.PHONY: config menuconfig
config menuconfig: $(MENUCONFIG)
	$(call quiet,menuconfig,MENUCONFIG)
	$(call quiet,genconfig,GENCONFIG)

# Board-specific defconfig targets (e.g., make discoveryf4_defconfig)
.PHONY: $(addsuffix _defconfig,$(BOARDS))
$(addsuffix _defconfig,$(BOARDS)): %_defconfig: $(DEFCONFIG) $(GENCONFIG)
	@if [ ! -f board/$*/defconfig ]; then \
		echo "Error: board/$*/defconfig not found"; exit 1; \
	fi
	@echo "  DEFCONFIG board/$*/defconfig"
	@$(cmd_board_defconfig)
	$(call quiet,genconfig,GENCONFIG)

# Default defconfig (uses discoveryf4)
.PHONY: defconfig
defconfig: discoveryf4_defconfig

.PHONY: oldconfig
oldconfig: $(OLDCONFIG) $(GENCONFIG)
	$(call quiet,oldconfig,OLDCONFIG)
	$(call quiet,genconfig,GENCONFIG)

# Save current config to board defconfig (requires BOARD to be set via .config)
.PHONY: savedefconfig
savedefconfig: $(SAVEDEFCONFIG)
	@echo "  SAVEDEFCONFIG board/$(BOARD)/defconfig"
	@$(cmd_savedefconfig)

# List available boards
.PHONY: list-boards
list-boards:
	@echo "Available boards:"
	@for b in $(BOARDS); do echo "  make $${b}_defconfig"; done

.SECONDARY:

-include $(deps)
