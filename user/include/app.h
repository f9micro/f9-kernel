/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef USER_APP_H
#define USER_APP_H

#include <l4/types.h>

typedef struct app_struct app_struct;
typedef void (*app_entry)(app_struct *app);

typedef struct {
	L4_Word_t base;
	L4_Word_t size;
} app_fpage_t;

struct app_struct {
	L4_Word_t    tid;
	app_entry    entry;
	L4_Word_t    thread_num;
	app_fpage_t *fpages;
	const char  *name;
};

#define DECLARE_FPAGE(_base, _size)	\
	{.base = _base, .size = _size},

#define DECLARE_APP(_tid, _name, _entry, ...)	\
	app_fpage_t _app_fpages_##_name[]	\
			__attribute__((section(".user_data"))) = {	\
		__VA_ARGS__	\
		{.base = 0x0, .size = 0x0}	\
	};	\
	const app_struct _app_struct_##_name	\
			__attribute__((used, section(".user_app"))) = {	\
		.tid = _tid,	\
		.entry = _entry,	\
		.fpages = _app_fpages_##_name,	\
		.name = #_name,	\
	}

#endif /* USER_APP_H */
