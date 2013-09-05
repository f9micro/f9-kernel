/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef INIT_H_
#define INIT_H_

enum f9_init_levels {
	F9_INIT_LEVEL_EARLIEST		 = 1,
	F9_INIT_LEVEL_PLATFORM		 = 0x1000,
	F9_INIT_LEVEL_KERNEL		 = 0x2000,
	F9_INIT_LEVEL_LAST		 = 0xFFFFFFFF,
};

typedef void (*init_hook_t)(unsigned int level);

typedef struct {
	unsigned int level;
	init_hook_t hook;
	const char *hook_name;
} f9_init_struct;

#define F9_INIT_HOOK(_name,_hook,_level) \
	const f9_init_struct _init_struct_##_name __attribute__((section(".f9_init"))) = { \
		.level = _level, \
		.hook = _hook, \
		.hook_name = #_name, \
	};

int f9_init_level(unsigned int level);

#endif /* INIT_H_ */
