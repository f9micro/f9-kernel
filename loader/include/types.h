/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <stddef.h>

typedef uint32_t ptr_t;
typedef uintptr_t memptr_t;

typedef uint32_t bool;

#define true 1;
#define false 0;


#define ALIGNED(size, align) \
	(size / align) + ((size & (align - 1)) != 0)

#endif /* TYPES_H_ */
