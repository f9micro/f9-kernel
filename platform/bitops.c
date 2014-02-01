/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/armv7m.h>
#include <platform/bitops.h>

#ifdef CONFIG_SMP

/* Atomic ops */
void atomic_set(atomic_t *atom, atomic_t newval)
{
	__asm__ __volatile__("mov r1, %0" : : "r"(newval));
	__asm__ __volatile__("atomic_try: ldrex r0, [%0]\n"
	                     "strex r0, r1, [%0]\n"
	                     "cmp r0, #0"
	                     :
	                     : "r"(atom));
	__asm__ __volatile__("bne atomic_try");
}

uint32_t atomic_get(atomic_t *atom)
{
	atomic_t result;

	__asm__ __volatile__("ldrex r0, [%0]"
	                     :
	                     : "r"(atom));
	__asm__ __volatile__("clrex");
	__asm__ __volatile__("mov %0, r0" : "=r"(result));

	return result;
}

#else	/* !CONFIG_SMP */

void atomic_set(atomic_t *atom, atomic_t newval)
{
	*atom = newval;
}

uint32_t atomic_get(atomic_t *atom)
{
	return *atom;
}

#endif	/* CONFIG_SMP */

uint32_t test_and_set_word(uint32_t *word)
{
	register int result = 1;

	__asm__ __volatile__(
	    "mov r1, #1\n"
	    "mov r2, %[word]\n"
	    "ldrex r0, [r2]\n"	/* Load value [r2] */
	    "cmp r0, #0\n"	/* Checking is word set to 1 */

	    "itt eq\n"
	    "strexeq r0, r1, [r2]\n"
	    "moveq %[result], r0\n"
	    : [result] "=r"(result)
	    : [word] "r"(word)
	    : "r0", "r1", "r2");

	return result == 0;
}

uint32_t test_and_set_bit(uint32_t *word, int bitmask)
{
	register int result = 1;

	__asm__ __volatile__(
	    "mov r2, %[word]\n"
	    "ldrex r0, [r2]\n"		/* Load value [r2] */
	    "tst r0, %[bitmask]\n"	/* Compare value with bitmask */

	    "ittt eq\n"
	    "orreq r1, r0, %[bitmask]\n"	/* Set bit: r1 = r0 | bitmask */
	    "strexeq r0, r1, [r2]\n"		/* Write value back to [r2] */
	    "moveq %[result], r0\n"
	    : [result] "=r"(result)
	    : [word] "r"(word), [bitmask] "r"(bitmask)
	    : "r0", "r1", "r2");

	return result == 0;
}
