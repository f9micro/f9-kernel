/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <lib/stdarg.h>
#include <lib/stdio.h>

void __l4_puts(char *str)
{
	while (*str) {
		if (*str == '\n')
			__l4_putchar('\r');
		__l4_putchar(*(str++));
	}
}

static void __l4_puts_x(char *str, int width, const char pad)
{
	while (*str) {
		if (*str == '\n')
			__l4_putchar('\r');
		__l4_putchar(*(str++));
		--width;
	}

	while (width > 0) {
		__l4_putchar(pad);
		--width;
	}
}

#define hexchars(x)			\
	(((x) < 10) ?			\
		('0' + (x)) :		\
	 	('a' + ((x) - 10)))

static int __l4_put_hex(const uint32_t val, int width, const char pad)
{
	int i, n = 0;
	int nwidth = 0;

	/* Find width of hexnumber */
	while ((val >> (4 * nwidth)) && ((unsigned) nwidth <  2 * sizeof(val)))
		nwidth++;
	if (nwidth == 0)
		nwidth = 1;

	/* May need to increase number of printed characters */
	if (width == 0 && width < nwidth)
		width = nwidth;

	/* Print number with padding */
	for (i = width - nwidth; i > 0; i--, n++)
		__l4_putchar(pad);
	for (i = 4 * (nwidth - 1); i >= 0; i -= 4, n++)
		__l4_putchar(hexchars((val >> i) & 0xF));

	return n;
}

static void __l4_put_dec(const uint32_t val, const int width, const char pad)
{
	uint32_t divisor;
	int digits;

	/* estimate number of spaces and digits */
	for (divisor = 1, digits = 1; val / divisor >= 10; divisor *= 10, digits++)
		/* */ ;

	/* print spaces */
	for (; digits < width; digits++)
		__l4_putchar(pad);

	/* print digits */
	do {
		__l4_putchar(((val / divisor) % 10) + '0');
	} while (divisor /= 10);
}


void __l4_printf(char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	__l4_vprintf(fmt, va);
	va_end(va);
}

void __l4_vprintf(char *fmt, va_list va)
{
	int mode = 0;	/* 0: usual char; 1: specifiers */
	int width = 0;
	char pad = ' ';
	int size = 16;

	while (*fmt) {
		if (*fmt == '%') {
			mode = 1;
			pad = ' ';
			width = 0;
			size = 32;

			fmt++;
			continue;
		}

		if (!mode) {
			if (*fmt == '\n')
				__l4_putchar('\r');
			__l4_putchar(*fmt);
		} else {
			switch (*fmt) {
			case 'c':
				__l4_putchar(va_arg(va, uint32_t));
				mode = 0;
				break;
			case 's':
				__l4_puts_x(va_arg(va, char *), width, pad);
				mode = 0;
				break;
			case 'l':
			case 'L':
				size = 64;
				break;
			case 'd':
			case 'D':
				__l4_put_dec((size == 32) ?
				             va_arg(va, uint32_t) :
				             va_arg(va, uint64_t),
				             width, pad);
				mode = 0;
				break;
			case 'p':
			case 't':
				size = 32;
				width = 8;
				pad = '0';
			case 'x':
			case 'X':
				__l4_put_hex((size == 32) ?
				             va_arg(va, uint32_t) :
				             va_arg(va, uint64_t),
				             width, pad);
				mode = 0;
				break;
			case '%':
				__l4_putchar('%');
				mode = 0;
				break;
			case '0':
				if (!width)
					pad = '0';
				break;
			case ' ':
				pad = ' ';
			}

			if (*fmt >= '0' && *fmt <= '9') {
				width = width * 10 + (*fmt - '0');
			}
		}

		fmt++;
	}
}
