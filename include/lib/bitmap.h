/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef LIB_BITMAP_H_
#define LIB_BITMAP_H_

#include <platform/link.h>
#include <platform/bitops.h>
#include <types.h>

/**
 * @file 	bitmap.h
 * @brief 	Kernel bitmaps
 *
 * Bit band bitmaps relocated in AHB SRAM, so we use BitBang addresses
 * accessing bits. Also uses linker segment ".bitmap" from platform/link.h
 *
 * Bitmap iterates using bitmap cursor (type bitmap_cursor_t).
 * For bit-band bitmaps it an address in bit-band region, for classic bitmaps,
 * it is simply number of bit.
 */

#define BITMAP_ALIGN	32
#define DECLARE_BITMAP(name, size) \
	static __BITMAP uint32_t name [ALIGNED(size, BITMAP_ALIGN)];

typedef uint32_t *bitmap_ptr_t;

#ifdef CONFIG_BITMAP_BITBAND

typedef uint8_t *bitmap_cursor_t;

/* Generate address in bit-band region */
#define BITBAND_ADDR_SHIFT 	5
#define ADDR_BITBAND(addr) \
	(bitmap_cursor_t) (0x22000000 + \
	                   ((((ptr_t) addr) & 0xFFFFF) << BITBAND_ADDR_SHIFT))
#define BIT_SHIFT	 		2

#define bitmap_cursor(bitmap, bit) \
	((ADDR_BITBAND(bitmap) + (bit << BIT_SHIFT)))
#define bitmap_cursor_id(cursor) \
	(((ptr_t) cursor & ((1 << (BITBAND_ADDR_SHIFT + BIT_SHIFT)) - 1)) >> BIT_SHIFT)
#define bitmap_cursor_goto_next(cursor) \
	cursor += 1 << BIT_SHIFT

static inline void bitmap_set_bit(bitmap_cursor_t cursor)
{
	*cursor = 1;
}

static inline void bitmap_clear_bit(bitmap_cursor_t cursor)
{
	*cursor = 0;
}

static inline int bitmap_get_bit(bitmap_cursor_t cursor)
{
	return *cursor;
}

static inline int bitmap_test_and_set_bit(bitmap_cursor_t cursor)
{
	return test_and_set_word((uint32_t *) cursor);
}

#else

typedef struct {
	bitmap_ptr_t bc_bitmap;
	int bc_bit;
} bitmap_cursor_t;

#define bitmap_cursor(bitmap, bit) \
	(bitmap_cursor_t) { .bc_bitmap = bitmap, .bc_bit = bit }
#define bitmap_cursor_id(cursor) \
	cursor.bc_bit
#define bitmap_cursor_goto_next(cursor) \
	cursor.bc_bit++

#define BITOFF(bit) \
	(bit % BITMAP_ALIGN)		/* bit offset inside 32-bit word */
#define BITMASK(bit) \
	(1 << BITOFF(bit))		/* Mask used for bit number N */
#define BITINDEX(bit) \
	(bit / BITMAP_ALIGN)		/* Bit index in bitmap array */
#define BITWORD(cursor) \
	cursor.bc_bitmap[BITINDEX(cursor.bc_bit)]

static inline void bitmap_set_bit(bitmap_cursor_t cursor)
{
	BITWORD(cursor) |= BITMASK(cursor.bc_bit);
}

static inline void bitmap_clear_bit(bitmap_cursor_t cursor)
{
	BITWORD(cursor) &= ~BITMASK(cursor.bc_bit);
}

static inline int bitmap_get_bit(bitmap_cursor_t cursor)
{
	return (BITWORD(cursor) >> BITOFF(cursor.bc_bit)) & 0x1;
}

static inline int bitmap_test_and_set_bit(bitmap_cursor_t cursor)
{
	return test_and_set_bit(&BITWORD(cursor), BITMASK(cursor.bc_bit));
}

#endif

#define for_each_in_bitmap(cursor, bitmap, size, start) \
	for (cursor = bitmap_cursor(bitmap, start); \
	     bitmap_cursor_id(cursor) < size;	\
	     bitmap_cursor_goto_next(cursor))

#endif /* LIB_BITMAP_H_ */
