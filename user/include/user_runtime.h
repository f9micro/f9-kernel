/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef USER_RUNTIME_H
#define USER_RUNTIME_H

#include <l4/types.h>
#include <l4/ipc.h>

typedef struct user_struct user_struct;
typedef void (*user_entry)(user_struct *);

typedef struct {
	L4_Word_t base;
	L4_Word_t size;
} user_fpage_t;

struct user_struct {
	L4_Word_t tid;
	void (*entry)(void);
	L4_Word_t thread_num;
	user_fpage_t *fpages;
	const char *name;
};

#define DECLARE_FPAGE(_base, _size)	\
	{.base = _base, .size = _size},

#define DECLARE_USER(_tid, _name, _entry, ...)	\
	user_fpage_t _user_fpages_##_name[]	\
			__attribute__((section(".user_data"))) = {	\
		__VA_ARGS__	\
		{.base = 0x0, .size = 0x0}	\
	};	\
	static void _user_entry_##_name(void);	\
	user_struct _user_struct_##_name	\
			__attribute__((section(".user_runtime"))) = {	\
		.tid = _tid,	\
		.entry = _user_entry_##_name,	\
		.fpages = _user_fpages_##_name,	\
		.name = #_name,	\
	};	\
	static void __USER_TEXT _user_entry_##_name(void)	\
	{	\
		pager_thread(&_user_struct_##_name, _entry);  \
		while (1)	\
			L4_Sleep(L4_Never);	\
	}

#include <l4/pager.h>

#endif /* USER_RUNTIME_H */
