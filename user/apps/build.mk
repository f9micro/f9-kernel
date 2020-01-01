# Copyright (c) 2013,2017 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

ifdef CONFIG_BUILD_USER_APPS

user-apps-dirs = ""

ifdef CONFIG_L4_TEST
user-apps-dirs += \
	l4test
endif

ifdef CONFIG_PINGPONG
user-apps-dirs += \
	pingpong
endif

ifdef CONFIG_EXTI_INTERRUPT_TEST
user-apps-dirs += \
	irq_test
endif

ifdef CONFIG_LCD_TEST
user-apps-dirs += \
	lcd_test
endif
endif
