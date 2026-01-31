# Copyright (c) 2013,2017 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

user-apps-dirs = ""

ifdef CONFIG_USER_APP_PINGPONG
user-apps-dirs += \
	pingpong
endif

ifdef CONFIG_USER_APP_TESTS
user-apps-dirs += \
	tests
endif

ifdef CONFIG_LCD_TEST
user-apps-dirs += \
	lcd_test
endif
