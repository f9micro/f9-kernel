# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

config_list := $(shell touch include/autoconf.h ; $(CC) -E -dM include/config.h | \
	grep -o "\#define CONFIG_[A-Za-z0-9_]*" | cut -c9- | sort)
$(foreach c,$(config_list) $(config_list),$(eval $(c)=y))
