# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import_from_dirs = $(foreach dir,$($(1)-dirs),\
		$(addprefix $(dir)/,$($(1)-$(dir)-$(2))))

include_build_recusive = \
	$(foreach dir,$($(2)-dirs),\
		$(eval -include $(1)/$(dir)/build.mk) \
		$(call include_build_recusive,$(1)/$(dir),$(2)-$(dir))) \
	$(eval $(2)-y += $(call import_from_dirs,$(2),y)) \
	$(eval $(2)-dirs := $($(2)-dirs) $(call import_from_dirs,$(2),dirs)) \

user-dirs = \
	lib \
	apps

$(eval $(call include_build_recusive,user,user))
$(eval user-dirs := $(addprefix user/,$(user-dirs)))
