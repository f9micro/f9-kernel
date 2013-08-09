# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

CONFIG := .config

ifneq (,$(wildcard $(CONFIG)))
include $(CONFIG)
else
all: unconfigured_error
.PHONY: unconfigured_error
unconfigured_error:
	@echo "Please ensure that F9 Microkernel is configured by 'make config'"
	@$(MAKE) -s UNKNOWN 2>/dev/null
endif

BOARD ?= discoveryf4

PROJECT ?= f9

# output directory for build objects
out ?= build/$(BOARD)

# output directory for host build targets
out_host ?= build/host

# toolchain specific configurations; common cflags and ldflags
include mk/toolchain.mk

# obtain CHIP name
include board/$(BOARD)/build.mk

# Transform the configuration into make variables
includes = \
	board/$(BOARD) \
	include \
	include/platform \
	$(dirs) $(out)
$(eval BOARD_$(BOARD)=y)

# Read configurations about system features and characteristics
include mk/config.mk

# Get build configuration from sub-directories
include platform/$(CHIP)/build.mk
include platform/build.mk
include kernel/lib/build.mk
include kernel/build.mk
include user/build.mk

includes += $(includes-y)

objs_from_dir = $(foreach obj, $($(2)-y), \
		$(out)/$(1)/$(firstword $($(2)-mock-$(PROJECT)-$(obj)) $(obj)))

# Get all sources to build
all-y += $(call objs_from_dir,platform/$(CHIP),chip)
all-y += $(call objs_from_dir,board/$(BOARD),board)
all-y += $(call objs_from_dir,platform,platform)
all-y += $(call objs_from_dir,kernel/lib,kernel-lib)
all-y += $(call objs_from_dir,kernel,kernel)
all-y += $(call objs_from_dir,user,user)
dirs = \
	kernel/lib \
	kernel \
	platform/$(CHIP) \
	board/$(BOARD) \
	user

# Get special rules
dir-rules := mk/rules
$(foreach rule, $(shell ls $(dir-rules)), \
	$(eval include $(dir-rules)/$(rule)))

include mk/generic.mk
