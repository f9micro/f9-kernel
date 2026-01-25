# Copyright (c) 2024 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

user-apps-tests-y = \
	main.o

# FAULT_TYPE selects fault test mode (passed via make parameter)
# When set, only fault test code is compiled and -DFAULT_TYPE=N is added
# Note: CFLAGS_USER is safe here because fault tests do a clean build
# and are mutually exclusive with other test modes
ifdef FAULT_TYPE
user-apps-tests-y += test-fault.o
CFLAGS_USER += -DFAULT_TYPE=$(FAULT_TYPE)
else
# Normal test suite
user-apps-tests-y += \
	test-ipc.o \
	test-thread.o \
	test-timer.o \
	test-kip.o \
	test-safety.o

# IRQ tests only for normal test suite (not fault tests)
ifdef CONFIG_EXTI_INTERRUPT_TEST
user-apps-tests-y += test-irq.o
endif
endif
