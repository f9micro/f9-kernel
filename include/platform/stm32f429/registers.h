/* STM32F429 Registers and Memory Locations */

#ifndef STM32F429_REGISTERS_H_INCLUDED
#define STM32F429_REGISTERS_H_INCLUDED

#include <stdint.h>

#define STM32F4X

/* Memory Map */
#define MEMORY_BASE                     (uint32_t) (0x00000000)                                 /* Base of memory map */
#define FLASH_BASE                      (uint32_t) (0x08000000)                                 /* Flash Memory Base Address */
#define RAM_BASE                        (uint32_t) (0x20000000)                                 /* RAM Base Address */
#define ETHRAM_BASE                     (uint32_t) (0x2001C000)                                 /* ETHRAM Base Address */
#define CCMRAM_BASE                     (uint32_t) (0x10000000)                                 /* CCMRAM Base Address - Accessible only to CPU */

/* Peripheral Map */
#define PERIPH_BASE                     (uint32_t) (0x40000000)                                 /* Peripheral base address */
#define PRIV_PERIPH_BASE                (uint32_t) (0xED000000)                                 /* Private peripheral base address */
#define APB1PERIPH_BASE                 (PERIPH_BASE)
#define APB2PERIPH_BASE                 (PERIPH_BASE + 0x00010000)
#define AHB1PERIPH_BASE                 (PERIPH_BASE + 0x00020000)
#define AHB2PERIPH_BASE                 (PERIPH_BASE + 0x10000000)

#define PERIPH_BB_BASE                  (PERIPH_BASE + 0x02000000)                                 /* Peripheral base address in bit-band region */

/* APB1 peripherals */
#define TIM2_BASE             (APB1PERIPH_BASE + 0x0000)
#define TIM3_BASE             (APB1PERIPH_BASE + 0x0400)
#define TIM4_BASE             (APB1PERIPH_BASE + 0x0800)
#define TIM5_BASE             (APB1PERIPH_BASE + 0x0C00)
#define TIM6_BASE             (APB1PERIPH_BASE + 0x1000)
#define TIM7_BASE             (APB1PERIPH_BASE + 0x1400)
#define TIM12_BASE            (APB1PERIPH_BASE + 0x1800)
#define TIM13_BASE            (APB1PERIPH_BASE + 0x1C00)
#define TIM14_BASE            (APB1PERIPH_BASE + 0x2000)
#define RTC_BASE              (APB1PERIPH_BASE + 0x2800)
#define WWDG_BASE             (APB1PERIPH_BASE + 0x2C00)
#define IWDG_BASE             (APB1PERIPH_BASE + 0x3000)
#define I2S2ext_BASE          (APB1PERIPH_BASE + 0x3400)
#define SPI2_BASE             (APB1PERIPH_BASE + 0x3800)
#define SPI3_BASE             (APB1PERIPH_BASE + 0x3C00)
#define I2S3ext_BASE          (APB1PERIPH_BASE + 0x4000)
#define USART2_BASE           (APB1PERIPH_BASE + 0x4400)
#define USART3_BASE           (APB1PERIPH_BASE + 0x4800)
#define UART4_BASE            (APB1PERIPH_BASE + 0x4C00)
#define UART5_BASE            (APB1PERIPH_BASE + 0x5000)
#define I2C1_BASE             (APB1PERIPH_BASE + 0x5400)
#define I2C2_BASE             (APB1PERIPH_BASE + 0x5800)
#define I2C3_BASE             (APB1PERIPH_BASE + 0x5C00)
#define CAN1_BASE             (APB1PERIPH_BASE + 0x6400)
#define CAN2_BASE             (APB1PERIPH_BASE + 0x6800)
#define PWR_BASE              (APB1PERIPH_BASE + 0x7000)
#define DAC_BASE              (APB1PERIPH_BASE + 0x7400)

/* APB2 peripherals */
#define TIM1_BASE             (APB2PERIPH_BASE + 0x0000)
#define TIM8_BASE             (APB2PERIPH_BASE + 0x0400)
#define USART1_BASE           (APB2PERIPH_BASE + 0x1000)
#define USART6_BASE           (APB2PERIPH_BASE + 0x1400)
#define ADC1_BASE             (APB2PERIPH_BASE + 0x2000)
#define ADC2_BASE             (APB2PERIPH_BASE + 0x2100)
#define ADC3_BASE             (APB2PERIPH_BASE + 0x2200)
#define ADC_BASE              (APB2PERIPH_BASE + 0x2300)
#define SDIO_BASE             (APB2PERIPH_BASE + 0x2C00)
#define SPI1_BASE             (APB2PERIPH_BASE + 0x3000)
#define SYSCFG_BASE           (APB2PERIPH_BASE + 0x3800)
#define EXTI_BASE             (APB2PERIPH_BASE + 0x3C00)
#define TIM9_BASE             (APB2PERIPH_BASE + 0x4000)
#define TIM10_BASE            (APB2PERIPH_BASE + 0x4400)
#define TIM11_BASE            (APB2PERIPH_BASE + 0x4800)
#define SPI5_BASE             (APB2PERIPH_BASE + 0x5000)
#define LTDC_BASE             (APB2PERIPH_BASE + 0x6800)

/* AHB */
#define GPIO_BASE(port)                 (AHB1PERIPH_BASE + (0x400*port))                        /* GPIO Port base address */
#define RCC_BASE                        (AHB1PERIPH_BASE + 0x3800)                              /* Reset and Clock Control base address */
#define FLASH_R_BASE                    (AHB1PERIPH_BASE + 0x3C00)                              /* Flash registers base address */
#define DMA1_BASE                       (AHB1PERIPH_BASE + 0x6000)                              /* DMA1 base address */
#define DMA2_BASE                       (AHB1PERIPH_BASE + 0x6400)                              /* DMA2 base address */
#define DMA2D_BASE                      (AHB1PERIPH_BASE + 0xB000)                              /* DMA2D base address */
#define USB_FS_BASE                     (AHB2PERIPH_BASE + 0x0000)                              /* USB OTG FS base address */


/* Timer 1 (TIM1) */
#define TIM1_CR1                        (volatile uint32_t *) (TIM1_BASE + 0x00)                /* TIM1 control register 1 */
#define TIM1_CR2                        (volatile uint32_t *) (TIM1_BASE + 0x04)                /* TIM1 control register 2 */
#define TIM1_SMCR                       (volatile uint32_t *) (TIM1_BASE + 0x08)                /* TIM1 slave mode control register */
#define TIM1_DIER                       (volatile uint32_t *) (TIM1_BASE + 0x0C)                /* TIM1 DMA/Interrupt enable register */
#define TIM1_SR                         (volatile uint32_t *) (TIM1_BASE + 0x10)                /* TIM1 status register */
#define TIM1_EGR                        (volatile uint32_t *) (TIM1_BASE + 0x14)                /* TIM1 event generation register */
#define TIM1_CCMR1                      (volatile uint32_t *) (TIM1_BASE + 0x18)                /* TIM1 capture/compare mode register 1 */
#define TIM1_CCMR2                      (volatile uint32_t *) (TIM1_BASE + 0x1C)                /* TIM1 capture/compare mode register 2 */
#define TIM1_CCER                       (volatile uint32_t *) (TIM1_BASE + 0x20)                /* TIM1 capture/compare enable register */
#define TIM1_CNT                        (volatile uint32_t *) (TIM1_BASE + 0x24)                /* TIM1 counter */
#define TIM1_PSC                        (volatile uint32_t *) (TIM1_BASE + 0x28)                /* TIM1 prescaler */
#define TIM1_ARR                        (volatile uint32_t *) (TIM1_BASE + 0x2C)                /* TIM1 auto-reload register */
#define TIM1_CCR1                       (volatile uint32_t *) (TIM1_BASE + 0x34)                /* TIM1 capture/compare register 1 */
#define TIM1_CCR2                       (volatile uint32_t *) (TIM1_BASE + 0x38)                /* TIM1 capture/compare register 2 */
#define TIM1_CCR3                       (volatile uint32_t *) (TIM1_BASE + 0x3C)                /* TIM1 capture/compare register 3 */
#define TIM1_CCR4                       (volatile uint32_t *) (TIM1_BASE + 0x40)                /* TIM1 capture/compare register 4 */
#define TIM1_DCR                        (volatile uint32_t *) (TIM1_BASE + 0x48)                /* TIM1 DMA control register */
#define TIM1_DMAR                       (volatile uint32_t *) (TIM1_BASE + 0x4C)                /* TIM1 DMA address for full transfer */
#define TIM1_OR                         (volatile uint32_t *) (TIM1_BASE + 0x50)                /* TIM1 option register */

#define TIM2_CR1                        (volatile uint32_t *) (TIM2_BASE + 0x00)                /* TIM2 control register 1 */
#define TIM2_CR2                        (volatile uint32_t *) (TIM2_BASE + 0x04)                /* TIM2 control register 2 */
#define TIM2_SMCR                       (volatile uint32_t *) (TIM2_BASE + 0x08)                /* TIM2 slave mode control register */
#define TIM2_DIER                       (volatile uint32_t *) (TIM2_BASE + 0x0C)                /* TIM2 DMA/Interrupt enable register */
#define TIM2_SR                         (volatile uint32_t *) (TIM2_BASE + 0x10)                /* TIM2 status register */
#define TIM2_EGR                        (volatile uint32_t *) (TIM2_BASE + 0x14)                /* TIM2 event generation register */
#define TIM2_CCMR1                      (volatile uint32_t *) (TIM2_BASE + 0x18)                /* TIM2 capture/compare mode register 1 */
#define TIM2_CCMR2                      (volatile uint32_t *) (TIM2_BASE + 0x1C)                /* TIM2 capture/compare mode register 2 */
#define TIM2_CCER                       (volatile uint32_t *) (TIM2_BASE + 0x20)                /* TIM2 capture/compare enable register */
#define TIM2_CNT                        (volatile uint32_t *) (TIM2_BASE + 0x24)                /* TIM2 counter */
#define TIM2_PSC                        (volatile uint32_t *) (TIM2_BASE + 0x28)                /* TIM2 prescaler */
#define TIM2_ARR                        (volatile uint32_t *) (TIM2_BASE + 0x2C)                /* TIM2 auto-reload register */
#define TIM2_CCR1                       (volatile uint32_t *) (TIM2_BASE + 0x34)                /* TIM2 capture/compare register 1 */
#define TIM2_CCR2                       (volatile uint32_t *) (TIM2_BASE + 0x38)                /* TIM2 capture/compare register 2 */
#define TIM2_CCR3                       (volatile uint32_t *) (TIM2_BASE + 0x3C)                /* TIM2 capture/compare register 3 */
#define TIM2_CCR4                       (volatile uint32_t *) (TIM2_BASE + 0x40)                /* TIM2 capture/compare register 4 */
#define TIM2_DCR                        (volatile uint32_t *) (TIM2_BASE + 0x48)                /* TIM2 DMA control register */
#define TIM2_DMAR                       (volatile uint32_t *) (TIM2_BASE + 0x4C)                /* TIM2 DMA address for full transfer */
#define TIM2_OR                         (volatile uint32_t *) (TIM2_BASE + 0x50)                /* TIM2 option register */
/* Power Control (PWR) */
#define PWR_CR                          (volatile uint32_t *) (PWR_BASE + 0x00)                 /* Power Control Register */
#define PWR_CSR                         (volatile uint32_t *) (PWR_BASE + 0x04)                 /* Power Control/Status Register */

/* SPI */
#define SPI_BASE(port)                  (port)                              /* Temporary SPI base */
#define SPI_CR1(port)                   (volatile uint32_t *) (SPI_BASE(port) + 0x00)           /* SPI control register 1 */
#define SPI_CR2(port)                   (volatile uint32_t *) (SPI_BASE(port) + 0x04)           /* SPI control register 2 */
#define SPI_SR(port)                    (volatile uint32_t *) (SPI_BASE(port) + 0x08)           /* SPI status register */
#define SPI_DR(port)                    (volatile uint32_t *) (SPI_BASE(port) + 0x0c)           /* SPI data register */
#define SPI_CRCPR(port)                 (volatile uint32_t *) (SPI_BASE(port) + 0x10)           /* SPI CRC polynomial register */
#define SPI_RXCRCR(port)                (volatile uint32_t *) (SPI_BASE(port) + 0x14)           /* SPI RX CRC register */
#define SPI_TXCRCR(port)                (volatile uint32_t *) (SPI_BASE(port) + 0x18)           /* SPI TX CRC register */
#define SPI_I2SCFGR(port)               (volatile uint32_t *) (SPI_BASE(port) + 0x1c)           /* SPI I2C configuration register */
#define SPI_I2SPR(port)                 (volatile uint32_t *) (SPI_BASE(port) + 0x20)           /* SPI I2C prescaler register */

/* I2C */
#define I2C_BASE(port)                  (port)                                                  /* I2C Base */
#define I2C_CR1(port)                   (volatile uint32_t *) (I2C_BASE(port) + 0x00)           /* I2C control register 1 */
#define I2C_CR2(port)                   (volatile uint32_t *) (I2C_BASE(port) + 0x04)           /* I2C control register 2 */
#define I2C_OAR1(port)                  (volatile uint32_t *) (I2C_BASE(port) + 0x08)           /* I2C own address register 1 */
#define I2C_OAR2(port)                  (volatile uint32_t *) (I2C_BASE(port) + 0x0C)           /* I2C own address register 2 */
#define I2C_DR(port)                    (volatile uint32_t *) (I2C_BASE(port) + 0x10)           /* I2C data register */
#define I2C_SR1(port)                   (volatile uint32_t *) (I2C_BASE(port) + 0x14)           /* I2C status register 1 */
#define I2C_SR2(port)                   (volatile uint32_t *) (I2C_BASE(port) + 0x18)           /* I2C status register 2 */
#define I2C_CCR(port)                   (volatile uint32_t *) (I2C_BASE(port) + 0x1C)           /* I2C clock control register */
#define I2C_TRISE(port)                 (volatile uint32_t *) (I2C_BASE(port) + 0x20)           /* I2C TRISE register */

/* USART 1 */
#define USART1_SR                       (volatile uint32_t *) (USART1_BASE + 0x00)              /* USART1 status register */
#define USART1_DR                       (volatile uint32_t *) (USART1_BASE + 0x04)              /* USART1 data register */
#define USART1_BRR                      (volatile uint32_t *) (USART1_BASE + 0x08)              /* USART1 baud rate register */
#define USART1_CR1                      (volatile uint32_t *) (USART1_BASE + 0x0C)              /* USART1 control register 1 */
#define USART1_CR2                      (volatile uint32_t *) (USART1_BASE + 0x10)              /* USART1 control register 2 */
#define USART1_CR3                      (volatile uint32_t *) (USART1_BASE + 0x14)              /* USART1 control register 3 */
#define USART1_GTPR                     (volatile uint32_t *) (USART1_BASE + 0x18)              /* USART1 gaurd time and prescale register */

/* GPIO Port (GPIO) */
#define GPIO_MODER(port)                (volatile uint32_t *) (GPIO_BASE(port) + 0x00)          /* Port mode register */
#define GPIO_OTYPER(port)               (volatile uint32_t *) (GPIO_BASE(port) + 0x04)          /* Port output type register */
#define GPIO_OSPEEDR(port)              (volatile uint32_t *) (GPIO_BASE(port) + 0x08)          /* Port output speed register */
#define GPIO_PUPDR(port)                (volatile uint32_t *) (GPIO_BASE(port) + 0x0C)          /* Port pull up/down register */
#define GPIO_IDR(port)                  (volatile uint32_t *) (GPIO_BASE(port) + 0x10)          /* Port input data register */
#define GPIO_ODR(port)                  (volatile uint32_t *) (GPIO_BASE(port) + 0x14)          /* Port output data register */
#define GPIO_BSRR(port)                 (volatile uint32_t *) (GPIO_BASE(port) + 0x18)          /* Port bit set/reset register */
#define GPIO_LCKR(port)                 (volatile uint32_t *) (GPIO_BASE(port) + 0x1C)          /* Port configuration lock register */
#define GPIO_AFRL(port)                 (volatile uint32_t *) (GPIO_BASE(port) + 0x20)          /* Port alternate function low register */
#define GPIO_AFRH(port)                 (volatile uint32_t *) (GPIO_BASE(port) + 0x24)          /* Port alternate function high register */

/* Reset and Clock Control (RCC) */
#define RCC_CR                          (volatile uint32_t *) (RCC_BASE + 0x00)                 /* Clock Control Register */
#define RCC_PLLCFGR                     (volatile uint32_t *) (RCC_BASE + 0x04)                 /* PLL Configuration Register */
#define RCC_CFGR                        (volatile uint32_t *) (RCC_BASE + 0x08)                 /* Clock Configuration Register */
#define RCC_CIR                         (volatile uint32_t *) (RCC_BASE + 0x0C)                 /* Clock Interrupt Register */
#define RCC_AHB1RSTR                    (volatile uint32_t *) (RCC_BASE + 0x10)                 /* AHB1 reset Register */
#define RCC_APB1RSTR                    (volatile uint32_t *) (RCC_BASE + 0x20)                 /* APB1 reset Register */
#define RCC_APB2RSTR                    (volatile uint32_t *) (RCC_BASE + 0x24)                 /* APB2 reset Register */
#define RCC_AHB1ENR                     (volatile uint32_t *) (RCC_BASE + 0x30)                 /* AHB1 Enable Register */
#define RCC_AHB2ENR                     (volatile uint32_t *) (RCC_BASE + 0x34)                 /* AHB2 Enable Register */
#define RCC_AHB3ENR                     (volatile uint32_t *) (RCC_BASE + 0x38)                 /* AHB3 Enable Register */
#define RCC_APB1ENR                     (volatile uint32_t *) (RCC_BASE + 0x40)                 /* APB1 Peripheral Clock Enable Register */
#define RCC_APB2ENR                     (volatile uint32_t *) (RCC_BASE + 0x44)                 /* APB2 Peripheral Clock Enable Register */
#define RCC_BDCR                        (volatile uint32_t *) (RCC_BASE + 0x70)                 /* RCC Backup Domain Control Register */
#define RCC_CSR                         (volatile uint32_t *) (RCC_BASE + 0x74)                 /* RCC Clock Control & Status Register */
#define RCC_PLLSAICFGR                  (volatile uint32_t *) (RCC_BASE + 0x88)                 /* RCC PLLSAI Configuration register */
#define RCC_DCKCFGR                     (volatile uint32_t *) (RCC_BASE + 0x8C)                 /* RCC Dedicated Clocks Configuration register */

/* SYSCFG */
#define SYSCFG_MEMRMP					(volatile uint32_t *)(SYSCFG_BASE + 0x0)				/* Memory remap register */
#define SYSCFG_PMC						(volatile uint32_t *)(SYSCFG_BASE + 0x4)				/* Peripheral mode config register */
#define SYSCFG_EXTICR1					(volatile uint32_t *)(SYSCFG_BASE + 0x8)				/* External INT config register1 */
#define SYSCFG_EXTICR2					(volatile uint32_t *)(SYSCFG_BASE + 0xc)				/* External INT config register2 */
#define SYSCFG_EXTICR3					(volatile uint32_t *)(SYSCFG_BASE + 0x10)				/* External INT config register3 */
#define SYSCFG_EXTICR4					(volatile uint32_t *)(SYSCFG_BASE + 0x14)				/* External INT config register4 */
#define SYSCFG_CMPCR					(volatile uint32_t *)(SYSCFG_BASE + 0x20)				/* Compensation cell control register*/



/* APB1 RCC ENR */
#define RCC_TIM2_APBENR					RCC_APB1ENR
#define RCC_TIM3_APBENR					RCC_APB1ENR
#define RCC_TIM4_APBENR					RCC_APB1ENR
#define RCC_TIM5_APBENR					RCC_APB1ENR
#define RCC_TIM6_APBENR					RCC_APB1ENR
#define RCC_TIM7_APBENR					RCC_APB1ENR
#define RCC_TIM12_APBENR				RCC_APB1ENR
#define RCC_TIM13_APBENR				RCC_APB1ENR
#define RCC_TIM14_APBENR				RCC_APB1ENR
#define RCC_WWDG_APBENR					RCC_APB1ENR
#define RCC_SPI2_APBENR					RCC_APB1ENR
#define RCC_SPI3_APBENR					RCC_APB1ENR
#define RCC_USART2_APBENR				RCC_APB1ENR
#define RCC_USART3_APBENR				RCC_APB1ENR
#define RCC_UART4_APBENR				RCC_APB1ENR
#define RCC_UART5_APBENR				RCC_APB1ENR
#define RCC_I2C1_APBENR					RCC_APB1ENR
#define RCC_I2C2_APBENR					RCC_APB1ENR
#define RCC_I2C3_APBENR					RCC_APB1ENR
#define RCC_CAN1_APBENR					RCC_APB1ENR
#define RCC_CAN2_APBENR					RCC_APB1ENR
#define RCC_PWR_APBENR					RCC_APB1ENR
#define RCC_DAC_APBENR					RCC_APB1ENR

/* APB2 RCC ENR */
#define RCC_TIM1_APBENR					RCC_APB2ENR
#define RCC_TIM8_APBENR					RCC_APB2ENR
#define RCC_USART1_APBENR				RCC_APB2ENR
#define RCC_USART6_APBENR				RCC_APB2ENR
#define RCC_ADC1_APBENR					RCC_APB2ENR
#define RCC_ADC2_APBENR					RCC_APB2ENR
#define RCC_ADC3_APBENR					RCC_APB2ENR
#define RCC_SDIO_APBENR					RCC_APB2ENR
#define RCC_SPI1_APBENR					RCC_APB2ENR
#define RCC_SYSCFG_APBENR				RCC_APB2ENR
#define RCC_TIM9_APBENR					RCC_APB2ENR
#define RCC_TIM10_APBENR				RCC_APB2ENR
#define RCC_TIM11_APBENR				RCC_APB2ENR

/* Flash Registers (FLASH) */
#define FLASH_ACR                       (volatile uint32_t *) (FLASH_R_BASE + 0x00)             /* Flash Access Control Register */

/* Direct Memory Access 1 (DMA) */
#define DMA1_LISR                       (volatile uint32_t *) (DMA1_BASE + 0x00)                /* DMA1 low interrupt status register */
#define DMA1_HISR                       (volatile uint32_t *) (DMA1_BASE + 0x04)                /* DMA1 high interrupt status register */
#define DMA1_LIFCR                      (volatile uint32_t *) (DMA1_BASE + 0x08)                /* DMA1 low interrupt flag clear register */
#define DMA1_HIFCR                      (volatile uint32_t *) (DMA1_BASE + 0x0C)                /* DMA1 high interrupt flag clear register */
/* Stream n */
#define DMA1_CR_S(n)                    (volatile uint32_t *) (DMA1_BASE + 0x10 + (0x18*n))     /* DMA1 stream n configuration register */
#define DMA1_NDTR_S(n)                  (volatile uint32_t *) (DMA1_BASE + 0x14 + (0x18*n))     /* DMA1 stream n number of data register */
#define DMA1_PAR_S(n)                   (volatile uint32_t *) (DMA1_BASE + 0x18 + (0x18*n))     /* DMA1 stream n peripheral address register */
#define DMA1_M0AR_S(n)                  (volatile uint32_t *) (DMA1_BASE + 0x1C + (0x18*n))     /* DMA1 stream n memory 0 address register */
#define DMA1_M1AR_S(n)                  (volatile uint32_t *) (DMA1_BASE + 0x20 + (0x18*n))     /* DMA1 stream n memory 1 address register */
#define DMA1_FCR_S(n)                   (volatile uint32_t *) (DMA1_BASE + 0x24 + (0x18*n))     /* DMA1 stream n FIFO control register */

/* Direct Memory Access 2 (DMA) */
#define DMA2_LISR                       (volatile uint32_t *) (DMA2_BASE + 0x00)                /* DMA2 low interrupt status register */
#define DMA2_HISR                       (volatile uint32_t *) (DMA2_BASE + 0x04)                /* DMA2 high interrupt status register */
#define DMA2_LIFCR                      (volatile uint32_t *) (DMA2_BASE + 0x08)                /* DMA2 low interrupt flag clear register */
#define DMA2_HIFCR                      (volatile uint32_t *) (DMA2_BASE + 0x0C)                /* DMA2 high interrupt flag clear register */
/* Stream n */
#define DMA2_CR_S(n)                    (volatile uint32_t *) (DMA2_BASE + 0x10 + (0x18*n))     /* DMA2 stream n configuration register */
#define DMA2_NDTR_S(n)                  (volatile uint32_t *) (DMA2_BASE + 0x14 + (0x18*n))     /* DMA2 stream n number of data register */
#define DMA2_PAR_S(n)                   (volatile uint32_t *) (DMA2_BASE + 0x18 + (0x18*n))     /* DMA2 stream n peripheral address register */
#define DMA2_M0AR_S(n)                  (volatile uint32_t *) (DMA2_BASE + 0x1C + (0x18*n))     /* DMA2 stream n memory 0 address register */
#define DMA2_M1AR_S(n)                  (volatile uint32_t *) (DMA2_BASE + 0x20 + (0x18*n))     /* DMA2 stream n memory 1 address register */
#define DMA2_FCR_S(n)                   (volatile uint32_t *) (DMA2_BASE + 0x24 + (0x18*n))     /* DMA2 stream n FIFO control register */

/* DMA2D Registers */
#define DMA2D_CR                        (volatile uint32_t *) (DMA2D_BASE + 0x00)               /* DMA2D Control Register */
#define DMA2D_ISR                       (volatile uint32_t *) (DMA2D_BASE + 0x04)               /* DMA2D Interrupt Status Register */
#define DMA2D_IFCR                      (volatile uint32_t *) (DMA2D_BASE + 0x08)               /* DMA2D Interrupt Flag Clear Register */
#define DMA2D_FGMAR                     (volatile uint32_t *) (DMA2D_BASE + 0x0C)               /* DMA2D Foreground Memory Address Register */
#define DMA2D_FGOR                      (volatile uint32_t *) (DMA2D_BASE + 0x10)               /* DMA2D Foreground Offset Register */
#define DMA2D_BGMAR                     (volatile uint32_t *) (DMA2D_BASE + 0x14)               /* DMA2D Background Memory Address Register */
#define DMA2D_BGOR                      (volatile uint32_t *) (DMA2D_BASE + 0x18)               /* DMA2D Background Offset Register */
#define DMA2D_FGPFCCR                   (volatile uint32_t *) (DMA2D_BASE + 0x1C)               /* DMA2D Foreground PFC Control Register */
#define DMA2D_FGCOLR                    (volatile uint32_t *) (DMA2D_BASE + 0x20)               /* DMA2D Foreground Color Control Register */
#define DMA2D_BGPFCCR                   (volatile uint32_t *) (DMA2D_BASE + 0x24)               /* DMA2D Background PFC Control Register */
#define DMA2D_BGCOLR                    (volatile uint32_t *) (DMA2D_BASE + 0x28)               /* DMA2D Background Color Control Register */
#define DMA2D_FGCMAR                    (volatile uint32_t *) (DMA2D_BASE + 0x2C)               /* DMA2D Foreground CLUT Memory Address Register */
#define DMA2D_BGCMAR                    (volatile uint32_t *) (DMA2D_BASE + 0x30)               /* DMA2D Background CLUT Memory Address Register */
#define DMA2D_OPFCCR                    (volatile uint32_t *) (DMA2D_BASE + 0x34)               /* DMA2D Output PFC Control Register */
#define DMA2D_OCOLR                     (volatile uint32_t *) (DMA2D_BASE + 0x38)               /* DMA2D Output Color Register */
#define DMA2D_OMAR                      (volatile uint32_t *) (DMA2D_BASE + 0x3C)               /* DMA2D Output Memory Address Register */
#define DMA2D_OOR                       (volatile uint32_t *) (DMA2D_BASE + 0x40)               /* DMA2D Output Offset Register */
#define DMA2D_NLR                       (volatile uint32_t *) (DMA2D_BASE + 0x44)               /* DMA2D Number of Line Register */
#define DMA2D_LWR                       (volatile uint32_t *) (DMA2D_BASE + 0x48)               /* DMA2D Line Watermark Register */
#define DMA2D_AMTCR                     (volatile uint32_t *) (DMA2D_BASE + 0x4C)               /* DMA2D AHB Master Timer Configuration Register */

/* USB OTG Full-Speed */
/* Global Control and Status Registers */
#define USB_FS_GOTGCTL                  (volatile uint32_t *) (USB_FS_BASE + 0x0000)            /* USB control and status register */
#define USB_FS_GOTGINT                  (volatile uint32_t *) (USB_FS_BASE + 0x0004)            /* USB interrupt register */
#define USB_FS_GAHBCFG                  (volatile uint32_t *) (USB_FS_BASE + 0x0008)            /* USB AHB configuration register */
#define USB_FS_GUSBCFG                  (volatile uint32_t *) (USB_FS_BASE + 0x000C)            /* USB USB configuration register */
#define USB_FS_GRSTCTL                  (volatile uint32_t *) (USB_FS_BASE + 0x0010)            /* USB reset control register */
#define USB_FS_GINTSTS                  (volatile uint32_t *) (USB_FS_BASE + 0x0014)            /* USB core interrupt register */
#define USB_FS_GINTMSK                  (volatile uint32_t *) (USB_FS_BASE + 0x0018)            /* USB interrupt mask register */
#define USB_FS_GRXSTSR                  (volatile uint32_t *) (USB_FS_BASE + 0x001C)            /* USB receive status debug read register */
#define USB_FS_GRXSTSP                  (volatile uint32_t *) (USB_FS_BASE + 0x0020)            /* USB receive status debug read and pop register */
#define USB_FS_GRXFSIZ                  (volatile uint32_t *) (USB_FS_BASE + 0x0024)            /* USB receive FIFO size register */
#define USB_FS_DIEPTXF0                 (volatile uint32_t *) (USB_FS_BASE + 0x0028)            /* USB endpoint 0 transmit FIFO size register */
#define USB_FS_HNPTXFSIZ                (volatile uint32_t *) (USB_FS_BASE + 0x0028)            /* USB host non-periodic transmit FIFO size register */
#define USB_FS_HNPTXSTS                 (volatile uint32_t *) (USB_FS_BASE + 0x002C)            /* USB host non-periodic transmit FIFO/queue status register */
#define USB_FS_GCCFG                    (volatile uint32_t *) (USB_FS_BASE + 0x0038)            /* USB general core configuration register */
#define USB_FS_CID                      (volatile uint32_t *) (USB_FS_BASE + 0x003C)            /* USB core ID register */
#define USB_FS_HPTXFSIZ                 (volatile uint32_t *) (USB_FS_BASE + 0x0100)            /* USB host periodic transmit FIFO size register */
#define USB_FS_DIEPTXF1                 (volatile uint32_t *) (USB_FS_BASE + 0x0104)            /* USB device IN endpoint transmit FIFO size register 1 */
#define USB_FS_DIEPTXF2                 (volatile uint32_t *) (USB_FS_BASE + 0x0108)            /* USB device IN endpoint transmit FIFO size register 2 */
#define USB_FS_DIEPTXF3                 (volatile uint32_t *) (USB_FS_BASE + 0x010C)            /* USB device IN endpoint transmit FIFO size register 3 */

/* Host-mode Registers */
/* ... */

/* Device-mode Registers */
#define USB_FS_DCFG                     (volatile uint32_t *) (USB_FS_BASE + 0x0800)            /* USB device configuration register */
#define USB_FS_DCTL                     (volatile uint32_t *) (USB_FS_BASE + 0x0804)            /* USB device control register */
#define USB_FS_DSTS                     (volatile uint32_t *) (USB_FS_BASE + 0x0808)            /* USB device status register */
#define USB_FS_DSTS                     (volatile uint32_t *) (USB_FS_BASE + 0x0808)            /* USB device status register */
#define USB_FS_DIEPMSK                  (volatile uint32_t *) (USB_FS_BASE + 0x0810)            /* USB device IN endpoint common interrupt mask register */
#define USB_FS_DOEPMSK                  (volatile uint32_t *) (USB_FS_BASE + 0x0814)            /* USB device OUT endpoint common interrupt mask register */
#define USB_FS_DAINT                    (volatile uint32_t *) (USB_FS_BASE + 0x0818)            /* USB device all endpoints interrupt register */
#define USB_FS_DAINTMSK                 (volatile uint32_t *) (USB_FS_BASE + 0x081C)            /* USB device all endpoints interrupt mask register */
#define USB_FS_DVBUSDIS                 (volatile uint32_t *) (USB_FS_BASE + 0x0828)            /* USB device VBUS discharge time register */
#define USB_FS_DVBUSPULSE               (volatile uint32_t *) (USB_FS_BASE + 0x082C)            /* USB device VBUS pulse time register */
#define USB_FS_DIEPEMPMSK               (volatile uint32_t *) (USB_FS_BASE + 0x0834)            /* USB device IN endpoint FIFO empty interrupt mask register */
#define USB_FS_DIEPCTL0                 (volatile uint32_t *) (USB_FS_BASE + 0x0900)            /* USB device IN endpoint 0 control register */
#define USB_FS_DIEPCTL(n)               (volatile uint32_t *) (USB_FS_BASE + 0x0900 + n*0x20)   /* USB device endpoint n control register */
#define USB_FS_DIEPINT(n)               (volatile uint32_t *) (USB_FS_BASE + 0x0908 + n*0x20)   /* USB device endpoint n interrupt register */
#define USB_FS_DIEPTSIZ0                (volatile uint32_t *) (USB_FS_BASE + 0x0910)            /* USB device IN endpoint 0 transfer size register */
#define USB_FS_DIEPTSIZ(n)              (volatile uint32_t *) (USB_FS_BASE + 0x0910 + n*0x20)   /* USB device OUT endpoint n transfer size register */
#define USB_FS_DTXFSTS(n)               (volatile uint32_t *) (USB_FS_BASE + 0x0918 + n*0x20)   /* USB device IN endpoint n FIFO status register */
#define USB_FS_DOEPCTL0                 (volatile uint32_t *) (USB_FS_BASE + 0x0B00)            /* USB device OUT endpoint 0 control register */
#define USB_FS_DOEPCTL(n)               (volatile uint32_t *) (USB_FS_BASE + 0x0B00 + n*0x20)   /* USB device endpoint n control register */
#define USB_FS_DOEPINT(n)               (volatile uint32_t *) (USB_FS_BASE + 0x0B08 + n*0x20)   /* USB device endpoint 1 interrupt register */
#define USB_FS_DOEPTSIZ0                (volatile uint32_t *) (USB_FS_BASE + 0x0B10)            /* USB device OUT endpoint 0 transfer size register */
#define USB_FS_DOEPTSIZ(n)              (volatile uint32_t *) (USB_FS_BASE + 0x0B10 + n*0x20)   /* USB device OUT endpoint n transfer size register */

/* Data FIFO Registers */
#define USB_FS_DFIFO_EP(n)              (volatile uint32_t *) (USB_FS_BASE + 0x1000 + n*0x1000) /* USB endpoint n data FIFO base address */

/* Power and Clock Gating Registers */
#define USB_FS_PCGCR                    (volatile uint32_t *) (USB_FS_BASE + 0x0E00)            /* USB power and clock gating control register */


/**********************************************************************************************************************************************/

/* Bit Masks - See RM0090 Reference Manual for STM32F4 for details */
#define PWR_CR_VOS                      (uint16_t) (1 << 14)                                    /* Regulator voltage scaling output selection */

#define HSI_VALUE                       (uint32_t) (16000000)                                   /* HSI value in Hz */
#define HSE_VALUE                       (uint32_t) (1 << 27)                                    /* HSE value in Hz */

#define RCC_CR_HSION                    (uint32_t) (1 << 0)                                     /* HSI clock enable */
#define RCC_CR_HSIRDY                   (uint32_t) (1 << 1)                                     /* HSI ready */
#define RCC_CR_HSITRIM_M                (uint32_t) (0x1F << 3)                                  /* HSI trimming mask */
#define RCC_CR_HSITRIM(n)               (uint32_t) (n << 3)                                     /* HSI trimming */
#define RCC_CR_HSICAL_M                 (uint32_t) (0xFF << 8)                                  /* HSI calibration mask */
#define RCC_CR_HSEON                    (uint32_t) (1 << 16)                                    /* HSE clock enable */
#define RCC_CR_HSERDY                   (uint32_t) (1 << 17)                                    /* HSE ready */
#define RCC_CR_HSEBYP                   (uint32_t) (1 << 18)                                    /* HSE bypass */
#define RCC_CR_CSSON                    (uint32_t) (1 << 19)                                    /* Clock security system enable */
#define RCC_CR_PLLON                    (uint32_t) (1 << 24)                                    /* Main PLL enable */
#define RCC_CR_PLLRDY                   (uint32_t) (1 << 25)                                    /* Main PLL clock ready */
#define RCC_CR_PLLI2SON                 (uint32_t) (1 << 26)                                    /* PLLI2S enable */
#define RCC_CR_PLLI2SRDY                (uint32_t) (1 << 27)                                    /* PLLI2S clock ready */

#define RCC_PLLCFGR_RESET               (uint32_t) (0x24003010)                                 /* PLLCFGR register reset value */
#define RCC_PLLCFGR_PLLSRC_HSE          (uint32_t) (1 << 22)                                    /* HSE oscillator selected as clock entry */
#define RCC_PLLCFGR_PLLM                (uint32_t) (0x003F)
#define RCC_PLLCFGR_PLLN                (uint32_t) (0x7FC0)
#define RCC_PLLCFGR_PLLP                (uint32_t) (0x00030000)

#define RCC_CFGR_SW_M                   (uint32_t) (3 << 0)                                     /* System clock switch mask */
#define RCC_CFGR_SW_HSI                 (uint32_t) (0 << 0)                                     /* System clock switch - HSI selected as system clock */
#define RCC_CFGR_SW_HSE                 (uint32_t) (1 << 0)                                     /* System clock switch - HSE selected as system clock */
#define RCC_CFGR_SW_PLL                 (uint32_t) (2 << 0)                                     /* System clock switch - PLL selected as system clock */
#define RCC_CFGR_SWS_M                  (uint32_t) (3 << 2)                                     /* System clock switch status mask */
#define RCC_CFGR_SWS_HSI                (uint32_t) (0 << 2)                                     /* System clock switch status - HSI used as system clock */
#define RCC_CFGR_SWS_HSE                (uint32_t) (1 << 2)                                     /* System clock switch status - HSE used as system clock */
#define RCC_CFGR_SWS_PLL                (uint32_t) (2 << 2)                                     /* System clock switch status - PLL used as system clock */
#define RCC_CFGR_HPRE_M                 (uint32_t) (0xF << 4)                                   /* AHB prescaler mask */
#define RCC_CFGR_HPRE_DIV1              (uint32_t) (0x0 << 4)                                   /* AHB prescaler - SYSCLK not divided */
#define RCC_CFGR_HPRE_DIV2              (uint32_t) (0x8 << 4)                                   /* AHB prescaler - SYSCLK/2 */
#define RCC_CFGR_HPRE_DIV4              (uint32_t) (0x9 << 4)                                   /* AHB prescaler - SYSCLK/4 */
#define RCC_CFGR_HPRE_DIV8              (uint32_t) (0xA << 4)                                   /* AHB prescaler - SYSCLK/8 */
#define RCC_CFGR_HPRE_DIV16             (uint32_t) (0xB << 4)                                   /* AHB prescaler - SYSCLK/16 */
#define RCC_CFGR_HPRE_DIV64             (uint32_t) (0xC << 4)                                   /* AHB prescaler - SYSCLK/64 */
#define RCC_CFGR_HPRE_DIV128            (uint32_t) (0xD << 4)                                   /* AHB prescaler - SYSCLK/128 */
#define RCC_CFGR_HPRE_DIV256            (uint32_t) (0xE << 4)                                   /* AHB prescaler - SYSCLK/256 */
#define RCC_CFGR_HPRE_DIV512            (uint32_t) (0xF << 4)                                   /* AHB prescaler - SYSCLK/512 */
#define RCC_CFGR_PPRE1_M                (uint32_t) (7 << 10)                                    /* APB low speed prescaler mask */
#define RCC_CFGR_PPRE1_DIV1             (uint32_t) (0 << 10)                                    /* APB low speed prescaler - HCLK/1 */
#define RCC_CFGR_PPRE1_DIV2             (uint32_t) (4 << 10)                                    /* APB low speed prescaler - HCLK/2 */
#define RCC_CFGR_PPRE1_DIV4             (uint32_t) (5 << 10)                                    /* APB low speed prescaler - HCLK/4 */
#define RCC_CFGR_PPRE1_DIV8             (uint32_t) (6 << 10)                                    /* APB low speed prescaler - HCLK/8 */
#define RCC_CFGR_PPRE1_DIV16            (uint32_t) (7 << 10)                                    /* APB low speed prescaler - HCLK/16 */
#define RCC_CFGR_PPRE2_M                (uint32_t) (7 << 13)                                    /* APB high speec prescaler mask */
#define RCC_CFGR_PPRE2_DIV1             (uint32_t) (0 << 13)                                    /* APB high speed prescaler - HCLK/1 */
#define RCC_CFGR_PPRE2_DIV2             (uint32_t) (4 << 13)                                    /* APB high speed prescaler - HCLK/2 */
#define RCC_CFGR_PPRE2_DIV4             (uint32_t) (5 << 13)                                    /* APB high speed prescaler - HCLK/4 */
#define RCC_CFGR_PPRE2_DIV8             (uint32_t) (6 << 13)                                    /* APB high speed prescaler - HCLK/8 */
#define RCC_CFGR_PPRE2_DIV16            (uint32_t) (7 << 13)                                    /* APB high speed prescaler - HCLK/16 */
#define RCC_CFGR_RTCPRE_M               (uint32_t) (0x1F << 16)                                 /* HSE division factor for RTC clock mask */
#define RCC_CFGR_RTCPRE(n)              (uint32_t) (n << 16)                                    /* HSE division factor for RTC clock */

#define RCC_AHB1RSTR_GPIOARST           (uint32_t) (1 << 0)                                     /* GPIOA reset */
#define RCC_AHB1RSTR_GPIOBRST           (uint32_t) (1 << 1)                                     /* GPIOB reset */
#define RCC_AHB1RSTR_GPIOCRST           (uint32_t) (1 << 2)                                     /* GPIOC reset */
#define RCC_AHB1RSTR_GPIODRST           (uint32_t) (1 << 3)                                     /* GPIOD reset */
#define RCC_AHB1RSTR_GPIOERST           (uint32_t) (1 << 4)                                     /* GPIOE reset */
#define RCC_AHB1RSTR_GPIOFRST           (uint32_t) (1 << 5)                                     /* GPIOF reset */
#define RCC_AHB1RSTR_GPIOGRST           (uint32_t) (1 << 6)                                     /* GPIOG reset */
#define RCC_AHB1RSTR_GPIOHRST           (uint32_t) (1 << 7)                                     /* GPIOH reset */
#define RCC_AHB1RSTR_GPIOIRST           (uint32_t) (1 << 8)                                     /* GPIOI reset */
#define RCC_AHB1RSTR_CRCRST             (uint32_t) (1 << 12)                                    /* CRC reset */
#define RCC_AHB1RSTR_DMA1RST            (uint32_t) (1 << 21)                                    /* DMA1 reset */
#define RCC_AHB1RSTR_DMA2RST            (uint32_t) (1 << 22)                                    /* DMA2 reset */
#define RCC_AHB1RSTR_ETHMACRST          (uint32_t) (1 << 25)                                    /* Ethernet MAC reset */
#define RCC_AHB1RSTR_OTGHSRST           (uint32_t) (1 << 29)                                    /* USB OTG HS reset */

#define RCC_AHB2RSTR_DCMIRST            (uint32_t) (1 << 0)                                     /* Camera interface reset */
#define RCC_AHB2RSTR_CRYPRST            (uint32_t) (1 << 4)                                     /* Cyrpto modules reset */
#define RCC_AHB2RSTR_HASHRST            (uint32_t) (1 << 5)                                     /* Hash modules reset */
#define RCC_AHB2RSTR_RNGRST             (uint32_t) (1 << 6)                                     /* Random number generator reset */
#define RCC_AHB2RSTR_OTGFSRST           (uint32_t) (1 << 7)                                     /* USB OTG FS reset */

#define RCC_AHB3RSTR_FSMCRST            (uint32_t) (1 << 0)                                     /* Flexible static memeory controller reset */

#define RCC_APB1RSTR_TIM2RST            (uint32_t) (1 << 0)                                     /* TIM2 reset */
#define RCC_APB1RSTR_TIM3RST            (uint32_t) (1 << 1)                                     /* TIM3 reset */
#define RCC_APB1RSTR_TIM4RST            (uint32_t) (1 << 2)                                     /* TIM4 reset */
#define RCC_APB1RSTR_TIM5RST            (uint32_t) (1 << 3)                                     /* TIM5 reset */
#define RCC_APB1RSTR_TIM6RST            (uint32_t) (1 << 4)                                     /* TIM6 reset */
#define RCC_APB1RSTR_TIM7RST            (uint32_t) (1 << 5)                                     /* TIM7 reset */
#define RCC_APB1RSTR_TIM12RST           (uint32_t) (1 << 6)                                     /* TIM12 reset */
#define RCC_APB1RSTR_TIM13RST           (uint32_t) (1 << 7)                                     /* TIM13 reset */
#define RCC_APB1RSTR_TIM14RST           (uint32_t) (1 << 8)                                     /* TIM14 reset */
#define RCC_APB1RSTR_WWDGRST            (uint32_t) (1 << 11)                                    /* Window watchdog reset */
#define RCC_APB1RSTR_SPI2RST            (uint32_t) (1 << 14)                                    /* SPI2 reset */
#define RCC_APB1RSTR_SPI3RST            (uint32_t) (1 << 15)                                    /* SPI3 reset */
#define RCC_APB1RSTR_USART2RST          (uint32_t) (1 << 17)                                    /* USART2 reset */
#define RCC_APB1RSTR_USART3RST          (uint32_t) (1 << 18)                                    /* USART3 reset */
#define RCC_APB1RSTR_USART4RST          (uint32_t) (1 << 19)                                    /* USART4 reset */
#define RCC_APB1RSTR_USART5RST          (uint32_t) (1 << 20)                                    /* USART5 reset */
#define RCC_APB1RSTR_I2C1RST            (uint32_t) (1 << 21)                                    /* I2C1 reset */
#define RCC_APB1RSTR_I2C2RST            (uint32_t) (1 << 22)                                    /* I2C2 reset */
#define RCC_APB1RSTR_I2C3RST            (uint32_t) (1 << 23)                                    /* I2C3 reset */
#define RCC_APB1RSTR_CAN1RST            (uint32_t) (1 << 25)                                    /* CAN1 reset */
#define RCC_APB1RSTR_CAN2RST            (uint32_t) (1 << 26)                                    /* CAN2 reset */
#define RCC_APB1RSTR_PWRRST             (uint32_t) (1 << 28)                                    /* Power interface reset */
#define RCC_APB1RSTR_DACRST             (uint32_t) (1 << 29)                                    /* DAC reset */

#define RCC_APB2RSTR_TIM1RST            (uint32_t) (1 << 0)                                     /* TIM1 reset */
#define RCC_APB2RSTR_TIM8RST            (uint32_t) (1 << 1)                                     /* TIM8 reset */
#define RCC_APB2RSTR_USART1RST          (uint32_t) (1 << 4)                                     /* USART1 reset */
#define RCC_APB2RSTR_USART6RST          (uint32_t) (1 << 5)                                     /* USART6 reset */
#define RCC_APB2RSTR_ADCRST             (uint32_t) (1 << 8)                                     /* ADC1 reset */
#define RCC_APB2RSTR_SDIORST            (uint32_t) (1 << 11)                                    /* SDIO reset */
#define RCC_APB2RSTR_SPI1RST            (uint32_t) (1 << 12)                                    /* SPI1 reset */
#define RCC_APB2RSTR_SPI5RST            (uint32_t) (1 << 20)                                    /* SPI5 reset */
#define RCC_APB2RSTR_SYSCFGRST          (uint32_t) (1 << 14)                                    /* System configuration controller reset */
#define RCC_APB2RSTR_TIM9RST            (uint32_t) (1 << 16)                                    /* TIM9 reset */
#define RCC_APB2RSTR_TIM10RST           (uint32_t) (1 << 17)                                    /* TIM10 reset */
#define RCC_APB2RSTR_TIM11RST           (uint32_t) (1 << 18)                                    /* TIM11 reset */
#define RCC_APB2RSTR_LTDC               (uint32_t) (1 << 26)                                    /* LTDC reset */

#define RCC_AHB1ENR_GPIOAEN             (uint32_t) (1 << 0)                                     /* GPIOA clock enable */
#define RCC_AHB1ENR_GPIOBEN             (uint32_t) (1 << 1)                                     /* GPIOB clock enable */
#define RCC_AHB1ENR_GPIOCEN             (uint32_t) (1 << 2)                                     /* GPIOC clock enable */
#define RCC_AHB1ENR_GPIODEN             (uint32_t) (1 << 3)                                     /* GPIOD clock enable */
#define RCC_AHB1ENR_GPIOEEN             (uint32_t) (1 << 4)                                     /* GPIOE clock enable */
#define RCC_AHB1ENR_GPIOFEN             (uint32_t) (1 << 5)                                     /* GPIOF clock enable */
#define RCC_AHB1ENR_GPIOGEN             (uint32_t) (1 << 6)                                     /* GPIOG clock enable */
#define RCC_AHB1ENR_GPIOHEN             (uint32_t) (1 << 7)                                     /* GPIOH clock enable */
#define RCC_AHB1ENR_GPIOIEN             (uint32_t) (1 << 8)                                     /* GPIOI clock enable */
#define RCC_AHB1ENR_CRCEN               (uint32_t) (1 << 12)                                    /* CRC clock enable */
#define RCC_AHB1ENR_BKPSRAMEN           (uint32_t) (1 << 18)                                    /* Backup SRAM clock enable */
#define RCC_AHB1ENR_CCMDATARAMEN        (uint32_t) (1 << 20)                                    /* CCM data RAM clock enable */
#define RCC_AHB1ENR_DMA1EN              (uint32_t) (1 << 21)                                    /* DMA1 clock enable */
#define RCC_AHB1ENR_DMA2EN              (uint32_t) (1 << 22)                                    /* DMA2 clock enable */
#define RCC_AHB1ENR_DMA2DEN             (uint32_t) (1 << 23)                                    /* DMA2D clock enable */
#define RCC_AHB1ENR_ETHMACEN            (uint32_t) (1 << 25)                                    /* Ethernet MAC clock enable */
#define RCC_AHB1ENR_ETHMACTXEN          (uint32_t) (1 << 26)                                    /* Ethernet MAC TX clock enable */
#define RCC_AHB1ENR_ETHMACRXEN          (uint32_t) (1 << 27)                                    /* Ethernet MAC RX clock enable */
#define RCC_AHB1ENR_ETHMACPTPEN         (uint32_t) (1 << 28)                                    /* Ethernet MAC PTP clock enable */
#define RCC_AHB1ENR_OTGHSEN             (uint32_t) (1 << 29)                                    /* USB OTG HS clock enable */
#define RCC_AHB1ENR_OTGHSULPIEN         (uint32_t) (1 << 30)                                    /* USB OTG HSULPI clock enable */

#define RCC_AHB2ENR_DCMIEN              (uint32_t) (1 << 0)                                     /* Camera interface clock enable */
#define RCC_AHB2ENR_CRYPEN              (uint32_t) (1 << 4)                                     /* Cyrpto modules clock enable */
#define RCC_AHB2ENR_HASHEN              (uint32_t) (1 << 5)                                     /* Hash modules clock enable */
#define RCC_AHB2ENR_RNGEN               (uint32_t) (1 << 6)                                     /* Random number generator clock enable */
#define RCC_AHB2ENR_OTGFSEN             (uint32_t) (1 << 7)                                     /* USB OTG FS clock enable */

#define RCC_AHB3ENR_FSMCEN              (uint32_t) (1 << 0)                                     /* Flexible static memeory controller clock enable */
#define RCC_AHB3ENR_FMCEN               (uint32_t) (1 << 0)                                     /* FMC clock enable */

#define RCC_APB1ENR_TIM2EN              (uint32_t) (1 << 0)                                     /* TIM2 clock enable */
#define RCC_APB1ENR_TIM3EN              (uint32_t) (1 << 1)                                     /* TIM3 clock enable */
#define RCC_APB1ENR_TIM4EN              (uint32_t) (1 << 2)                                     /* TIM4 clock enable */
#define RCC_APB1ENR_TIM5EN              (uint32_t) (1 << 3)                                     /* TIM5 clock enable */
#define RCC_APB1ENR_TIM6EN              (uint32_t) (1 << 4)                                     /* TIM6 clock enable */
#define RCC_APB1ENR_TIM7EN              (uint32_t) (1 << 5)                                     /* TIM7 clock enable */
#define RCC_APB1ENR_TIM12EN             (uint32_t) (1 << 6)                                     /* TIM12 clock enable */
#define RCC_APB1ENR_TIM13EN             (uint32_t) (1 << 7)                                     /* TIM13 clock enable */
#define RCC_APB1ENR_TIM14EN             (uint32_t) (1 << 8)                                     /* TIM14 clock enable */
#define RCC_APB1ENR_WWDGEN              (uint32_t) (1 << 11)                                    /* Window watchdog clock enable */
#define RCC_APB1ENR_SPI2EN              (uint32_t) (1 << 14)                                    /* SPI2 clock enable */
#define RCC_APB1ENR_SPI3EN              (uint32_t) (1 << 15)                                    /* SPI3 clock enable */
#define RCC_APB1ENR_USART2EN            (uint32_t) (1 << 17)                                    /* USART2 clock enable */
#define RCC_APB1ENR_USART3EN            (uint32_t) (1 << 18)                                    /* USART3 clock enable */
#define RCC_APB1ENR_USART4EN            (uint32_t) (1 << 19)                                    /* USART4 clock enable */
#define RCC_APB1ENR_USART5EN            (uint32_t) (1 << 20)                                    /* USART5 clock enable */
#define RCC_APB1ENR_I2C1EN              (uint32_t) (1 << 21)                                    /* I2C1 clock enable */
#define RCC_APB1ENR_I2C2EN              (uint32_t) (1 << 22)                                    /* I2C2 clock enable */
#define RCC_APB1ENR_I2C3EN              (uint32_t) (1 << 23)                                    /* I2C3 clock enable */
#define RCC_APB1ENR_CAN1EN              (uint32_t) (1 << 25)                                    /* CAN1 clock enable */
#define RCC_APB1ENR_CAN2EN              (uint32_t) (1 << 26)                                    /* CAN2 clock enable */
#define RCC_APB1ENR_PWREN               (uint32_t) (1 << 28)                                    /* Power interface clock enable */
#define RCC_APB1ENR_DACEN               (uint32_t) (1 << 29)                                    /* DAC clock enable */

#define RCC_APB2ENR_TIM1EN              (uint32_t) (1 << 0)                                     /* TIM1 clock enable */
#define RCC_APB2ENR_TIM8EN              (uint32_t) (1 << 1)                                     /* TIM8 clock enable */
#define RCC_APB2ENR_USART1EN            (uint32_t) (1 << 4)                                     /* USART1 clock enable */
#define RCC_APB2ENR_USART2EN            (uint32_t) (1 << 5)                                     /* USART2 clock enable */
#define RCC_APB2ENR_ADC1EN              (uint32_t) (1 << 8)                                     /* ADC1 clock enable */
#define RCC_APB2ENR_ADC2EN              (uint32_t) (1 << 9)                                     /* ADC2 clock enable */
#define RCC_APB2ENR_ADC3EN              (uint32_t) (1 << 10)                                    /* ADC3 clock enable */
#define RCC_APB2ENR_SDIOEN              (uint32_t) (1 << 11)                                    /* SDIO clock enable */
#define RCC_APB2ENR_SPI1EN              (uint32_t) (1 << 12)                                    /* SPI1 clock enable */
#define RCC_APB2ENR_SYSCFGEN            (uint32_t) (1 << 14)                                    /* System configuration controller clock enable */
#define RCC_APB2ENR_TIM9EN              (uint32_t) (1 << 16)                                    /* TIM9 clock enable */
#define RCC_APB2ENR_TIM10EN             (uint32_t) (1 << 17)                                    /* TIM10 clock enable */
#define RCC_APB2ENR_TIM11EN             (uint32_t) (1 << 18)                                    /* TIM11 clock enable */
#define RCC_APB2ENR_SPI5EN              (uint32_t) (1 << 20)                                    /* SPI5 clock enable */
#define RCC_APB2ENR_LTDCEN              (uint32_t) (1 << 26)                                    /* LTDC clock enable */

#define FLASH_ACR_PRFTEN                (uint32_t) (1 << 8)                                     /* Prefetch enable */
#define FLASH_ACR_ICEN                  (uint32_t) (1 << 9)                                     /* Instruction cache enable */
#define FLASH_ACR_DCEN                  (uint32_t) (1 << 10)                                    /* Data cache enable */
#define FLASH_ACR_ICRST                 (uint32_t) (1 << 11)                                    /* Instruction cache reset */
#define FLASH_ACR_CCRST                 (uint32_t) (1 << 12)                                    /* Data cache reset */
#define FLASH_ACR_LATENCY_M             (uint32_t) (7 << 0)                                     /* Latency mask */
#define FLASH_ACR_LATENCY(n)            (uint32_t) (n << 0)                                     /* Latency - n wait states */

/* TIMx */
#define TIMx_CR1_CEN                    (uint32_t) (1 << 0)                                     /* TIMx counter enable */
#define TIMx_CR1_UDIS                   (uint32_t) (1 << 1)                                     /* TIMx update disable */
#define TIMx_CR1_URS                    (uint32_t) (1 << 2)                                     /* TIMx update request source */
#define TIMx_CR1_OPM                    (uint32_t) (1 << 3)                                     /* TIMx one-pulse mode */
#define TIMx_CR1_DIR_DOWN               (uint32_t) (1 << 4)                                     /* TIMx downcounter */
#define TIMx_CR1_CMS_EDGE               (uint32_t) (0 << 5)                                     /* TIMx center-aligned mode selection - counter up or down depending on DIR bit */
#define TIMx_CR1_CMS_CM1                (uint32_t) (1 << 5)                                     /* TIMx center-aligned mode selection - up and down, compare flags set down */
#define TIMx_CR1_CMS_CM2                (uint32_t) (2 << 5)                                     /* TIMx center-aligned mode selection - up and down, compare flags set up */
#define TIMx_CR1_CMS_CM3                (uint32_t) (3 << 5)                                     /* TIMx center-aligned mode selection - up and down, compare flags set up/down */
#define TIMx_CR1_ARPE                   (uint32_t) (1 << 7)                                     /* TIMx auto-reload preload enable */
#define TIMx_CR1_CKD_1                  (uint32_t) (0 << 8)                                     /* TIMx clock division 1 */
#define TIMx_CR1_CKD_2                  (uint32_t) (1 << 8)                                     /* TIMx clock division 2 */
#define TIMx_CR1_CKD_4                  (uint32_t) (2 << 8)                                     /* TIMx clock division 4 */

#define TIMx_CR2_CCDS                   (uint32_t) (1 << 3)                                     /* TIMx capture/compare DMA requests send when update event occurs */
#define TIMx_CR2_MMS_RST                (uint32_t) (0 << 4)                                     /* TIMx master mode - reset */
#define TIMx_CR2_MMS_EN                 (uint32_t) (1 << 4)                                     /* TIMx master mode - enable */
#define TIMx_CR2_MMS_UP                 (uint32_t) (2 << 4)                                     /* TIMx master mode - update */
#define TIMx_CR2_MMS_CMP_PUL            (uint32_t) (3 << 4)                                     /* TIMx master mode - compare pulse */
#define TIMx_CR2_MMS_CMP_OC1            (uint32_t) (4 << 4)                                     /* TIMx master mode - compare OC1 */
#define TIMx_CR2_MMS_CMP_OC2            (uint32_t) (5 << 4)                                     /* TIMx master mode - compare OC2 */
#define TIMx_CR2_MMS_CMP_OC3            (uint32_t) (6 << 4)                                     /* TIMx master mode - compare OC3 */
#define TIMx_CR2_MMS_CMP_OC4            (uint32_t) (7 << 4)                                     /* TIMx master mode - compare OC4 */
#define TIMx_CR2_TI1_123                (uint32_t) (1 << 7)                                     /* TIMx CH1, CH2, CH3 pins connected to TI1 */

#define TIMx_DIER_UIE                   (uint32_t) (1 << 0)                                     /* TIMx update interrupt enable */
#define TIMx_DIER_CC1IE                 (uint32_t) (1 << 1)                                     /* TIMx CC1 interrupt enable */
#define TIMx_DIER_CC2IE                 (uint32_t) (1 << 2)                                     /* TIMx CC2 interrupt enable */
#define TIMx_DIER_CC3IE                 (uint32_t) (1 << 3)                                     /* TIMx CC3 interrupt enable */
#define TIMx_DIER_CC4IE                 (uint32_t) (1 << 4)                                     /* TIMx CC4 interrupt enable */
#define TIMx_DIER_TIE                   (uint32_t) (1 << 6)                                     /* TIMx trigger interrupt enable */
#define TIMx_DIER_UDE                   (uint32_t) (1 << 8)                                     /* TIMx update DMA request enable */
#define TIMx_DIER_CC1DE                 (uint32_t) (1 << 9)                                     /* TIMx CC1 DMA request enable */
#define TIMx_DIER_CC2DE                 (uint32_t) (1 << 10)                                    /* TIMx CC2 DMA request enable */
#define TIMx_DIER_CC3DE                 (uint32_t) (1 << 11)                                    /* TIMx CC3 DMA request enable */
#define TIMx_DIER_CC4DE                 (uint32_t) (1 << 12)                                    /* TIMx CC4 DMA request enable */
#define TIMx_DIER_TDE                   (uint32_t) (1 << 14)                                    /* TIMx trigger DMA request enable */

/* SPI */
#define SPI_CR1_CPHA                    (uint32_t) (1 << 0)                                     /* SPI clock phase */
#define SPI_CR1_CPOL                    (uint32_t) (1 << 1)                                     /* SPI clock polarity */
#define SPI_CR1_MSTR                    (uint32_t) (1 << 2)                                     /* SPI master selection */
#define SPI_CR1_BR_2                    (uint32_t) (0 << 3)                                     /* SPI baud rate = fPCLK/2 */
#define SPI_CR1_BR_4                    (uint32_t) (1 << 3)                                     /* SPI baud rate = fPCLK/4 */
#define SPI_CR1_BR_8                    (uint32_t) (2 << 3)                                     /* SPI baud rate = fPCLK/8 */
#define SPI_CR1_BR_16                   (uint32_t) (3 << 3)                                     /* SPI baud rate = fPCLK/16 */
#define SPI_CR1_BR_32                   (uint32_t) (4 << 3)                                     /* SPI baud rate = fPCLK/32 */
#define SPI_CR1_BR_64                   (uint32_t) (5 << 3)                                     /* SPI baud rate = fPCLK/64 */
#define SPI_CR1_BR_128                  (uint32_t) (6 << 3)                                     /* SPI baud rate = fPCLK/128 */
#define SPI_CR1_BR_256                  (uint32_t) (7 << 3)                                     /* SPI baud rate = fPCLK/256 */
#define SPI_CR1_SPE                     (uint32_t) (1 << 6)                                     /* SPI enable */
#define SPI_CR1_LSBFIRST                (uint32_t) (1 << 7)                                     /* SPI LSB transmitted first */
#define SPI_CR1_SSI                     (uint32_t) (1 << 8)                                     /* SPI internal slave select */
#define SPI_CR1_SSM                     (uint32_t) (1 << 9)                                     /* SPI software slave management */
#define SPI_CR1_DFF                     (uint32_t) (1 << 11)                                    /* SPI data frame format (0 = 8bit, 1 = 16bit) */
#define SPI_SR_RXNE                     (uint32_t) (1 << 0)                                     /* SPI receive not empty */
#define SPI_SR_TXNE                     (uint32_t) (1 << 1)                                     /* SPI transmit not empty */
#define SPI_SR_CHSIDE                   (uint32_t) (1 << 2)                                     /* SPI channel side */
#define SPI_SR_UDR                      (uint32_t) (1 << 3)                                     /* SPI underrun flag */
#define SPI_SR_CRCERR                   (uint32_t) (1 << 4)                                     /* SPI CRC error flag */
#define SPI_SR_MODF                     (uint32_t) (1 << 5)                                     /* SPI mode fault */
#define SPI_SR_OVR                      (uint32_t) (1 << 6)                                     /* SPI overrun flag */
#define SPI_SR_BSY                      (uint32_t) (1 << 7)                                     /* SPI busy flag */
#define SPI_SR_TIRFE                    (uint32_t) (1 << 8)                                     /* SPI TI frame format error */

/* I2C */
#define I2C_CR1_PE                      (uint32_t) (1 << 0)                                     /* I2C peripheral enable */
#define I2C_CR1_SMBUS                   (uint32_t) (1 << 1)                                     /* I2C SMBus mode */
#define I2C_CR1_SMBTYPE                 (uint32_t) (1 << 3)                                     /* I2C SMBus type (0=Device, 1=Host) */
#define I2C_CR1_ENARP                   (uint32_t) (1 << 4)                                     /* I2C enable ARP */
#define I2C_CR1_ENPEC                   (uint32_t) (1 << 5)                                     /* I2C enable PEC */
#define I2C_CR1_ENGC                    (uint32_t) (1 << 6)                                     /* I2C enable general call */
#define I2C_CR1_NOSTRETCH               (uint32_t) (1 << 7)                                     /* I2C clock stretching disable */
#define I2C_CR1_START                   (uint32_t) (1 << 8)                                     /* I2C START generation */
#define I2C_CR1_STOP                    (uint32_t) (1 << 9)                                     /* I2C STOP generation */
#define I2C_CR1_ACK                     (uint32_t) (1 << 10)                                    /* I2C ACK enable */
#define I2C_CR1_POS                     (uint32_t) (1 << 11)                                    /* I2C ACK/PEC position */
#define I2C_CR1_PEC                     (uint32_t) (1 << 12)                                    /* I2C packet error checking */
#define I2C_CR1_ALERT                   (uint32_t) (1 << 13)                                    /* I2C SMBus alert */
#define I2C_CR1_SWRST                   (uint32_t) (1 << 15)                                    /* I2C software reset */

#define I2C_CR2_FREQ_ALL                (uint32_t) (0x003F)                                     /* FREQ[5:0] bits */
#define I2C_CR2_FREQ(n)                 (uint32_t) (n << 0)                                     /* I2C clock frequency */
#define I2C_CR2_ITERREN                 (uint32_t) (1 << 8)                                     /* I2C error interrupt enable */
#define I2C_CR2_ITEVTEN                 (uint32_t) (1 << 9)                                     /* I2C event interrupt enable */
#define I2C_CR2_ITBUFEN                 (uint32_t) (1 << 10)                                    /* I2C buffer interrupt enable */
#define I2C_CR2_DMAEN                   (uint32_t) (1 << 11)                                    /* I2C DMA requests enable */
#define I2C_CR2_LAST                    (uint32_t) (1 << 12)                                    /* I2C DMA last transfer */

#define I2C_OAR1_ADD(n)                 (uint16_t) (1 << n)                                     /* I2C interface address */
#define I2C_OAR1_ADD10(n)               (uint32_t) (n << 0)                                     /* I2C interface address (10-bit) */
#define I2C_OAR1_ADD7(n)                (uint32_t) (n << 1)                                     /* I2C interface address (7-bit) */
#define I2C_OAR1_ADDMODE                (uint32_t) (1 << 15)                                    /* I2C interface address mode (1=10-bit) */

#define I2C_OAR2_ENDUAL                 (uint32_t) (1 << 0)                                     /* I2C dual address mode enable */
#define I2C_OAR2_ADD2(n)                (uint32_t) (n << 1)                                     /* I2C interface address 2 (7-bit) */

#define I2C_SR1_SB                      (uint32_t) (1 << 0)                                     /* I2C start bit generated */
#define I2C_SR1_ADDR                    (uint32_t) (1 << 1)                                     /* I2C address sent/matched */
#define I2C_SR1_BTF                     (uint32_t) (1 << 2)                                     /* I2C byte transfer finished */
#define I2C_SR1_ADD10                   (uint32_t) (1 << 3)                                     /* I2C 10-bit header sent */
#define I2C_SR1_STOPF                   (uint32_t) (1 << 4)                                     /* I2C stop detection */
#define I2C_SR1_RXNE                    (uint32_t) (1 << 6)                                     /* I2C DR not empty */
#define I2C_SR1_TXE                     (uint32_t) (1 << 7)                                     /* I2C DR empty */
#define I2C_SR1_BERR                    (uint32_t) (1 << 8)                                     /* I2C bus error */
#define I2C_SR1_ARLO                    (uint32_t) (1 << 9)                                     /* I2C attribution lost */
#define I2C_SR1_AF                      (uint32_t) (1 << 10)                                    /* I2C acknowledge failure */
#define I2C_SR1_OVR                     (uint32_t) (1 << 11)                                    /* I2C overrun/underrun */
#define I2C_SR1_PECERR                  (uint32_t) (1 << 12)                                    /* I2C PEC error in reception */
#define I2C_SR1_TIMEOUT                 (uint32_t) (1 << 14)                                    /* I2C timeout or tlow error */
#define I2C_SR1_SMBALERT                (uint32_t) (1 << 15)                                    /* I2C SMBus alert */

#define I2C_SR2_MSL                     (uint32_t) (1 << 0)                                     /* I2C master/slave */
#define I2C_SR2_BUSY                    (uint32_t) (1 << 1)                                     /* I2C bus busy */
#define I2C_SR2_TRA                     (uint32_t) (1 << 2)                                     /* I2C transmitter/receiver */
#define I2C_SR2_GENCALL                 (uint32_t) (1 << 4)                                     /* I2C general call address */
#define I2C_SR2_SMBDEFAULT              (uint32_t) (1 << 5)                                     /* I2C SMBus device default address */
#define I2C_SR2_SMBHOST                 (uint32_t) (1 << 6)                                     /* I2C SMBus host header */
#define I2C_SR2_DUALF                   (uint32_t) (1 << 7)                                     /* I2C dual flag */
#define I2C_SR2_PEC(r)                  (uint32_t) (r >> 8)                                     /* I2C packet error checking register */

#define I2C_CCR_CCR(n)                  (uint32_t) (n & 0x0FFF)                                 /* I2C clock control register */
#define I2C_CCR_DUTY                    (uint32_t) (1 << 14)                                    /* I2C fast mode duty cycle */
#define I2C_CCR_FS                      (uint32_t) (1 << 15)                                    /* I2C master mode selection */

/* USART */
#define USART_SR_TC                     (uint32_t) (1 << 6)                                     /* USART Transmission Complete */
#define USART_SR_RXNE                   (uint32_t) (1 << 5)                                     /* USART Read data register not empty */

/* USART_CR1 */
#define USART_CR1_OVER8                 (uint32_t) (1 << 15)                                    /* Oversampling mode */
#define USART_CR1_UE                    (uint32_t) (1 << 13)                                    /* USART Enable */
#define USART_CR1_M9                    (uint32_t) (1 << 12)                                    /* World length 1: 9 bits, 0: 8bits */
#define USART_CR1_WAKE                  (uint32_t) (1 << 11)                                    /* Wakeup method */
#define USART_CR1_PCE                   (uint32_t) (1 << 10)                                    /* PCE Parity control enable */
#define USART_CR1_PS                    (uint32_t) (1 << 9)                                     /* Parity selection */
#define USART_CR1_PEIE                  (uint32_t) (1 << 8)                                     /* PE Interrupt enable */
#define USART_CR1_TXEIE                 (uint32_t) (1 << 7)                                     /* TXE interrupt enable */
#define USART_CR1_TCIE                  (uint32_t) (1 << 6)                                     /* Transmission complete interrupt enable */
#define USART_CR1_RXNEIE                (uint32_t) (1 << 5)                                     /* RXNE Interrupt Enable */
#define USART_CR1_IDLEIE                (uint32_t) (1 << 4)                                     /* IDEL INterrupt Enable */
#define USART_CR1_TE                    (uint32_t) (1 << 3)                                     /* USART Transmit Enable */
#define USART_CR1_RE                    (uint32_t) (1 << 2)                                     /* USART Receive Enable */
#define USART_CR1_RWU                   (uint32_t) (1 << 1)
#define USART_CR1_SBK                   (uint32_t) (1 << 0)                                     /* Send break characters */

#define USART_CR3_DMAR_EN               (uint32_t) (1 << 6)                                     /* USART DMA Receive Enable */
#define USART_CR3_DMAT_EN               (uint32_t) (1 << 7)                                     /* USART DMA Transmit Enable */

/* GPIO */
#define GPIOA                           (uint8_t)  (0)                                          /* GPIO Port A */
#define GPIOB                           (uint8_t)  (1)                                          /* GPIO Port B */
#define GPIOC                           (uint8_t)  (2)                                          /* GPIO Port C */
#define GPIOD                           (uint8_t)  (3)                                          /* GPIO Port D */
#define GPIOE                           (uint8_t)  (4)                                          /* GPIO Port E */
#define GPIOF                           (uint8_t)  (5)                                          /* GPIO Port F */
#define GPIOG                           (uint8_t)  (6)                                          /* GPIO Port G */
#define GPIOH                           (uint8_t)  (7)                                          /* GPIO Port H */
#define GPIOI                           (uint8_t)  (8)                                          /* GPIO Port I */

#define GPIO_MODER_PIN(n)               (uint32_t) (2*n)                                        /* Pin bitshift */
#define GPIO_MODER_M(n)                 (uint32_t) (0x3 << 2*n)                                 /* Pin mask */
#define GPIO_MODER_IN                   (uint32_t) (0x0)                                        /* Input mode */
#define GPIO_MODER_OUT                  (uint32_t) (0x1)                                        /* Output mode */
#define GPIO_MODER_ALT                  (uint32_t) (0x2)                                        /* Alternative function mode */
#define GPIO_MODER_ANA                  (uint32_t) (0x3)                                        /* Analog mode */

#define GPIO_OTYPER_PIN(n)              (uint32_t) (n)                                          /* Pin bitshift */
#define GPIO_OTYPER_M(n)                (uint32_t) (1 << n)                                     /* Pin mask */
#define GPIO_OTYPER_PP                  (uint32_t) (0x0)                                        /* Output push-pull */
#define GPIO_OTYPER_OD                  (uint32_t) (0x1)                                        /* Output open drain */

#define GPIO_OSPEEDR_PIN(n)             (uint32_t) (2*n)                                        /* Pin bitshift */
#define GPIO_OSPEEDR_M(n)               (uint32_t) (0x3 << (2*n))                               /* Pin mask */
#define GPIO_OSPEEDR_2M                 (uint32_t) (0x0)                                        /* Output speed 2MHz */
#define GPIO_OSPEEDR_25M                (uint32_t) (0x1)                                        /* Output speed 25MHz */
#define GPIO_OSPEEDR_50M                (uint32_t) (0x2)                                        /* Output speed 50MHz */
#define GPIO_OSPEEDR_100M               (uint32_t) (0x3)                                        /* Output speed 100MHz */

#define GPIO_PUPDR_PIN(n)               (uint32_t) (2*n)                                        /* Pin bitshift */
#define GPIO_PUPDR_M(n)                 (uint32_t) (0x3 << (2*n))                               /* Pin mask */
#define GPIO_PUPDR_NONE                 (uint32_t) (0x0)                                        /* Port no pull-up, pull-down */
#define GPIO_PUPDR_UP                   (uint32_t) (0x1)                                        /* Port pull-up */
#define GPIO_PUPDR_DOWN                 (uint32_t) (0x2)                                        /* Port pull-down */

#define GPIO_IDR_PIN(n)                 (uint32_t) (1 << n)                                     /* Input for pin n */

#define GPIO_ODR_PIN(n)                 (uint32_t) (1 << n)                                     /* Output for pin n */

#define GPIO_BSRR_BS(n)                 (uint32_t) (1 << n)                                     /* Set pin n */
#define GPIO_BSRR_BR(n)                 (uint32_t) (1 << (n+16))                                /* Reset pin n */

#define GPIO_AFRL_PIN(n)                (uint32_t) (4*n)                                        /* Pin bitshift */
#define GPIO_AFRL_M(n)                  (uint32_t) (0xF << (4*n))                               /* Pin mask */
#define GPIO_AFRH_PIN(n)                (uint32_t) (4*(n-8))                                    /* Pin bitshift */
#define GPIO_AFRH_M(n)                  (uint32_t) (0xF << (4*(n-8)))                           /* Pin mask */

/* DMA */
#define DMA_LISR_TCIF2                  (uint32_t) (1 << 21)                                    /* DMA stream 2 transfer complete flag */
#define DMA_HISR_TCIF7                  (uint32_t) (1 << 27)                                    /* DMA stream 7 transfer complete flag */
#define DMA_LIFCR_CTCIF2                (uint32_t) (1 << 21)                                    /* DMA clear stream 2 transfer complete flag */
#define DMA_HIFCR_CTCIF7                (uint32_t) (1 << 27)                                    /* DMA clear stream 7 transfer complete flag */

#define DMA_SxCR_EN                     (uint32_t) (1 << 0)                                     /* DMA stream enable */
#define DMA_SxCR_DMEIE                  (uint32_t) (1 << 1)                                     /* DMA stream direct mode error interrupt enable */
#define DMA_SxCR_TEIE                   (uint32_t) (1 << 2)                                     /* DMA stream transmit error interrupt enable */
#define DMA_SxCR_HTIE                   (uint32_t) (1 << 3)                                     /* DMA stream half transfer interrupt enable */
#define DMA_SxCR_TCIE                   (uint32_t) (1 << 4)                                     /* DMA stream transfer complete interrupt enable */
#define DMA_SxCR_PFCTRL                 (uint32_t) (1 << 5)                                     /* DMA stream peripheral flow control */
#define DMA_SxCR_DIR_PM                 (uint32_t) (0 << 6)                                     /* DMA stream peripheral-to-memory data transfer */
#define DMA_SxCR_DIR_MP                 (uint32_t) (1 << 6)                                     /* DMA stream memory-to-peripheral data transfer */
#define DMA_SxCR_DIR_MM                 (uint32_t) (2 << 6)                                     /* DMA stream memory-to-memory data transfer */
#define DMA_SxCR_CIRC                   (uint32_t) (1 << 8)                                     /* DMA stream circular mode enable */
#define DMA_SxCR_PINC                   (uint32_t) (1 << 9)                                     /* DMA stream peripheral increment mode enable */
#define DMA_SxCR_MINC                   (uint32_t) (1 << 10)                                    /* DMA stream memory increment mode enable */
#define DMA_SxCR_PSIZE_BYTE             (uint32_t) (0 << 11)                                    /* DMA stream peripheral data size - Byte */
#define DMA_SxCR_PSIZE_HW               (uint32_t) (1 << 11)                                    /* DMA stream peripheral data size - Half-word */
#define DMA_SxCR_PSIZE_WORD             (uint32_t) (2 << 11)                                    /* DMA stream peripheral data size - Word */
#define DMA_SxCR_MSIZE_BYTE             (uint32_t) (0 << 13)                                    /* DMA stream memory data size - Byte */
#define DMA_SxCR_MSIZE_HW               (uint32_t) (1 << 13)                                    /* DMA stream memory data size - Half-word */
#define DMA_SxCR_MSIZE_WORD             (uint32_t) (2 << 13)                                    /* DMA stream memory data size - Word */
#define DMA_SxCR_PINCOS_LINKED          (uint32_t) (0 << 15)                                    /* DMA stream peripheral increment offset size linked to PSIZE */
#define DMA_SxCR_PINCOS_FIXED           (uint32_t) (1 << 15)                                    /* DMA stream peripheral increment offset size fixed */
#define DMA_SxCR_PL_LOW                 (uint32_t) (0 << 16)                                    /* DMA stream priority level low */
#define DMA_SxCR_PL_MED                 (uint32_t) (1 << 16)                                    /* DMA stream priority level medium */
#define DMA_SxCR_PL_HIGH                (uint32_t) (2 << 16)                                    /* DMA stream priority level high */
#define DMA_SxCR_PL_VHIGH               (uint32_t) (3 << 16)                                    /* DMA stream priority level very high */
#define DMA_SxCR_DBM                    (uint32_t) (1 << 18)                                    /* DMA stream double buffer mode */
#define DMA_SxCR_CT                     (uint32_t) (1 << 19)                                    /* DMA stream current target (M0 or M1) */
#define DMA_SxCR_PBURST_NO              (uint32_t) (0 << 21)                                    /* DMA stream peripheral burst disable */
#define DMA_SxCR_PBURST_4               (uint32_t) (1 << 21)                                    /* DMA stream peripheral burst of 4 beats */
#define DMA_SxCR_PBURST_8               (uint32_t) (2 << 21)                                    /* DMA stream peripheral burst of 8 beats */
#define DMA_SxCR_PBURST_16              (uint32_t) (3 << 21)                                    /* DMA stream peripheral burst of 16 beats */
#define DMA_SxCR_MBURST_NO              (uint32_t) (0 << 23)                                    /* DMA stream memory burst disable */
#define DMA_SxCR_MBURST_4               (uint32_t) (1 << 23)                                    /* DMA stream memory burst of 4 beats */
#define DMA_SxCR_MBURST_8               (uint32_t) (2 << 23)                                    /* DMA stream memory burst of 8 beats */
#define DMA_SxCR_MBURST_16              (uint32_t) (3 << 23)                                    /* DMA stream memory burst of 16 beats */
#define DMA_SxCR_CHSEL(x)               (uint32_t) (x << 25)                                    /* DMA stream channel select */

#define DMA_SxFCR_FTH_1                 (uint32_t) (0 << 0)                                     /* DMA stream FIFO threshold 1/4 */
#define DMA_SxFCR_FTH_2                 (uint32_t) (1 << 0)                                     /* DMA stream FIFO threshold 1/2 */
#define DMA_SxFCR_FTH_3                 (uint32_t) (2 << 0)                                     /* DMA stream FIFO threshold 3/4 */
#define DMA_SxFCR_FTH_4                 (uint32_t) (3 << 0)                                     /* DMA stream FIFO threshold full*/
#define DMA_SxFCR_DMDIS                 (uint32_t) (1 << 2)                                     /* DMA stream direct mode disable */
#define DMA_SxFCR_FS                    (uint32_t) (7 << 3)                                     /* DMA stream FIFO status */
#define DMA_SxFCR_FEIE                  (uint32_t) (1 << 7)                                     /* DMA stream FIFO error interrupt enable */

/* USB FS */
/* Global Registers */
#define USB_FS_GOTGCTL_SRQSCS           (uint32_t) (1 << 0)                                     /* USB session request success */
#define USB_FS_GOTGCTL_SRQ              (uint32_t) (1 << 1)                                     /* USB session request */
#define USB_FS_GOTGCTL_HNGSCS           (uint32_t) (1 << 8)                                     /* USB host negotiation success */
#define USB_FS_GOTGCTL_HNP              (uint32_t) (1 << 9)                                     /* USB HNP request */
#define USB_FS_GOTGCTL_HSHNPEN          (uint32_t) (1 << 10)                                    /* USB host set HPN enable */
#define USB_FS_GOTGCTL_DHNPEN           (uint32_t) (1 << 11)                                    /* USB device HPN enabled */
#define USB_FS_GOTGCTL_CIDSTS           (uint32_t) (1 << 16)                                    /* USB connector ID status */
#define USB_FS_GOTGCTL_DBCT             (uint32_t) (1 << 17)                                    /* USB debounce time */
#define USB_FS_GOTGCTL_ASVLD            (uint32_t) (1 << 18)                                    /* USB A session valid */
#define USB_FS_GOTGCTL_BSVLD            (uint32_t) (1 << 19)                                    /* USB B session valid */

#define USB_FS_GOTGINT_SEDET            (uint32_t) (1 << 2)                                     /* USB session end detected */
#define USB_FS_GOTGINT_SRSSCHG          (uint32_t) (1 << 8)                                     /* USB session request success status change */
#define USB_FS_GOTGINT_HNSSCHG          (uint32_t) (1 << 9)                                     /* USB host negotiation success status change */
#define USB_FS_GOTGINT_HNGDET           (uint32_t) (1 << 17)                                    /* USB host negotiation detected */
#define USB_FS_GOTGINT_ADTOCHG          (uint32_t) (1 << 18)                                    /* USB A-device timeout change */
#define USB_FS_GOTGINT_DBCDNE           (uint32_t) (1 << 19)                                    /* USB debounce done */

#define USB_FS_GAHBCFG_GINTMSK          (uint32_t) (1 << 0)                                     /* USB global interrupt mask */
#define USB_FS_GAHBCFG_TXFELVL          (uint32_t) (1 << 7)                                     /* USB TX FIFO empty level */
#define USB_FS_GAHBCFG_PTXFELVL         (uint32_t) (1 << 8)                                     /* USB Periodic TX FIFO empty level */

#define USB_FS_GUSBCFG_TOCAL(n)         (uint32_t) (n << 0)                                     /* USB FS timeout calibration */
#define USB_FS_GUSBCFG_PHYSEL           (uint32_t) (1 << 7)                                     /* USB FS serial transceiver select */
#define USB_FS_GUSBCFG_SRPCAP           (uint32_t) (1 << 8)                                     /* USB SRP capable */
#define USB_FS_GUSBCFG_HNPCAP           (uint32_t) (1 << 9)                                     /* USB HNP capable */
#define USB_FS_GUSBCFG_TRDT(n)          (uint32_t) (n << 10)                                    /* USB turnaround time (4 bits) */
#define USB_FS_GUSBCFG_FHMOD            (uint32_t) (1 << 29)                                    /* USB force host mode */
#define USB_FS_GUSBCFG_FDMOD            (uint32_t) (1 << 30)                                    /* USB force device mode */
#define USB_FS_GUSBCFG_CTXPKT           (uint32_t) (1 << 31)                                    /* USB corrupt packet */

#define USB_FS_GRSTCTL_CSRST            (uint32_t) (1 << 0)                                     /* USB core soft reset */
#define USB_FS_GRSTCTL_HSRST            (uint32_t) (1 << 1)                                     /* USB HCLK soft reset */
#define USB_FS_GRSTCTL_FCRST            (uint32_t) (1 << 2)                                     /* USB host frame counter reset */
#define USB_FS_GRSTCTL_RXFFLSH          (uint32_t) (1 << 4)                                     /* USB RX FIFO flush */
#define USB_FS_GRSTCTL_TXFFLSH          (uint32_t) (1 << 5)                                     /* USB TX FIFO flush */
#define USB_FS_GRSTCTL_TXFNUM(n)        (uint32_t) (n << 6)                                     /* USB TX FIFO number */
#define USB_FS_GRSTCTL_AHBIDL           (uint32_t) (1 << 31)                                    /* USB AHB master idle */

#define USB_FS_GINTSTS_CMOD             (uint32_t) (1 << 0)                                     /* USB current mode of operation */
#define USB_FS_GINTSTS_MMIS             (uint32_t) (1 << 1)                                     /* USB mode mismatch interrupt */
#define USB_FS_GINTSTS_OTGINT           (uint32_t) (1 << 2)                                     /* USB OTG interrupt */
#define USB_FS_GINTSTS_SOF              (uint32_t) (1 << 3)                                     /* USB start of frame */
#define USB_FS_GINTSTS_RXFLVL           (uint32_t) (1 << 4)                                     /* USB RX FIFO non-empty */
#define USB_FS_GINTSTS_NPTXFE           (uint32_t) (1 << 5)                                     /* USB non-periodic TX FIFO empty */
#define USB_FS_GINTSTS_GINAKEFF         (uint32_t) (1 << 6)                                     /* USB global IN non-periodic NAK effective */
#define USB_FS_GINTSTS_GONAKEFF         (uint32_t) (1 << 7)                                     /* USB global OUT NAK effective */
#define USB_FS_GINTSTS_ESUSP            (uint32_t) (1 << 10)                                    /* USB early suspend */
#define USB_FS_GINTSTS_USBSUSP          (uint32_t) (1 << 11)                                    /* USB suspend */
#define USB_FS_GINTSTS_USBRST           (uint32_t) (1 << 12)                                    /* USB reset */
#define USB_FS_GINTSTS_ENUMDNE          (uint32_t) (1 << 13)                                    /* USB enumeration done */
#define USB_FS_GINTSTS_ISOODRP          (uint32_t) (1 << 14)                                    /* USB isochronous OUT packet dropped interrupt */
#define USB_FS_GINTSTS_EOPF             (uint32_t) (1 << 15)                                    /* USB end of packet frame interrupt */
#define USB_FS_GINTSTS_IEPINT           (uint32_t) (1 << 18)                                    /* USB IN endpoint interrupt */
#define USB_FS_GINTSTS_OEPINT           (uint32_t) (1 << 19)                                    /* USB OUT endpoint interrupt */
#define USB_FS_GINTSTS_IISOIXFR         (uint32_t) (1 << 20)                                    /* USB incomplete isochronous IN transfer */
#define USB_FS_GINTSTS_IISOOXFR         (uint32_t) (1 << 21)                                    /* USB incomplete isochronous OUT transfer */
#define USB_FS_GINTSTS_IPXFR            (uint32_t) (1 << 21)                                    /* USB incomplete periodic transfer */
#define USB_FS_GINTSTS_HPRTINT          (uint32_t) (1 << 24)                                    /* USB host port interrupt */
#define USB_FS_GINTSTS_HCINT            (uint32_t) (1 << 25)                                    /* USB host channels interrupt */
#define USB_FS_GINTSTS_PTXFE            (uint32_t) (1 << 26)                                    /* USB periodic TX FIFO empty */
#define USB_FS_GINTSTS_CIDSCHG          (uint32_t) (1 << 28)                                    /* USB connector ID status change */
#define USB_FS_GINTSTS_DISCINT          (uint32_t) (1 << 29)                                    /* USB disconnect detected interrupt */
#define USB_FS_GINTSTS_SRQINT           (uint32_t) (1 << 30)                                    /* USB session request/new session detected interrupt */
#define USB_FS_GINTSTS_WKUPINT          (uint32_t) (1 << 31)                                    /* USB resume/remote wakeup detected interrupt */

#define USB_FS_GINTMSK_MMISM            (uint32_t) (1 << 1)                                     /* USB mode mismatch interrupt mask */
#define USB_FS_GINTMSK_OTGINT           (uint32_t) (1 << 2)                                     /* USB OTG interrupt mask */
#define USB_FS_GINTMSK_SOFM             (uint32_t) (1 << 3)                                     /* USB start of frame mask */
#define USB_FS_GINTMSK_RXFLVLM          (uint32_t) (1 << 4)                                     /* USB RX FIFO non-empty mask */
#define USB_FS_GINTMSK_NPTXFEM          (uint32_t) (1 << 5)                                     /* USB non-periodic TX FIFO empty mask */
#define USB_FS_GINTMSK_GINAKEFFM        (uint32_t) (1 << 6)                                     /* USB global IN non-periodic NAK effective mask */
#define USB_FS_GINTMSK_GONAKEFFM        (uint32_t) (1 << 7)                                     /* USB global OUT NAK effective mask */
#define USB_FS_GINTMSK_ESUSPM           (uint32_t) (1 << 10)                                    /* USB early suspend mask */
#define USB_FS_GINTMSK_USBSUSPM         (uint32_t) (1 << 11)                                    /* USB suspend mask */
#define USB_FS_GINTMSK_USBRSTM          (uint32_t) (1 << 12)                                    /* USB reset mask */
#define USB_FS_GINTMSK_ENUMDNEM         (uint32_t) (1 << 13)                                    /* USB enumeration done mask */
#define USB_FS_GINTMSK_ISOODRPM         (uint32_t) (1 << 14)                                    /* USB isochronous OUT packet dropped interrupt mask */
#define USB_FS_GINTMSK_EOPFM            (uint32_t) (1 << 15)                                    /* USB end of packet frame interrupt mask */
#define USB_FS_GINTMSK_EPMISM           (uint32_t) (1 << 17)                                    /* USB endpoint mismatch interrupt mask */
#define USB_FS_GINTMSK_IEPINT           (uint32_t) (1 << 18)                                    /* USB IN endpoint interrupt mask */
#define USB_FS_GINTMSK_OEPINT           (uint32_t) (1 << 19)                                    /* USB OUT endpoint interrupt mask */
#define USB_FS_GINTMSK_IISOIXFRM        (uint32_t) (1 << 20)                                    /* USB incomplete isochronous IN transfer mask */
#define USB_FS_GINTMSK_IISOOXFRM        (uint32_t) (1 << 21)                                    /* USB incomplete isochronous OUT transfer mask */
#define USB_FS_GINTMSK_IPXFRM           (uint32_t) (1 << 21)                                    /* USB incomplete periodic transfer mask */
#define USB_FS_GINTMSK_HPRTINT          (uint32_t) (1 << 24)                                    /* USB host port interrupt mask */
#define USB_FS_GINTMSK_HCINT            (uint32_t) (1 << 25)                                    /* USB host channels interrupt mask */
#define USB_FS_GINTMSK_PTXFEM           (uint32_t) (1 << 26)                                    /* USB periodic TX FIFO empty mask */
#define USB_FS_GINTMSK_CIDSCHGM         (uint32_t) (1 << 28)                                    /* USB connector ID status change mask */
#define USB_FS_GINTMSK_DISCINT          (uint32_t) (1 << 29)                                    /* USB disconnect detected interrupt mask */
#define USB_FS_GINTMSK_SRQINT           (uint32_t) (1 << 30)                                    /* USB session request/new session detected interrupt mask */
#define USB_FS_GINTMSK_WKUPINT          (uint32_t) (1 << 31)                                    /* USB resume/remote wakeup detected interrupt mask */

#define USB_FS_GRXSTS_EPNUM(r)          (uint32_t) (r & 0xF)                                    /* USB RX FIFO endpoint number */
#define USB_FS_GRXSTS_BCNT(r)           (uint32_t) ((r & 0x7FF0) >> 4)                          /* USB RX FIFO byte count */
#define USB_FS_GRXSTS_DPID(r)           (uint32_t) ((r & 0x18000) >> 15)                        /* USB RX FIFO data PID */
#define USB_FS_GRXSTS_PKTSTS(r)         (uint32_t) ((r & 0x1E0000) >> 17)                       /* USB RX FIFO packet status */
#define USB_FS_GRXSTS_FRMNUM(r)         (uint32_t) ((r & 0x1E00000) >> 21)                      /* USB RX FIFO frame number */
#define USB_FS_GRXSTS_PKTSTS_NAK        (uint8_t)  (1)                                          /* USB RX FIFO packet status = global OUT NAK */
#define USB_FS_GRXSTS_PKTSTS_ORX        (uint8_t)  (2)                                          /* USB RX FIFO packet status = OUT data packet received */
#define USB_FS_GRXSTS_PKTSTS_OCP        (uint8_t)  (3)                                          /* USB RX FIFO packet status = OUT transfer completed */
#define USB_FS_GRXSTS_PKTSTS_STUPCP     (uint8_t)  (4)                                          /* USB RX FIFO packet status = SETUP transaction completed */
#define USB_FS_GRXSTS_PKTSTS_STUPRX     (uint8_t)  (6)                                          /* USB RX FIFO packet status = SETUP data packet received */

#define USB_FS_DIEPTXF0_TX0FSA(n)       (uint32_t) (n << 0)                                     /* USB endpoint 0 transmit RAM start address */
#define USB_FS_DIEPTXF0_TX0FD(n)        (uint32_t) (n << 16)                                    /* USB endpoint 0 TX FIFO depth */

#define USB_FS_GCCFG_PWRDWN             (uint32_t) (1 << 16)                                    /* USB power down */
#define USB_FS_GCCFG_VBUSASEN           (uint32_t) (1 << 18)                                    /* USB VBUS "A" sensing enable */
#define USB_FS_GCCFG_VBUSBSEN           (uint32_t) (1 << 19)                                    /* USB VBUS "B" sensing enable */
#define USB_FS_GCCFG_SOFOUTEN           (uint32_t) (1 << 20)                                    /* USB SOF output enable */
#define USB_FS_GCCFG_NOVBUSSSENS        (uint32_t) (1 << 21)                                    /* USB VBUS sensing disable */

#define USB_FS_DIEPTXF_INEPTXSA(n)      (uint32_t) (n << 0)                                     /* USB IN endpoint FIFOx transmit RAM start address */
#define USB_FS_DIEPTXF_INEPTXFD(n)      (uint32_t) (n << 16)                                    /* USB IN endpoint TX FIFOx depth */

/* Device-mode Registers */
#define USB_FS_DCFG_DSPD_FS             (uint32_t) (1 << 1 | 1 << 0)                            /* USB device speed: full speed (USB 1.1) */
#define USB_FS_DCFG_NZLSOHSK            (uint32_t) (1 << 2)                                     /* USB device non-zero-length status OUT handshake */
#define USB_FS_DCFG_DAD(x)              (uint32_t) (x << 4)                                     /* USB device address */
#define USB_FS_DCFG_PFIVL_80            (uint32_t) (0 << 11)                                    /* USB device periodic frame interval: 80% */
#define USB_FS_DCFG_PFIVL_85            (uint32_t) (1 << 11)                                    /* USB device periodic frame interval: 85% */
#define USB_FS_DCFG_PFIVL_90            (uint32_t) (2 << 11)                                    /* USB device periodic frame interval: 90% */
#define USB_FS_DCFG_PFIVL_95            (uint32_t) (3 << 11)                                    /* USB device periodic frame interval: 95% */

#define USB_FS_DCTL_RWUSIG              (uint32_t) (1 << 0)                                     /* USB device remote wakeup signaling */
#define USB_FS_DCTL_SDIS                (uint32_t) (1 << 1)                                     /* USB device soft disconnect */
#define USB_FS_DCTL_GINSTS              (uint32_t) (1 << 2)                                     /* USB device global IN NAK status */
#define USB_FS_DCTL_GONSTS              (uint32_t) (1 << 3)                                     /* USB device global OUT NAK status */
#define USB_FS_DCTL_TCTL_J              (uint32_t) (1 << 4)                                     /* USB device Test_J mode */
#define USB_FS_DCTL_TCTL_K              (uint32_t) (2 << 4)                                     /* USB device Test_K mode */
#define USB_FS_DCTL_TCTL_SE0_NAK        (uint32_t) (3 << 4)                                     /* USB device Test_SE0_NAK mode */
#define USB_FS_DCTL_TCTL_PKT            (uint32_t) (4 << 4)                                     /* USB device Test_Packet mode */
#define USB_FS_DCTL_TCTL_FEN            (uint32_t) (5 << 4)                                     /* USB device Test_Force_Enable mode */
#define USB_FS_DCTL_SGINAK              (uint32_t) (1 << 7)                                     /* USB device set global IN NAK */
#define USB_FS_DCTL_CGINAK              (uint32_t) (1 << 8)                                     /* USB device clear global IN NAK */
#define USB_FS_DCTL_SGONAK              (uint32_t) (1 << 9)                                     /* USB device set global OUT NAK */
#define USB_FS_DCTL_CGONAK              (uint32_t) (1 << 10)                                    /* USB device clear global OUT NAK */
#define USB_FS_DCTL_POPRGDNE            (uint32_t) (1 << 11)                                    /* USB device power-no programming done */

#define USB_FS_DSTS_SUSPSTS             (uint32_t) (1 << 0)                                     /* USB device suspend status */
#define USB_FS_DSTS_ENUMSPD             (uint32_t) (3 << 1)                                     /* USB device enumerated speed (Must == 3) */
#define USB_FS_DSTS_ENUMSPD_FS          (uint32_t) (3 << 1)                                          /* USB device enumerated speed (Must == 3) */
#define USB_FS_DSTS_EERR                (uint32_t) (1 << 3)                                     /* USB device erratic error */
#define USB_FS_DSTS_FNSOF               (uint32_t) (0x3FFF00)                                   /* USB device frame number of received SOF (bits 21:8) */

#define USB_FS_DIEPMSK_XFRCM            (uint32_t) (1 << 0)                                     /* USB device transfer completed interrupt mask */
#define USB_FS_DIEPMSK_EPDM             (uint32_t) (1 << 1)                                     /* USB device endpoint disabled interrupt mask */
#define USB_FS_DIEPMSK_TOM              (uint32_t) (1 << 3)                                     /* USB device timout condition mask */
#define USB_FS_DIEPMSK_ITTXFEMSK        (uint32_t) (1 << 4)                                     /* USB device IN token received when TX FIFO empty mask */
#define USB_FS_DIEPMSK_INEPNMM          (uint32_t) (1 << 5)                                     /* USB device IN token received with EP mismatch mask */
#define USB_FS_DIEPMSK_INEPNEM          (uint32_t) (1 << 6)                                     /* USB device IN endpoint NAK effective mask */

#define USB_FS_DOEPMSK_XFRCM            (uint32_t) (1 << 0)                                     /* USB device transfer completed interrupt mask */
#define USB_FS_DOEPMSK_EPDM             (uint32_t) (1 << 1)                                     /* USB device endpoint disabled interrupt mask */
#define USB_FS_DOEPMSK_STUPM            (uint32_t) (1 << 3)                                     /* USB device SETUP phase done mask */
#define USB_FS_DOEPMSK_OTEPDM           (uint32_t) (1 << 4)                                     /* USB device OUT token received when endpoint disabled mask */

#define USB_FS_DAINT_IEPINT(n)          (uint32_t) (1 << n)                                     /* USB device IN endpoint interrupt bits */
#define USB_FS_DAINT_OEPINT(n)          (uint32_t) (1 << (n+16))                                /* USB device OUT endpoint interrupt bits */

#define USB_FS_DAINT_IEPM(n)            (uint32_t) (1 << n)                                     /* USB device IN endpoint interrupt mask bits */
#define USB_FS_DAINT_OEPM(n)            (uint32_t) (1 << (n+16))                                /* USB device OUT endpoint interrupt mask bits */

#define USB_FS_DIEPEMPMSK_INEPTXFEM(n)  (uint32_t) (1 << n)                                     /* USB device IN EP TX FIFO empty interrupt mask bits */

#define USB_FS_DIEPCTL0_MPSIZE          (uint32_t) (3 << 0)                                     /* USB device endpoint 0 IN maximum packet size */
#define USB_FS_DIEPCTL0_MPSIZE_64       (uint32_t) (0 << 0)                                     /* USB device endpoint 0 IN maximum packet size 64 bytes */
#define USB_FS_DIEPCTL0_MPSIZE_32       (uint32_t) (1 << 0)                                     /* USB device endpoint 0 IN maximum packet size 32 bytes */
#define USB_FS_DIEPCTL0_MPSIZE_16       (uint32_t) (2 << 0)                                     /* USB device endpoint 0 IN maximum packet size 16 bytes */
#define USB_FS_DIEPCTL0_MPSIZE_8        (uint32_t) (3 << 0)                                     /* USB device endpoint 0 IN maximum packet size 8 bytes */
#define USB_FS_DIEPCTL0_USBAEP          (uint32_t) (1 << 15)                                    /* USB device endpoint 0 IN USB active endpoint (always 1) */
#define USB_FS_DIEPCTL0_NAKSTS          (uint32_t) (1 << 17)                                    /* USB device endpoint 0 IN NAK status */
#define USB_FS_DIEPCTL0_STALL           (uint32_t) (1 << 21)                                    /* USB device endpoint 0 IN STALL handshake */
#define USB_FS_DIEPCTL0_TXFNUM(n)       (uint32_t) (n << 22)                                    /* USB device endpoint 0 IN TX FIFO number */
#define USB_FS_DIEPCTL0_CNAK            (uint32_t) (1 << 26)                                    /* USB device endpoint 0 IN clear NAK */
#define USB_FS_DIEPCTL0_SNAK            (uint32_t) (1 << 27)                                    /* USB device endpoint 0 IN set NAK */
#define USB_FS_DIEPCTL0_EPDIS           (uint32_t) (1 << 30)                                    /* USB device endpoint 0 IN endpoint disable */
#define USB_FS_DIEPCTL0_EPENA           (uint32_t) (1 << 31)                                    /* USB device endpoint 0 IN endpoint enable */

#define USB_FS_DIEPCTLx_MPSIZE(n)       (uint32_t) (n << 0)                                     /* USB device endpoint x IN maximum packet size in bytes (11 bits) */
#define USB_FS_DIEPCTLx_USBAEP          (uint32_t) (1 << 15)                                    /* USB device endpoint x IN USB active endpoint */
#define USB_FS_DIEPCTLx_EONUM_EVEN      (uint32_t) (0 << 16)                                    /* USB device endpoint x IN Odd frame */
#define USB_FS_DIEPCTLx_EONUM_ODD       (uint32_t) (1 << 16)                                    /* USB device endpoint x IN Odd frame */
#define USB_FS_DIEPCTLx_DPID_0          (uint32_t) (0 << 16)                                    /* USB device endpoint x IN data PID 0 */
#define USB_FS_DIEPCTLx_DPID_1          (uint32_t) (1 << 16)                                    /* USB device endpoint x IN data PID 1 */
#define USB_FS_DIEPCTLx_NAKSTS          (uint32_t) (1 << 17)                                    /* USB device endpoint x IN NAK status */
#define USB_FS_DIEPCTLx_EPTYP_CTL       (uint32_t) (0 << 18)                                    /* USB device endpoint x IN type: control */
#define USB_FS_DIEPCTLx_EPTYP_ISO       (uint32_t) (1 << 18)                                    /* USB device endpoint x IN type: isochronous */
#define USB_FS_DIEPCTLx_EPTYP_BLK       (uint32_t) (2 << 18)                                    /* USB device endpoint x IN type: bulk */
#define USB_FS_DIEPCTLx_EPTYP_INT       (uint32_t) (3 << 18)                                    /* USB device endpoint x IN type: interrupt */
#define USB_FS_DIEPCTLx_STALL           (uint32_t) (1 << 21)                                    /* USB device endpoint x IN STALL handshake */
#define USB_FS_DIEPCTLx_TXFNUM(n)       (uint32_t) (n << 22)                                    /* USB device endpoint x IN TX FIFO number (4 bits) */
#define USB_FS_DIEPCTLx_CNAK            (uint32_t) (1 << 26)                                    /* USB device endpoint x IN clear NAK */
#define USB_FS_DIEPCTLx_SNAK            (uint32_t) (1 << 27)                                    /* USB device endpoint x IN set NAK */
#define USB_FS_DIEPCTLx_SD0PID          (uint32_t) (1 << 28)                                    /* USB device endpoint x IN set DATA0 PID */
#define USB_FS_DIEPCTLx_SEVNFRM         (uint32_t) (1 << 28)                                    /* USB device endpoint x IN set even frame */
#define USB_FS_DIEPCTLx_SODDFRM         (uint32_t) (1 << 29)                                    /* USB device endpoint x IN set odd frame */
#define USB_FS_DIEPCTLx_EPDIS           (uint32_t) (1 << 30)                                    /* USB device endpoint x IN endpoint disable */
#define USB_FS_DIEPCTLx_EPENA           (uint32_t) (1 << 31)                                    /* USB device endpoint x IN endpoint enable */

#define USB_FS_DOEPCTL0_MPSIZE          (uint32_t) (3 << 0)                                     /* USB device endpoint 0 OUT maximum packet size */
#define USB_FS_DOEPCTL0_MPSIZE_64       (uint32_t) (0 << 0)                                     /* USB device endpoint 0 OUT maximum packet size 64 bytes */
#define USB_FS_DOEPCTL0_MPSIZE_32       (uint32_t) (1 << 0)                                     /* USB device endpoint 0 OUT maximum packet size 32 bytes */
#define USB_FS_DOEPCTL0_MPSIZE_16       (uint32_t) (2 << 0)                                     /* USB device endpoint 0 OUT maximum packet size 16 bytes */
#define USB_FS_DOEPCTL0_MPSIZE_8        (uint32_t) (3 << 0)                                     /* USB device endpoint 0 OUT maximum packet size 8 bytes */
#define USB_FS_DOEPCTL0_USBAEP          (uint32_t) (1 << 15)                                    /* USB device endpoint 0 OUT USB active endpoint (always 1) */
#define USB_FS_DOEPCTL0_NAKSTS          (uint32_t) (1 << 17)                                    /* USB device endpoint 0 OUT NAK status */
#define USB_FS_DOEPCTL0_SNPM            (uint32_t) (1 << 20)                                    /* USB device endpoint 0 OUT snoop mode */
#define USB_FS_DOEPCTL0_STALL           (uint32_t) (1 << 21)                                    /* USB device endpoint 0 OUT STALL handshake */
#define USB_FS_DOEPCTL0_CNAK            (uint32_t) (1 << 26)                                    /* USB device endpoint 0 OUT clear NAK */
#define USB_FS_DOEPCTL0_SNAK            (uint32_t) (1 << 27)                                    /* USB device endpoint 0 OUT set NAK */
#define USB_FS_DOEPCTL0_EPDIS           (uint32_t) (1 << 30)                                    /* USB device endpoint 0 OUT endpoint disable */
#define USB_FS_DOEPCTL0_EPENA           (uint32_t) (1 << 31)                                    /* USB device endpoint 0 OUT endpoint enable */

#define USB_FS_DOEPCTLx_MPSIZE(n)       (uint32_t) (n << 0)                                     /* USB device endpoint x OUT maximum packet size in bytes (11 bits) */
#define USB_FS_DOEPCTLx_USBAEP          (uint32_t) (1 << 15)                                    /* USB device endpoint x OUT USB active endpoint */
#define USB_FS_DOEPCTLx_EONUM_EVEN      (uint32_t) (0 << 16)                                    /* USB device endpoint x OUT Odd frame */
#define USB_FS_DOEPCTLx_EONUM_ODD       (uint32_t) (1 << 16)                                    /* USB device endpoint x OUT Odd frame */
#define USB_FS_DOEPCTLx_DPID_0          (uint32_t) (0 << 16)                                    /* USB device endpoint x OUT data PID 0 */
#define USB_FS_DOEPCTLx_DPID_1          (uint32_t) (1 << 16)                                    /* USB device endpoint x OUT data PID 1 */
#define USB_FS_DOEPCTLx_NAKSTS          (uint32_t) (1 << 17)                                    /* USB device endpoint x OUT NAK status */
#define USB_FS_DOEPCTLx_EPTYP_CTL       (uint32_t) (0 << 18)                                    /* USB device endpoint x OUT type: control */
#define USB_FS_DOEPCTLx_EPTYP_ISO       (uint32_t) (1 << 18)                                    /* USB device endpoint x OUT type: isochronous */
#define USB_FS_DOEPCTLx_EPTYP_BLK       (uint32_t) (2 << 18)                                    /* USB device endpoint x OUT type: bulk */
#define USB_FS_DOEPCTLx_EPTYP_INT       (uint32_t) (3 << 18)                                    /* USB device endpoint x OUT type: interrupt */
#define USB_FS_DOEPCTLx_SNPMN           (uint32_t) (1 << 20)                                    /* USB device endpoint x OUT snoop mode */
#define USB_FS_DOEPCTLx_STALL           (uint32_t) (1 << 21)                                    /* USB device endpoint x OUT STALL handshake */
#define USB_FS_DOEPCTLx_CNAK            (uint32_t) (1 << 26)                                    /* USB device endpoint x OUT clear NAK */
#define USB_FS_DOEPCTLx_SNAK            (uint32_t) (1 << 27)                                    /* USB device endpoint x OUT set NAK */
#define USB_FS_DOEPCTLx_SD0PID          (uint32_t) (1 << 28)                                    /* USB device endpoint x OUT set DATA0 PID */
#define USB_FS_DOEPCTLx_SEVNFRM         (uint32_t) (1 << 28)                                    /* USB device endpoint x OUT set even frame */
#define USB_FS_DOEPCTLx_SODDFRM         (uint32_t) (1 << 29)                                    /* USB device endpoint x OUT set odd frame */
#define USB_FS_DOEPCTLx_EPDIS           (uint32_t) (1 << 30)                                    /* USB device endpoint x OUT endpoint disable */
#define USB_FS_DOEPCTLx_EPENA           (uint32_t) (1 << 31)                                    /* USB device endpoint x OUT endpoint enable */

#define USB_FS_DIEPINTx_XFRC            (uint32_t) (1 << 0)                                     /* USB device endpoint x IN transfer complete interrupt */
#define USB_FS_DIEPINTx_EPDISD          (uint32_t) (1 << 1)                                     /* USB device endpoint x IN endpoint disabled interrupt */
#define USB_FS_DIEPINTx_TOC             (uint32_t) (1 << 3)                                     /* USB device endpoint x IN timeout condition */
#define USB_FS_DIEPINTx_ITTXFE          (uint32_t) (1 << 4)                                     /* USB device endpoint x IN token received when TX FIFO is empty */
#define USB_FS_DIEPINTx_INEPNE          (uint32_t) (1 << 6)                                     /* USB device endpoint x IN endpoint NAK effective */
#define USB_FS_DIEPINTx_TXFE            (uint32_t) (1 << 7)                                     /* USB device endpoint x IN TX FIFO empty */

#define USB_FS_DOEPINTx_XFRC            (uint32_t) (1 << 0)                                     /* USB device endpoint x OUT transfer complete interrupt */
#define USB_FS_DOEPINTx_EPDISD          (uint32_t) (1 << 1)                                     /* USB device endpoint x OUT endpoint disabled interrupt */
#define USB_FS_DOEPINTx_STUP            (uint32_t) (1 << 3)                                     /* USB device endpoint x OUT SETUP phase complete */
#define USB_FS_DOEPINTx_OTEPDIS         (uint32_t) (1 << 4)                                     /* USB device endpoint x OUT token receieved when endpoint disabled */
#define USB_FS_DOEPINTx_B2BSTUP         (uint32_t) (1 << 6)                                     /* USB device endpoint x OUT back-to-back SETUP packets received */

#define USB_FS_DIEPTSIZ0_XFRSIZ(n)      (uint32_t) (n << 0)                                     /* USB device endpoint 0 IN transfer size (7 bits) */
#define USB_FS_DIEPTSIZ0_PKTCNT(n)      (uint32_t) (n << 19)                                    /* USB device endpoint 0 IN packet count */

#define USB_FS_DIEPTSIZx_XFRSIZ(n)      (uint32_t) (n << 0)                                     /* USB device endpoint x IN transfer size (19 bits) */
#define USB_FS_DIEPTSIZx_PKTCNT(n)      (uint32_t) (n << 19)                                    /* USB device endpoint x IN packet count (10 bits) */
#define USB_FS_DIEPTSIZx_MCNT_1         (uint32_t) (1 << 29)                                    /* USB device endpoint x IN multi count 1 */
#define USB_FS_DIEPTSIZx_MCNT_2         (uint32_t) (2 << 29)                                    /* USB device endpoint x IN multi count 1 */
#define USB_FS_DIEPTSIZx_MCNT_3         (uint32_t) (3 << 29)                                    /* USB device endpoint x IN multi count 1 */

#define USB_FS_DOEPTSIZ0_XFRSIZ(n)      (uint32_t) (n << 0)                                     /* USB device endpoint 0 OUT transfer size (7 bits) */
#define USB_FS_DOEPTSIZ0_PKTCNT(n)      (uint32_t) (n << 19)                                    /* USB device endpoint 0 OUT packet count */
#define USB_FS_DOEPTSIZ0_STUPCNT(n)     (uint32_t) (n << 29)                                    /* USB device endpoint 0 OUT back-to-back SETUP packets allowed */

#define USB_FS_DOEPTSIZx_XFRSIZ(n)      (uint32_t) (n << 0)                                     /* USB device endpoint x OUT transfer size (19 bits) */
#define USB_FS_DOEPTSIZx_PKTCNT(n)      (uint32_t) (n << 19)                                    /* USB device endpoint x OUT packet count (10 bits) */
#define USB_FS_DOEPTSIZx_STUPCNT(n)     (uint32_t) (n << 29)                                    /* USB device endpoint x OUT back-to-back SETUP packets allowed */
#define USB_FS_DOEPTSIZx_RXDPID         (uint32_t) (1 << 29 | 1 << 30)                          /* USB device endpoint x OUT received data PID */

/* Powe and Clock Gating Control Register */
#define USB_FS_PCGCCTL_STPPCLK          (uint32_t) (1 << 0)                                     /* USB stop PHY clock */
#define USB_FS_PCGCCTL_GATEHCLK         (uint32_t) (1 << 1)                                     /* USB gate HCLK */
#define USB_FS_PCGCCTL_PHYSUSP          (uint32_t) (1 << 4)                                     /* USB PHY suspended */

/* FMC definition */
#define FMC_R_BASE                      ((uint32_t) 0xA0000000)                                 /* FMC registers base address */

#define FMC_Bank5_6_R_BASE              (FMC_R_BASE + 0x0140)

#define FMC_Bank5_6_SDCR(n)             (volatile uint32_t *) (FMC_Bank5_6_R_BASE + 0x00 + (4*n))      /* SDRAM Control Register */
#define FMC_Bank5_6_SDTR(n)             (volatile uint32_t *) (FMC_Bank5_6_R_BASE + 0x08 + (4*n))      /* SDRAM Timing Register */
#define FMC_Bank5_6_SDCMR               (volatile uint32_t *) (FMC_Bank5_6_R_BASE + 0x10)      /* SDRAM Command Mode Register */
#define FMC_Bank5_6_SDRTR               (volatile uint32_t *) (FMC_Bank5_6_R_BASE + 0x14)      /* SDRAM Refresh Timer Register */
#define FMC_Bank5_6_SDSR                (volatile uint32_t *) (FMC_Bank5_6_R_BASE + 0x18)      /* SDRAM Status Register */

/* RCC_PLL_Clock definition */
#define RCC_PLLSAIDivR_Div2             ((uint32_t)0x00000000)
#define RCC_PLLSAIDivR_Div4             ((uint32_t)0x00010000)
#define RCC_PLLSAIDivR_Div8             ((uint32_t)0x00020000)
#define RCC_PLLSAIDivR_Div16            ((uint32_t)0x00030000)

/* RCC_DCKCFGR definition */
#define  RCC_DCKCFGR_PLLI2SDIVQ         ((uint32_t)0x0000001F)
#define  RCC_DCKCFGR_PLLSAIDIVQ         ((uint32_t)0x00001F00)
#define  RCC_DCKCFGR_PLLSAIDIVR         ((uint32_t)0x00030000)
#define  RCC_DCKCFGR_SAI1ASRC           ((uint32_t)0x00300000)
#define  RCC_DCKCFGR_SAI1BSRC           ((uint32_t)0x00C00000)
#define  RCC_DCKCFGR_TIMPRE             ((uint32_t)0x01000000)

/* LTDC definition */
#define LTDC_Layer_BASE(layer)          (LTDC_BASE + 0x84 + ((layer - 1) * 0x80))
#define LTDC_Layer1                     1
#define LTDC_Layer2                     2
#define LTDC_Layer1_BASE                LTDC_Layer_BASE(LTDC_Layer1)
#define LTDC_Layer2_BASE                LTDC_Layer_BASE(LTDC_Layer2)

#define LTDC_SSCR                       (volatile uint32_t *) (LTDC_BASE + 0x08)                /* LTDC Synchronization Size Configuration Register */
#define LTDC_BPCR                       (volatile uint32_t *) (LTDC_BASE + 0x0C)                /* LTDC Back Porch Configuration Register */
#define LTDC_AWCR                       (volatile uint32_t *) (LTDC_BASE + 0x10)                /* LTDC Active Width Configuration Register */
#define LTDC_TWCR                       (volatile uint32_t *) (LTDC_BASE + 0x14)                /* LTDC Total Width Configuration Register */
#define LTDC_GCR                        (volatile uint32_t *) (LTDC_BASE + 0x18)                /* LTDC Global Control Register */
#define LTDC_SRCR                       (volatile uint32_t *) (LTDC_BASE + 0x24)                /* LTDC Shadow Reload Configuration Register */
#define LTDC_BCCR                       (volatile uint32_t *) (LTDC_BASE + 0x2C)                /* LTDC Background Color Configuration Register */
#define LTDC_IER                        (volatile uint32_t *) (LTDC_BASE + 0x34)                /* LTDC Interrupt Enable Register */
#define LTDC_ISR                        (volatile uint32_t *) (LTDC_BASE + 0x38)                /* LTDC Interrupt Status Register */
#define LTDC_ICR                        (volatile uint32_t *) (LTDC_BASE + 0x3C)                /* LTDC Interrupt Clear Register */
#define LTDC_LIPCR                      (volatile uint32_t *) (LTDC_BASE + 0x40)                /* LTDC Line Interrupt Position Configuration Register */
#define LTDC_CPSR                       (volatile uint32_t *) (LTDC_BASE + 0x44)                /* LTDC Current Position Status Register */
#define LTDC_CDSR                       (volatile uint32_t *) (LTDC_BASE + 0x48)                /* LTDC Current Display Status Register */

#define LTDC_SSCR_VSH                   ((uint32_t)0x000007FF)                                  /* Vertical Synchronization Height */
#define LTDC_SSCR_HSW                   ((uint32_t)0x0FFF0000)                                  /* Horizontal Synchronization Width */

#define LTDC_BPCR_AVBP                  ((uint32_t)0x000007FF)                                  /* Accumulated Vertical Back Porch */
#define LTDC_BPCR_AHBP                  ((uint32_t)0x0FFF0000)                                  /* Accumulated Horizontal Back Porch */

#define LTDC_AWCR_AAH                   ((uint32_t)0x000007FF)                                  /* Accumulated Active heigh */
#define LTDC_AWCR_AAW                   ((uint32_t)0x0FFF0000)                                  /* Accumulated Active Width */

#define LTDC_TWCR_TOTALH                ((uint32_t)0x000007FF)                                  /* Total Heigh */
#define LTDC_TWCR_TOTALW                ((uint32_t)0x0FFF0000)                                  /* Total Width */

#define LTDC_GCR_LTDCEN                 ((uint32_t)0x00000001)                                  /* LCD-TFT controller enable bit */
#define LTDC_GCR_DBW                    ((uint32_t)0x00000070)                                  /* Dither Blue Width */
#define LTDC_GCR_DGW                    ((uint32_t)0x00000700)                                  /* Dither Green Width */
#define LTDC_GCR_DRW                    ((uint32_t)0x00007000)                                  /* Dither Red Width */
#define LTDC_GCR_DTEN                   ((uint32_t)0x00010000)                                  /* Dither Enable */
#define LTDC_GCR_PCPOL                  ((uint32_t)0x10000000)                                  /* Pixel Clock Polarity */
#define LTDC_GCR_DEPOL                  ((uint32_t)0x20000000)                                  /* Data Enable Polarity */
#define LTDC_GCR_VSPOL                  ((uint32_t)0x40000000)                                  /* Vertical Synchronization Polarity */
#define LTDC_GCR_HSPOL                  ((uint32_t)0x80000000)                                  /* Horizontal Synchronization Polarity */

#define LTDC_SRCR_IMR                   ((uint32_t) 1 << 0)                                     /* Immediate Reload */
#define LTDC_SRCR_VBR                   ((uint32_t) 1 << 1)                                     /* Vertical Blanking Reload */

#define LTDC_BCCR_BCBLUE                ((uint32_t)0x000000FF)                                  /* Background Blue value */
#define LTDC_BCCR_BCGREEN               ((uint32_t)0x0000FF00)                                  /* Background Green value */
#define LTDC_BCCR_BCRED                 ((uint32_t)0x00FF0000)                                  /* Background Red value */

#define LTDC_IER_LIE                    ((uint32_t) 1 << 0)                                    /* Line Interrupt Enable */
#define LTDC_IER_FUIE                   ((uint32_t) 1 << 1)                                    /* FIFO Underrun Interrupt Enable */
#define LTDC_IER_TERRIE                 ((uint32_t) 1 << 2)                                    /* Transfer Error Interrupt Enable */
#define LTDC_IER_RRIE                   ((uint32_t) 1 << 3)                                    /* Register Reload interrupt enable */

#define LTDC_ISR_LIF                    ((uint32_t) 1 << 0)                                    /* Line Interrupt Flag */
#define LTDC_ISR_FUIF                   ((uint32_t) 1 << 1)                                    /* FIFO Underrun Interrupt Flag */
#define LTDC_ISR_TERRIF                 ((uint32_t) 1 << 2)                                    /* Transfer Error Interrupt Flag */
#define LTDC_ISR_RRIF                   ((uint32_t) 1 << 3)                                    /* Register Reload interrupt Flag */

#define LTDC_ICR_CLIF                   ((uint32_t) 1 << 0)                                    /* Clears the Line Interrupt Flag */
#define LTDC_ICR_CFUIF                  ((uint32_t) 1 << 1)                                    /* Clears the FIFO Underrun Interrupt Flag */
#define LTDC_ICR_CTERRIF                ((uint32_t) 1 << 2)                                    /* Clears the Transfer Error Interrupt Flag */
#define LTDC_ICR_CRRIF                  ((uint32_t) 1 << 3)                                    /* Clears Register Reload interrupt Flag */

#define LTDC_LIPCR_LIPOS                ((uint32_t)0x000007FF)                                 /* Line Interrupt Position */

#define LTDC_CPSR_CYPOS                 ((uint32_t)0x0000FFFF)                                 /* Current Y Position */
#define LTDC_CPSR_CXPOS                 ((uint32_t)0xFFFF0000)                                 /* Current X Position */

#define LTDC_CDSR_VDES                  ((uint32_t) 1 << 0)                                    /* Vertical Data Enable Status */
#define LTDC_CDSR_HDES                  ((uint32_t) 1 << 1)                                    /* Horizontal Data Enable Status */
#define LTDC_CDSR_VSYNCS                ((uint32_t) 1 << 2)                                    /* Vertical Synchronization Status */
#define LTDC_CDSR_HSYNCS                ((uint32_t) 1 << 3)                                    /* Horizontal Synchronization Status */

#define LTDC_Layer_CR(layer)            (volatile uint32_t *) (LTDC_Layer_BASE(layer) + 0x00)      /* Control Register */
#define LTDC_Layer_WHPCR(layer)         (volatile uint32_t *) (LTDC_Layer_BASE(layer) + 0x04)      /* Window Horizontal Position Configuration Register */
#define LTDC_Layer_WVPCR(layer)         (volatile uint32_t *) (LTDC_Layer_BASE(layer) + 0x08)      /* Window Vertical Position Configuration Register */
#define LTDC_Layer_CKCR(layer)          (volatile uint32_t *) (LTDC_Layer_BASE(layer) + 0x0C)      /* Color Keying Configuration Register */
#define LTDC_Layer_PFCR(layer)          (volatile uint32_t *) (LTDC_Layer_BASE(layer) + 0x10)      /* Pixel Format Configuration Register */
#define LTDC_Layer_CACR(layer)          (volatile uint32_t *) (LTDC_Layer_BASE(layer) + 0x14)      /* Constant Alpha Configuration Register */
#define LTDC_Layer_DCCR(layer)          (volatile uint32_t *) (LTDC_Layer_BASE(layer) + 0x18)      /* Default Color Configuration Register */
#define LTDC_Layer_BFCR(layer)          (volatile uint32_t *) (LTDC_Layer_BASE(layer) + 0x1C)      /* Blending Factors Configuration Register */
#define LTDC_Layer_CFBAR(layer)         (volatile uint32_t *) (LTDC_Layer_BASE(layer) + 0x28)      /* Color Frame Buffer Address Register */
#define LTDC_Layer_CFBLR(layer)         (volatile uint32_t *) (LTDC_Layer_BASE(layer) + 0x2C)      /* Color Frame Buffer Length Register */
#define LTDC_Layer_CFBLNR(layer)        (volatile uint32_t *) (LTDC_Layer_BASE(layer) + 0x30)      /* Color Frame Buffer Line Number Register */

#endif
