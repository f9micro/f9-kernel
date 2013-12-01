/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FPAGE_IMPL_H_
#define FPAGE_IMPL_H_

void fpages_init(void);

fpage_t *split_fpage(as_t *as, fpage_t *fpage, memptr_t split, int rl);

int assign_fpages(as_t *as, memptr_t base, size_t size);
int assign_fpages_ext(int mpid, as_t *as, memptr_t base, size_t size,
                      fpage_t **pfirst, fpage_t **plast);

int map_fpage(as_t *src, as_t *dst, fpage_t *fpage, map_action_t action);
int unmap_fpage(as_t *as, fpage_t *fpage);
void destroy_fpage(fpage_t *fpage);

#endif /* FPAGE_IMPL_H_ */
