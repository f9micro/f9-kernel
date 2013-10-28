/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef BITOPS_H_
#define BITOPS_H_

#include <types.h>

typedef	uint32_t atomic_t;

/* Atomic ops */
void atomic_set(atomic_t *atom, atomic_t newval);
uint32_t atomic_get(atomic_t* atom);

uint32_t test_and_set_word(uint32_t *word);
uint32_t test_and_set_bit(uint32_t *word, int bitmask);

#endif /* BITOPS_H_ */
