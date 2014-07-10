/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_STM32F429_SYSCFG_H_
#define PLATFORM_STM32F429_SYSCFG_H_

#include <platform/stm32f429/registers.h>

static inline void memory_remap_flash(void)
{
	*SYSCFG_MEMRMP = 0x0;
}

static inline void memory_remap_sysflash(void)
{
	*SYSCFG_MEMRMP = 0x1;
}


static inline void memory_remap_sram(void)
{
	*SYSCFG_MEMRMP = 0x3;
}

#endif

