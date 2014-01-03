# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
ifeq "$(CONFIG_LOADER)" "y"
F9_LD_FILE = f9_sram.ld
include mk/loader.mk
else
F9_LD_FILE = f9_flash.ld
endif

# TODO: Put final target here temporarily
.PHONY: all
ifeq "$(CONFIG_LOADER)" "y"
all: loader
else
all: bare
endif
