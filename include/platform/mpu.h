/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_MPU_H_
#define PLATFORM_MPU_H_

#include <memory.h>

#define MPU_BASE_ADDR		0xE000ED9C
#define MPU_ATTR_ADDR		0xE000EDA0
#define MPU_CTRL_ADDR		0xE000ED94
#define MPU_RNR_ADDR		0xE000ED98

#define MPU_FAULT_STATUS_ADDR	0xE000ED28
#define MPU_FAULT_ADDRESS_ADDR	0xE000ED34

#define MPU_REGION_MASK		0xFFFFFFE0

/* 0x4 is PRIVDEFENA bit that allows access from kernel */
#define MPU_PRIVDEFENA		4

#define MPU_MEM_FAULT		0x80
#define MPU_MSTKERR		0x10
#define MPU_MUSTKERR		0x08
#define MPU_DACCVIOL		0x02
#define	MPU_IACCVIOL		0x01

void mpu_setup_region(int n, fpage_t *fp);
void mpu_enable(mpu_state_t i);
int mpu_select_lru(as_t *as, uint32_t addr);

#endif	/* PLATFORM_MPU_H_ */
