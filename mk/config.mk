# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# FIXME: the current pattern can not handle the following definition:
#	#define CONFIG_KTIMER_HEARTBEAT		65536
# It will be set to "y" forcely, which is incorrect.
#
config_list := $(shell $(CC) -E -dM include/config.h | \
	grep -o "\#define CONFIG_[A-Za-z0-9_]*" | cut -c9- | sort)
$(foreach c,$(config_list) $(config_list),$(eval $(c)=y))
