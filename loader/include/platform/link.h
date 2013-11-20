/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_LINK_H_
#define PLATFORM_LINK_H_

#include <types.h>

/*
 * Entries about linker address
 */

extern uint32_t bss_start;
extern uint32_t bss_end;

extern uint32_t kernel_flash_start;
extern uint32_t kernel_start;
extern uint32_t kernel_end;
extern uint32_t data_start;
extern uint32_t data_end;
extern uint32_t stack_end;

#define __BSS 			__attribute__ ((section(".bss")))
#define __ISR_VECTOR		__attribute__ ((section(".isr_vector")))


#define __PACKED		__attribute__ ((packed))

#define __NAKED 		__attribute__ ((naked))

#endif	/* PLATFORM_LINK_H_ */
