/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <lib/ktable.h>
#include <debug.h>

#ifdef CONFIG_KDB
#include <kdb.h>

#define KTABLE_NUM 16
static ktable_t *kdb_ktables[KTABLE_NUM];
static uint8_t kdb_ktable_cnt;

static void kdb_register_ktable(ktable_t *kt)
{
	if (kdb_ktable_cnt < (KTABLE_NUM - 1)) {
		kdb_ktables[kdb_ktable_cnt++] = kt;
	}
}

void kdb_dump_ktable(void)
{
	int i = 0, j;
	ktable_t *kt;

	for (; i < kdb_ktable_cnt; ++i) {
		kt = kdb_ktables[i];
		dbg_printf(DL_KDB, "\nKT: %s\nbitmap:%p, data:%p, num: %d size: %d\n",
		           kt->tname, kt->bitmap, kt->data, kt->num, kt->size);
		/* Dump bitmap */
		for (j = 0; j < kt->num; ++j) {
			if (j % 64 == 0)
				dbg_printf(DL_KDB, "%5d: ", j);

			dbg_putchar((bitmap_get_bit(bitmap_cursor(kt->bitmap, j))) ? 'X' : '-');

			if ((j + 1) % 64 == 0)
				dbg_puts("\n");
		}
	}
}

#endif	/* CONFIG_KDB */

/**
 * Initialize kernel table kt
 *
 * @param kt - pointer to kernel table
 * */
void ktable_init(ktable_t *kt)
{
	char *kt_ptr = (char *) kt->bitmap;
	char *kt_end = (char *) kt->bitmap + kt->num / 8;

	while (kt_ptr != kt_end)
		*(kt_ptr++) = 0x0;

#ifdef CONFIG_KDB
	kdb_register_ktable(kt);
#endif
}

/**
 * Checks if element already allocated
 *
 * @param kt - pointer to kernel table
 * @param i - index of element to check
 *
 * @result
 * 		-1 if index is out of bounds
 * 		0 if element is free
 * 		1 if element is allocated
 * */
int ktable_is_allocated(ktable_t *kt, int i)
{
	if (i > kt->num)
		return -1;

	return bitmap_get_bit(bitmap_cursor(kt->bitmap, i));
}

/**
 * Allocates element with specified index
 *
 * @param kt - pointer to kernel table
 * @param i - index of element to allocate
 *
 * @result
 * 		NULL if index is out of bounds or element is already allocated
 * 		address of allocated element
 */
void *ktable_alloc_id(ktable_t *kt, int i)
{
	bitmap_cursor_t	cursor = bitmap_cursor(kt->bitmap, i);

	if (i > kt->num)
		return NULL;

	if (bitmap_test_and_set_bit(cursor)) {
		dbg_printf(DL_KTABLE,
		           "KT: %s allocated %d [%p]\n", kt->tname, i,
		           kt->data + (i * kt->size));

		return (void *) kt->data + (i * kt->size);
	}

	return NULL;
}

/**
 * Allocates first free element
 *
 * @param kt - pointer to kernel table
 * @param i - index of element to allocate
 *
 * @result
 * 		NULL if index is out of bounds or if ktable is full
 * 		address of allocated element
 */
void *ktable_alloc(ktable_t *kt)
{
	bitmap_cursor_t	cursor;

	/* Search for free element */
	for_each_in_bitmap(cursor, kt->bitmap, kt->num, 0) {
		if (bitmap_test_and_set_bit(cursor)) {
			int i = bitmap_cursor_id(cursor);

			dbg_printf(DL_KTABLE,
			           "KT: %s allocated %d [%p]\n", kt->tname, i,
			           kt->data + (i * kt->size));

			return (void *) kt->data + (i * kt->size);
		}
	}

	dbg_printf(DL_KTABLE, "KT: %s allocated failed\n", kt->tname);

	return NULL;
}

/**
 * Gets index of element by it's pointer
 *
 * @param kt - pointer to kernel table
 * @param element - pointer to table's element
 *
 * @result
 * 		-1 if index is out of bounds
 * 		or index of element
 */
uint32_t ktable_getid(ktable_t *kt, void *element)
{
	int i = ((ptr_t) element - kt->data) / kt->size;

	/* Element does not belong to this ktable */
	if (i > kt->num || i < 0)
		return -1;

	return i;
}

/**
 * Frees an element
 *
 * @param kt - pointer to kernel table
 * @param element - pointer to table's element
 *
 * */
void ktable_free(ktable_t *kt, void *element)
{
	size_t i = ktable_getid(kt, element);

	if (i != -1) {
		bitmap_cursor_t	cursor = bitmap_cursor(kt->bitmap, i);

		dbg_printf(DL_KTABLE,
		           "KT: %s free %d [%p]\n", kt->tname, i, element);

		bitmap_clear_bit(cursor);
	}
}
