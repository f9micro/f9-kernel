/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_SPINLOCK_H_
#define PLATFORM_SPINLOCK_H_

#include <types.h>

typedef uint32_t spinlock_t;

#define SPINLOCK_INITIALIZER 	0x0

/* Basic spinlock ops for ARMv7M architecture */
int spinlock_trylock(spinlock_t *sl);
void spinlock_lock(spinlock_t *sl);
void spinlock_unlock(spinlock_t *sl);

#endif /* PLATFORM_SPINLOCK_H_ */
