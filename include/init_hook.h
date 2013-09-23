/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef INIT_HOOK_H_
#define INIT_HOOK_H_

enum init_levels {
	INIT_LEVEL_EARLIEST		= 1,
	INIT_LEVEL_PLATFORM_EARLY 	= 0x1000,
	INIT_LEVEL_PLATFORM		= 0x2000,
	INIT_LEVEL_KERNEL_EARLY		= 0x3000,
	INIT_LEVEL_KERNEL		= 0x4000,
	INIT_LEVEL_LAST			= 0xFFFFFFFF,
};

typedef void (*init_hook_t)(void);

typedef struct {
	unsigned int level;
	init_hook_t hook;
	const char *hook_name;
} init_struct;

#define INIT_HOOK(_hook, _level)					\
	const init_struct _init_struct_##_hook				\
			__attribute__((section(".init_hook"))) = {	\
		.level = _level,					\
		.hook = _hook,						\
		.hook_name = #_hook,					\
	};

int run_init_hook(unsigned int level);

#endif /* INIT_HOOK_H_ */
