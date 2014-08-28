/* Copyright (c) 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <platform/link.h>
#include "string.h"

enum SEMIHOST_SYSCALL {
	SYS_OPEN = 0x01,
	SYS_CLOSE,
	SYS_WRITEC,
	SYS_WRITE0,
	SYS_WRITE,
	SYS_READ,
	SYS_READC,
	SYS_ISERROR,
	SYS_ISTTY,
	SYS_SEEK,
	SYS_FLEN = 0xC,
	SYS_TMPNAM,
	SYS_REMOVE,
	SYS_RENAME,
	SYS_CLOCK,
	SYS_TIME,
	SYS_SYSTEM,
	SYS_ERRNO,
	SYS_GET_CMDLINE = 0x15,
	SYS_HEAPINFO,
	SYS_EXIT = 0x18,
	SYS_ELAPSED = 0x30,
	SYS_TICKFREQ
};

typedef union param_t {
	int entry_int;
	void *entry_ptr;
	char *entry_chr;
} param;

//extern int strlen( const char *src );

int __USER_TEXT semihost_call(enum SEMIHOST_SYSCALL action, void *param)
{
	static volatile int result;
	__asm__ __volatile__(
		"bkpt 0xAB\n"
		"nop\n"
		"bx lr\n"
		: "=r" (result) ::
	);
	return result;
}

int __USER_TEXT semihost_open(char *file, int mode)
{
	return semihost_call(SYS_OPEN, (param []) {
	                                {.entry_chr = file},
	                                {.entry_int = mode},
	                                {.entry_int = strlen(file)}});
}

int __USER_TEXT semihost_close(int fp)
{
	return semihost_call(SYS_CLOSE, (param []) {{.entry_int = fp}});
}

int __USER_TEXT semihost_write(int fp, char *buffer)
{
	return semihost_call(SYS_WRITE, (param []) {
	                                 {.entry_int = fp},
	                                 {.entry_chr = buffer},
	                                 {.entry_int = strlen(buffer)}});
}

int __USER_TEXT semihost_puts(char *log)
{
	char file[6] = "mylog";
	int handle;
	handle = semihost_open(file, 6);
	if (handle == -1)
		return -1;
	semihost_write(handle, log);
	semihost_close(handle);
	return 0;
}
