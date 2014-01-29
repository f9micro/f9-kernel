/* Copyright (c) 2003, 2007 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

struct menuitem {
	void (*func)(void);  /* function to call on select (NULL == return) */
	const char *prompt;  /* user prompt */
};

struct menu {
	const char *title;      /* menu title */
	int clear;              /* always clear when drawing this */
	int nitems;             /* number of items */
	struct menuitem* items; /* ptr to an array of items */
};

/* handy for static guys */
#define NUM_ITEMS(x) (sizeof(x) / sizeof(struct menuitem))

/* interface functions */
void menu_input(struct menu *menu);
