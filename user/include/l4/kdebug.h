/* Copyright (c) 2003 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __L4__KDEBUG_H__
#define __L4__KDEBUG_H__

#include <l4/types.h>
#include __L4_INC_ARCH(kdebug.h)

/*
 * Compatibility with old naming scheme.
 */
#define enter_kdebug(x)		L4_KDB_Enter(x)
#define kd_inchar()		L4_KDB_ReadChar_Blocked()

#endif /* !__L4__KDEBUG_H__ */
