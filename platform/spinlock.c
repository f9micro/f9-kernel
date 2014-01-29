/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/armv7m.h>
#include <platform/spinlock.h>

#ifdef CONFIG_SMP

/* for future use only */

/* Basic spinlock ops for ARMv7M architecture */
int spinlock_trylock(spinlock_t *sl)
{
	int result = 0;	/* Assuming lock is busy */

	__asm__ __volatile__("mov r1, #1");
	__asm__ __volatile__("mov r7, %0" : : "r"(sl));
	__asm__ __volatile__("ldrex r0, [r7]" :);
	__asm__ __volatile__("cmp r0, #0");

	/* Lock is not busy, trying to get it */
	__asm__ __volatile__("itt eq\n"
	                     "strexeq r0, r1, [r7]\n"
	                     "moveq %0, r0"
	                     : "=r"(result));

	return result;
}

void spinlock_lock(spinlock_t *sl)
{
	while (spinlock_trylock(sl) != 1)
		/* */;
}

void spinlock_unlock(spinlock_t *sl)
{
	__asm__ __volatile__("mov r1, #0");
	__asm__ __volatile__("spinlock_try: ldrex r0, [%0]\n"
	                     "strex r0, r1, [%0]\n"
	                     "cmp r0, #0"
	                     :
	                     : "r"(sl));
	__asm__ __volatile__("bne spinlock_try");
}

#endif
