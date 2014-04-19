/* Copyright (c) 2003-2004 Karlsruhe University. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __GPIOER_H__
#define __GPIOER_H__

#include <l4/types.h>
#include <platform/stm32f4/registers.h>
#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_HIGH 1
#define GPIO_LOW 0

void gpioer_config_output(uint8_t port, uint8_t pin, uint8_t pupd, uint8_t speed);
void gpioer_out(uint8_t port, uint8_t pin, uint8_t action);

#ifdef __cplusplus
}
#endif

#endif /* !__GPIOER_H__ */
