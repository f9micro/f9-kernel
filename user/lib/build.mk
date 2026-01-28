# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

user-lib-dirs = \
	l4 \
	io \
	libposix \

# MPU and SCB mocks (conditional, for QEMU testing)
user-lib-$(CONFIG_MPU_MOCK) += mpu_mock.o
user-lib-$(CONFIG_SCB_MOCK) += scb_mock.o
