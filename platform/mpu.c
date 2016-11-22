/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fpage.h>
#include <memory.h>
#include <debug.h>
#include <error.h>
#include <platform/irq.h>
#include <platform/mpu.h>

void __attribute__((weak, alias("mpu_setup_region"))) mpu_setup_region(int n, fpage_t *fp)
{
}

void __attribute__((weak, alias("mpu_enable"))) mpu_enable(mpu_state_t i)
{
}

void __attribute__((weak, alias("__memmanage_handler"))) __memmanage_handler(void)
{
}
