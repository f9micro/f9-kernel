/* Copyright (c) 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_STM32F1_GPIO_H_
#define PLATFORM_STM32F1_GPIO_H_

#include <platform/stm32f1/registers.h>

enum {
    AF0 = 0,
    AF1,
    AF2,
    AF3,
    AF4,
    AF5,
    AF6,
    AF7,
    AF8,
    AF9,
    AF10,
    AF11,
    AF12,
    AF13,
    AF14,
    AF15,
};

struct gpio_cfg {
    uint8_t port;
    uint8_t pin;
    uint8_t mode;
    uint8_t ospeed;
    uint8_t func;
};

/* GPIO Alternative Function */
#define     af_spi1             ((uint32_t)0x00000001)
#define     af_i2c1             ((uint32_t)0x00000002)
#define     af_usart1           ((uint32_t)0x00000004)
#define     af_usart2           ((uint32_t)0x00000008)
#define     af_partial_usart3   ((uint32_t)0x00140010)
#define     af_full_usart3      ((uint32_t)0x00140030)
#define     af_partial_tim1     ((uint32_t)0x00160040)
#define     af_full_tim1        ((uint32_t)0x001600C0)
#define     af_partial_tim2_1   ((uint32_t)0x00180100)
#define     af_partial_tim2_2   ((uint32_t)0x00180200)
#define     af_full_tim2        ((uint32_t)0x00180300)
#define     af_partial_tim3     ((uint32_t)0x001A0800)
#define     af_full_tim3        ((uint32_t)0x001A0C00)
#define     af_tim4             ((uint32_t)0x00001000)
#define     af_can1_1           ((uint32_t)0x001D4000)
#define     af_can1_2           ((uint32_t)0x001D6000)
#define     af_pd01             ((uint32_t)0x00008000)
#define     af_tim5_ch4         ((uint32_t)0x00200001)
#define     af_adc1_ext_inj     ((uint32_t)0x00200002)
#define     af_adc1_ext_reg     ((uint32_t)0x00200004)
#define     af_adc2_ext_inj     ((uint32_t)0x00200008)
#define     af_adc2_ext_reg     ((uint32_t)0x00200010)
#define     af_eth              ((uint32_t)0x00200020)
#define     af_can2             ((uint32_t)0x00200040)
#define     af_swj_nojtrst      ((uint32_t)0x00300100)
#define     af_swj_jtagdisable  ((uint32_t)0x00300200)
#define     af_swj_disable      ((uint32_t)0x00300400)
#define     af_spi3             ((uint32_t)0x00201100)
#define     af_tim2itr1_ptp_sof ((uint32_t)0x00202000)
#define     af_ptp_pps          ((uint32_t)0x00200400)
#define     af_tim1_dma         ((uint32_t)0x80000010)

void gpio_config(struct gpio_cfg *cfg);
void gpio_config_output(uint8_t port, uint8_t pin, uint8_t mode, uint8_t ospeed);
void gpio_config_input(uint8_t port, uint8_t pin, uint8_t mode);
void gpio_out_high(uint8_t port, uint8_t pin);
void gpio_out_low(uint8_t port, uint8_t pin);
uint8_t gpio_input_bit(uint8_t port, uint8_t pin);
void gpio_writebit(uint8_t port, uint8_t pin, uint8_t bitval);

#endif /* PLATFORM_STM32F1_GPIO_H_ */
