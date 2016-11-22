/*
 * Copyright (c) 2013
 *	The F9 Microkernel Project. All rights reserved.
 *
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdint.h>
#include <types.h>

/* Qsort routine from Bentley & McIlroy's "Engineering a Sort Function" */
#define swapcode(TYPE, parmi, parmj, n)			\
	{						\
		long i = (n) / sizeof (TYPE);		\
		TYPE *pi = (TYPE *) (parmi);		\
		TYPE *pj = (TYPE *) (parmj);		\
		do {					\
			TYPE t = *pi;			\
			*pi++ = *pj;			\
			*pj++ = t;			\
		} while (--i > 0);			\
	}

#define SWAPINIT(a, es)						\
	swaptype = ((char *) a - (char *) 0) % sizeof(long) ||	\
		es % sizeof(long) ?				\
			2 : es == sizeof(long) ?		\
				0 : 1;

static inline void swapfunc(char *a, char *b, int n, int swaptype)
{
	if (swaptype <= 1)
		swapcode(long, a, b, n)
	else
		swapcode(char, a, b, n)
}

#define vecswap(a, b, n)			\
	if ((n) > 0)				\
		swapfunc(a, b, n, swaptype)

/**
 * sorting time is O(n log n) both on average and worst-case.
 *
 * Typically, qsort is faster on average, but it suffers from exploitable
 * O(n*n) worst-case behavior and extra memory requirements that make it
 * less suitable for kernel use.
 */
void sort(void *base, size_t num, size_t size,
          int (*cmp_func)(const void *, const void *))
{
	int i = (num / 2 - 1) * size;
	int n = num * size;
	int c, r;

	int swaptype;

	SWAPINIT(base, size);

	/* perform a heapsort on the given array */
	for (; i >= 0; i -= size) {
		for (r = i; r * 2 + size < n; r  = c) {
			c = r * 2 + size;
			if (c < n - size &&
			    cmp_func(base + c, base + c + size) < 0)
				c += size;
			if (cmp_func(base + r, base + c) >= 0)
				break;
			vecswap(base + r, base + c, size);
		}
	}

	/* sort */
	for (i = n - size; i > 0; i -= size) {
		vecswap(base, base + i, size);
		for (r = 0; r * 2 + size < i; r = c) {
			c = r * 2 + size;
			if (c < i - size &&
			    cmp_func(base + c, base + c + size) < 0)
				c += size;
			if (cmp_func(base + r, base + c) >= 0)
				break;
			vecswap(base + r, base + c, size);
		}
	}
}
