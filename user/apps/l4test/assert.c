/* Copyright (c) 2003 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/kdebug.h>
#include <l4io.h>
#include <platform/link.h>

#include "config.h"
#include "assert.h"

__USER_TEXT void
__assert(const char *msg, const char *file, int line)
{
	printf("\n\nassertion failed: %s, line %d in %s\n\n",
	       msg, (int) line, file);

	L4_KDB_Enter("assertion failed");
}
