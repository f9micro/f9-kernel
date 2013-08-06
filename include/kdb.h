/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef KDB_H_
#define KDB_H_

/* Simple in-kernel debugger */

int kdb_handler(char c);
int kdb_dump_error(void);

#endif /* KDB_H_ */
