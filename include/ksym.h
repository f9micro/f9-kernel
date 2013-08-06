/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef KSYM_H_
#define KSYM_H_

#define KSYM_MAGIC 0xA2CB
#define MAX_KSYM 512

typedef struct ksym {
	void *addr;
	int strid;
} ksym_t;

void ksym_init(void);
int ksym_total(void);
int ksym_lookup(void *addr);
char *ksym_id2name(int symid);
void *ksym_id2addr(int symid);

#endif /* KSYM_H_ */
