/* Copyright (c) 2003 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

L4_ThreadId_t do_exreg_thread(L4_ThreadId_t tid, L4_Word_t ip, L4_Word_t sp);
L4_ThreadId_t do_exreg_thread_pager(
		L4_ThreadId_t pager, L4_ThreadId_t tid, 
		L4_Word_t ip, L4_Word_t sp);
