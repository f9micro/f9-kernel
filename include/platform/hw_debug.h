/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_HW_BREAKPOINT_H_
#define PLATFORM_HW_BREAKPOINT_H_

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

void hw_breakpoint_init();
int breakpoint_install(uint32_t addr);
void breakpoint_uninstall(int id);

#define cpu_enable_single_step()        do {*DCB_DEMCR |= DCB_DEMCR_MON_STEP;} while(0)
#define cpu_disable_single_step()       do {*DCB_DEMCR &= ~DCB_DEMCR_MON_STEP;} while(0)

#define enable_bkptid(id)               do {*(FPB_COMP + (id)) |= FPB_COMP_ENABLE;} while(0)
#define disable_bkptid(id)              do {*(FPB_COMP + (id)) &= ~FPB_COMP_ENABLE;} while(0)

#define enable_hw_breakpoint()          do {*FPB_CTRL = FPB_CTRL_KEY | FPB_CTRL_ENABLE;} while(0)
#define disable_hw_breakpoint()         do {*FPB_CTRL = FPB_CTRL_KEY | ~FPB_CTRL_ENABLE;} while(0)

#endif /* PLATFORM_HW_BREAKPOINT_H_ */
