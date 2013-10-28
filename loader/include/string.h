/*
 * Copyright (C) 2013 National ICT Australia Limited (NICTA)
 *
 * For non-commercial use only. For license terms, see the file
 * LICENSE-NICTA.pdf in the top-level directory.
 */

#ifndef _STRING_H_
#define _STRING_H_

#include <types.h>

int strcmp(const char* s1, const char *s2);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);

#endif
