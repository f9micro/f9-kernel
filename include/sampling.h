/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SAMPLING_H
#define SAMPLING_H

#define MAX_SAMPLING_COUNT 10240
#define SAMPL_MAGIC 0xA2CB
#define MAX_KSYM 512

typedef struct _ksym {
	void *addr;
	int strid;
} ksym;

void sampling_init();
void sampled_pcpush(void *addr);
void sampled_prepare();
void sampled_enable();
void sampled_disable();

extern void *sampled_pc[];

#define for_each_sampled(addr, i) \
	for (i = 0, (addr) = sampled_pc; \
	    i < MAX_SAMPLING_COUNT; i++,(addr)++)

void ksym_init(int magic, int count, ksym *tbl, char *strings);
// total number of available kernel symbol
int ksym_total();

// symbol info lookup
int ksym_lookup(void *addr);
char *ksym_name(int symid);
void *ksym_addr(int symid);

#endif
