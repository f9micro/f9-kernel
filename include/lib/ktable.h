/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef LIB_KTABLE_H_
#define LIB_KTABLE_H_

#include <platform/armv7m.h>
#include <platform/link.h>
#include <lib/bitmap.h>
#include <types.h>

struct ktable {
	char *tname;
	bitmap_ptr_t bitmap;
	ptr_t data;
	size_t num;
	size_t size;
};

typedef struct ktable ktable_t;

#define DECLARE_KTABLE(type, name, num_)			\
	DECLARE_BITMAP(kt_ ## name ## _bitmap, num_);		\
	static __KTABLE type kt_ ## name ## _data[num_];	\
	ktable_t name = {					\
			.tname = #name,				\
			.bitmap = kt_ ## name ## _bitmap,	\
			.data = (ptr_t) kt_ ## name ## _data,	\
			.num = num_, .size = sizeof(type)	\
	}

void ktable_init(ktable_t *kt);
int ktable_is_allocated(ktable_t *kt, int i);
void *ktable_alloc_id(ktable_t *kt, int i);
void *ktable_alloc(ktable_t *kt);
void  ktable_free(ktable_t *kt, void *element);

uint32_t ktable_getid(ktable_t *kt, void *element);

/*
 * For each ALLOCATED element in ktable
 * Should be used as:
 *
 * type *el;
 * int idx;
 *
 * for_each_in_ktable(el, idx, my_ktable) {
 * 	 ...
 * }
 * */
#define for_each_in_ktable(el, idx, kt)	\
	for (el = (typeof(el)) (kt)->data, idx = 0; idx < (kt)->num; ++idx, ++el) \
		if (bitmap_get_bit(bitmap_cursor((kt)->bitmap, idx)) == 1)

#endif /* LIB_KTABLE_H_ */
