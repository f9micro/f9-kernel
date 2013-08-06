/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SAMPLING_H
#define SAMPLING_H

#define MAX_SAMPLING_COUNT 10240

void sampling_init(void);
void sampled_pcpush(void *addr);
void sampling_disable(void);
void sampling_enable(void);
void sampling_stats(int **hitcountp, int **symid_list);

#endif
