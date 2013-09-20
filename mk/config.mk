# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

AUTOCONF_H := include/autoconf.h
config_list := $(shell [ -f $(AUTOCONF_H) ] || touch $(AUTOCONF_H) ; \
	$(CC) $(CPPFLAGS) -E -dM $(AUTOCONF_H) | \
	grep -o "\#define CONFIG_[A-Za-z0-9_]*" | cut -c9- | sort)
$(foreach c,$(config_list) $(config_list),$(eval $(c)=y))

MCONF_MSG = mainmenu "F9 Microkernel Configurations"
define append_mconf_body
	MCONF_MSG += \n$(1)
endef
$(foreach k,$(strip $(KCONFIG_FILES)),\
	$(eval $(call append_mconf_body, \
		$(addprefix source ,$(k)))))
define init_mconf
	echo '$(MCONF_MSG)' > $(1)
endef
