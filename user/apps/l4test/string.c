/* Copyright (c) 2002-2003 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "string.h"
#include <platform/link.h>

int __USER_TEXT
strlen(const char *str)
{
	int len = 0;
	while (*str != '\0')
		str++, len++;

	return len;
}

int strcmp(const char *str1, const char *str2)
{
	while (*str1 && *str2) {
		if (*str1 < *str2)
			return -1;
		if (*str1 > *str2)
			return 1;
		str1++;
		str2++;
	}
	if (*str2)
		return -1;
	if (*str1)
		return 1;
	return 0;
}

int strcmp_of(const char *str_of, const char *search)
{
	while (*str_of && *search) {
		if ((*str_of == '@') && (*search == '/')) {
			while (*str_of && (*str_of != '/'))
				str_of++;
			if (!*str_of)
				return -1;
		}
		if (*str_of < *search)
			return -1;
		if (*str_of > *search)
			return 1;
		str_of++;
		search++;
	}

	if (*search)
		return -1;

	if (*str_of == '@')
		while (*str_of && (*str_of != '/'))
			str_of++;

	if (*str_of)
		return 1;

	return 0;
}
