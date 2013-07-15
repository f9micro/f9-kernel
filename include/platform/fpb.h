/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_FPB_H_
#define PLATFORM_FPB_H_

#define FPB_MAX_COMP                    6
#define FPB_BASE                        (uint32_t) (0xE0002000)
#define FPB_CTRL                        (volatile uint32_t *) (FPB_BASE)
#define FPB_REMAP                       (volatile uint32_t *) (FPB_BASE + 0x04)
#define FPB_COMP                        (volatile uint32_t *) (FPB_BASE + 0x08)

#define FPB_CTRL_ENABLE                 (uint32_t) (1 << 0)
#define FPB_CTRL_KEY                    (uint32_t) (1 << 1)

#define FPB_COMP_ENABLE                 (uint32_t) (1 << 0)
#define FPB_COMP_REPLACE_LOWER          (uint32_t) (1 << 30)
#define FPB_COMP_REPLACE_UPPER          (uint32_t) (2 << 30)

void fpb_init();
void fpb_setbkpt(int id, uint32_t addr);
void fpb_unsetbkpt(int id);
void fpb_bkpt_enable(int id);
void fpb_bkpt_disable(int id);
int fpb_avail_bkpt();

#define cpu_step_enable()            do {*DCB_DEMCR |= DCB_DEMCR_MON_STEP;} while(0)
#define cpu_step_disable()           do {*DCB_DEMCR &= ~DCB_DEMCR_MON_STEP;} while(0)

void fpb_enable();
void fpb_disable();

#endif /* PLATFORM_FPB_H_ */
