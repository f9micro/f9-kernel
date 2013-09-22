# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

MCONF_MSG = mainmenu "F9 Microkernel Configurations"
define append_mconf_body
	MCONF_MSG += \n$(1)
endef
$(foreach k,$(strip $(KCONFIG_FILES)),\
	$(eval $(call append_mconf_body, \
		$(addprefix source ,$(k)))))
define init_mconf
	printf '$(MCONF_MSG)\n' > $(1)
endef
