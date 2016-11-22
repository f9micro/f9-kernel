/* Copyright (c) 2005-2013 Rich Felker
 * Copyright (c) 2013 The F9 Microkernel Project
 * All rights reserved.
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file.
 */

#include <lib/string.h>

int strcmp(const char *l, const char *r)
{
	for (; *l == *r && *l && *r; l++, r++);
	return *(unsigned char *) l - *(unsigned char *) r;
}
