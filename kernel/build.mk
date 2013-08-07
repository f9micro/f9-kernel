# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

kernel-y = \
	debug.o \
	error.o \
	fpage.o \
	init.o \
	ipc.o \
	kip.o \
	ktimer.o \
	memory.o \
	sched.o \
	softirq.o \
	start.o \
	syscall.o \
	systhread.o \
	thread.o \
	tickless_verify.o

KDB-$(CONFIG_KDB) = \
	kdb.o

KPROBES-$(CONFIG_KPROBES) = \
	kprobes.o

SYMMAP-$(CONFIG_SYMMAP) = \
	ksym.o \
	sampling.o \
	sampling-kdb.o

kernel-y += $(KDB-y) $(KPROBES-y) $(SYMMAP-y)
