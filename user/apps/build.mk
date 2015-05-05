# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

user-apps-dirs = \
	l4test \
	pingpong


ifdef CONFIG_EXTI_INTERRUPT_TEST
user-apps-dirs += \
	irq_test
else
ifdef CONFIG_BOARD_STM32F429DISCOVERY
user-apps-dirs += \
	lcd_test
endif
endif
