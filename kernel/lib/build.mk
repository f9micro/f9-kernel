# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

lib-str-y += \
	memcpy.o \
	memset.o \


kernel-lib-y += \
	queue.o \
	ktable.o \
	stdio.o \
	bsearch.o \
	sort.o \
	$(lib-str-y)

loader-kernel-lib-y += \
	stdio.loader.o \
	queue.loader.o \
	strcmp.loader.o \
	memcpy.loader.o \
	memset.loader.o
