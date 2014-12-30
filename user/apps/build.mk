# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

user-apps-dirs = \
	posixtest \

ifdef CONFIG_BOARD_STM32F429DISCOVERY
user-apps-dirs += \
	lcd_test
endif
