/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef LIB_STDLIB_H_
#define LIB_STDLIB_H_

void sort(void *, size_t, size_t, int (*)(const void *, const void *));
void *bsearch(const void *key, const void *base0,
		size_t nmemb, size_t size,
		int (*compar)(const void *, const void *));

#endif
