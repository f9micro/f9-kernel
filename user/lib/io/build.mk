# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

user-lib-io-y = \
	l4io.o \
	user_interrupt.o

ifeq "$(CONFIG_SEMIHOST)" "y"
user-lib-io-y += \
	semihost-io.o
endif
