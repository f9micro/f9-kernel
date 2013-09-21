/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_DEBUG_RAM_H_
#define PLATFORM_DEBUG_RAM_H_

#include <types.h>

/* Default debug ram buffer size */
#define DBG_RAM_BUFSIZE    0x2000

void dbg_ram_init(void);

#endif /* PLATFORM_DEBUG_RAM_H_ */
