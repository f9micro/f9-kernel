/* STM32F1 Registers and Memory Locations */

#ifndef STM32F1_REGISTERS_H_INCLUDED
#define STM32F1_REGISTERS_H_INCLUDED

#include <stdint.h>

#define STM32F1X

/* Memory Map */
#define MEMORY_BASE                     (uint32_t) (0x00000000)                                 /* Base of memory map */
#define FLASH_BASE                      (uint32_t) (0x08000000)                                 /* Flash Memory Base Address */
#define SRAM_BASE                       (uint32_t) (0x20000000)                                 /* SRAM Base Address */

/* Peripheral Map */
#define PERIPH_BASE                     (uint32_t) (0x40000000)                                 /* Peripheral base address */
#define PRIV_PERIPH_BASE                (uint32_t) (0xED000000)                                 /* Private peripheral base address */
#define APB1PERIPH_BASE                 (PERIPH_BASE)
#define APB2PERIPH_BASE                 (PERIPH_BASE + 0x10000)
#define AHBPERIPH_BASE                  (PERIPH_BASE + 0x20000)



/* APB1 peripherals */
#define TIM2_BASE             (APB1PERIPH_BASE + 0x0000)
#define TIM3_BASE             (APB1PERIPH_BASE + 0x0400)
#define TIM4_BASE             (APB1PERIPH_BASE + 0x0800)
#define RTC_BASE              (APB1PERIPH_BASE + 0x2800)
#define WWDG_BASE             (APB1PERIPH_BASE + 0x2C00)
#define IWDG_BASE             (APB1PERIPH_BASE + 0x3000)
#define SPI2_BASE             (APB1PERIPH_BASE + 0x3800)
#define USART2_BASE           (APB1PERIPH_BASE + 0x4400)
#define USART3_BASE           (APB1PERIPH_BASE + 0x4800)
#define I2C1_BASE             (APB1PERIPH_BASE + 0x5400)
#define I2C2_BASE             (APB1PERIPH_BASE + 0x5800)
#define USB_BASE              (APB1PERIPH_BASE + 0x5C00)
#define CAN1_BASE             (APB1PERIPH_BASE + 0x6400)
#define BKP_BASE              (APB1PERIPH_BASE + 0x6C00)
#define PWR_BASE              (APB1PERIPH_BASE + 0x7000)

/* APB2 peripherals */
#define AFIO_BASE             (APB2PERIPH_BASE + 0x0000)
#define EXTI_BASE             (APB2PERIPH_BASE + 0x0400)
#define GPIO_BASE(port)       (APB2PERIPH_BASE + 0x0800 + (0x400*port))                        /* GPIO Port base address */
#define ADC1_BASE             (APB2PERIPH_BASE + 0x2400)
#define ADC2_BASE             (APB2PERIPH_BASE + 0x2800)
#define TIM1_BASE             (APB2PERIPH_BASE + 0x2C00)
#define SPI1_BASE             (APB2PERIPH_BASE + 0x3000)
#define USART1_BASE           (APB2PERIPH_BASE + 0x3800)

/* AHB */
#define DMA_BASE                        (AHBPERIPH_BASE)                                        /* DMA base address */
#define RCC_BASE                        (AHBPERIPH_BASE + 0x1000)                               /* Reset and Clock Control base address */
#define FLASH_R_BASE                    (AHBPERIPH_BASE + 0x2000)                               /* Flash registers base address */
#define CRC_BASE                        (AHBPERIPH_BASE + 0x3000)                               /* CRC base address */

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
#define SPI_BASE(port)                  (port)                                                  /* Temporary SPI base */
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
#define GPIO_CRL(port)                  (volatile uint32_t *) (GPIO_BASE(port) + 0x00)          /* Port configuration low register */
#define GPIO_CRH(port)                  (volatile uint32_t *) (GPIO_BASE(port) + 0x04)          /* Port configuration high register */
#define GPIO_IDR(port)                  (volatile uint32_t *) (GPIO_BASE(port) + 0x08)          /* Port input data register */
#define GPIO_ODR(port)                  (volatile uint32_t *) (GPIO_BASE(port) + 0x0C)          /* Port output data register */
#define GPIO_BSRR(port)                 (volatile uint32_t *) (GPIO_BASE(port) + 0x10)          /* Port bit set/reset register */
#define GPIO_BRR(port)                  (volatile uint32_t *) (GPIO_BASE(port) + 0x14)          /* Port bit reset register */
#define GPIO_LCKR(port)                 (volatile uint32_t *) (GPIO_BASE(port) + 0x18)          /* Port configuration lock register */

/* Reset and Clock Control (RCC) */
#define RCC_CR                          (volatile uint32_t *) (RCC_BASE + 0x00)                 /* Clock Control Register */
#define RCC_CFGR                        (volatile uint32_t *) (RCC_BASE + 0x04)                 /* Clock Configuration Register */
#define RCC_CIR                         (volatile uint32_t *) (RCC_BASE + 0x08)                 /* Clock Interrupt Register */
#define RCC_APB2RSTR                    (volatile uint32_t *) (RCC_BASE + 0x0C)                 /* APB2 reset Register */
#define RCC_APB1RSTR                    (volatile uint32_t *) (RCC_BASE + 0x10)                 /* APB1 reset Register */
#define RCC_AHBENR                      (volatile uint32_t *) (RCC_BASE + 0x14)                 /* AHB Enable Register */
#define RCC_APB2ENR                     (volatile uint32_t *) (RCC_BASE + 0x18)                 /* APB2 Peripheral Clock Enable Register */
#define RCC_APB1ENR                     (volatile uint32_t *) (RCC_BASE + 0x1C)                 /* APB1 Peripheral Clock Enable Register */
#define RCC_BDCR                        (volatile uint32_t *) (RCC_BASE + 0x20)                 /* RCC Backup Domain Control Register */
#define RCC_CSR                         (volatile uint32_t *) (RCC_BASE + 0x24)                 /* RCC Clock Control & Status Register */
#define RCC_AHBRSTR                     (volatile uint32_t *) (RCC_BASE + 0x28)                 /* AHB reset Register */

/* APB1 RCC ENR */
#define RCC_TIM2_APBENR					RCC_APB1ENR
#define RCC_TIM3_APBENR					RCC_APB1ENR
#define RCC_TIM4_APBENR					RCC_APB1ENR
#define RCC_RTC_APBENR					RCC_APB1ENR
#define RCC_WWDG_APBENR					RCC_APB1ENR
#define RCC_IWDG_APBENR					RCC_APB1ENR
#define RCC_SPI2_APBENR					RCC_APB1ENR
#define RCC_USART2_APBENR				RCC_APB1ENR
#define RCC_USART3_APBENR				RCC_APB1ENR
#define RCC_I2C1_APBENR					RCC_APB1ENR
#define RCC_I2C2_APBENR					RCC_APB1ENR
#define RCC_CAN1_APBENR					RCC_APB1ENR
#define RCC_BKP_APBENR					RCC_APB1ENR
#define RCC_PWR_APBENR					RCC_APB1ENR

/* APB2 RCC ENR */
#define RCC_AFIO_APBENR 				RCC_APB2ENR
#define RCC_EXTI_APBENR 				RCC_APB2ENR
#define RCC_GPIO_APBENR					RCC_APB2ENR
#define RCC_ADC1_APBENR					RCC_APB2ENR
#define RCC_ADC2_APBENR					RCC_APB2ENR
#define RCC_TIM1_APBENR					RCC_APB2ENR
#define RCC_SPI1_APBENR					RCC_APB2ENR
#define RCC_USART1_APBENR				RCC_APB2ENR

/* Flash Registers (FLASH) */
#define FLASH_ACR                       (volatile uint32_t *) (FLASH_R_BASE + 0x00)             /* Flash Access Control Register */

/* Direct Memory Access  (DMA) */
#define DMA_LISR                       (volatile uint32_t *) (DMA_BASE + 0x00)                  /* DMA interrupt status register */
#define DMA_IFCR                       (volatile uint32_t *) (DMA_BASE + 0x04)                  /* DMA interrupt flag clear register */
/* Channel n */
#define DMA_CR_C(n)                    (volatile uint32_t *) (DMA_BASE + 0x08 + (0x14*n))      /* DMA stream n configuration register */
#define DMA_NDTR_C(n)                  (volatile uint32_t *) (DMA_BASE + 0x0C + (0x14*n))      /* DMA stream n number of data register */
#define DMA_PAR_C(n)                   (volatile uint32_t *) (DMA_BASE + 0x10 + (0x14*n))      /* DMA stream n peripheral address register */
#define DMA_MAR_C(n)                   (volatile uint32_t *) (DMA_BASE + 0x14 + (0x14*n))      /* DMA stream n memory 0 address register */

/**********************************************************************************************************************************************/

/* Bit Masks - See RM0090 Reference Manual for STM32F4 for details */

#define RCC_CR_HSION                    (uint32_t) (1 << 0)                                     /* HSI clock enable */
#define RCC_CR_HSIRDY                   (uint32_t) (1 << 1)                                     /* HSI ready */
#define RCC_CR_HSITRIM_M                (uint32_t) (0xF8 << 0)                                  /* HSI trimming mask */
#define RCC_CR_HSICAL_M                 (uint32_t) (0xFF << 8)                                  /* HSI calibration mask */
#define RCC_CR_HSEON                    (uint32_t) (1 << 16)                                    /* HSE clock enable */
#define RCC_CR_HSERDY                   (uint32_t) (1 << 17)                                    /* HSE ready */
#define RCC_CR_HSEBYP                   (uint32_t) (1 << 18)                                    /* HSE bypass */
#define RCC_CR_CSSON                    (uint32_t) (1 << 19)                                    /* Clock security system enable */
#define RCC_CR_PLLON                    (uint32_t) (1 << 24)                                    /* Main PLL enable */
#define RCC_CR_PLLRDY                   (uint32_t) (1 << 25)                                    /* Main PLL clock ready */

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
#define RCC_CFGR_PPRE1_M                (uint32_t) (0x7 << 8)                                   /* APB low speed prescaler mask */
#define RCC_CFGR_PPRE1_DIV1             (uint32_t) (0x0 << 8)                                   /* APB low speed prescaler - HCLK/1 */
#define RCC_CFGR_PPRE1_DIV2             (uint32_t) (0x4 << 8)                                   /* APB low speed prescaler - HCLK/2 */
#define RCC_CFGR_PPRE1_DIV4             (uint32_t) (0x5 << 8)                                   /* APB low speed prescaler - HCLK/4 */
#define RCC_CFGR_PPRE1_DIV8             (uint32_t) (0x6 << 8)                                   /* APB low speed prescaler - HCLK/8 */
#define RCC_CFGR_PPRE1_DIV16            (uint32_t) (0x7 << 8)                                   /* APB low speed prescaler - HCLK/16 */
#define RCC_CFGR_PPRE2_M                (uint32_t) (0x38 << 8)                                  /* APB high speec prescaler mask */
#define RCC_CFGR_PPRE2_DIV1             (uint32_t) (0x0 << 8)                                   /* APB high speed prescaler - HCLK/1 */
#define RCC_CFGR_PPRE2_DIV2             (uint32_t) (0x20 << 8)                                  /* APB high speed prescaler - HCLK/2 */
#define RCC_CFGR_PPRE2_DIV4             (uint32_t) (0x28 << 8)                                  /* APB high speed prescaler - HCLK/4 */
#define RCC_CFGR_PPRE2_DIV8             (uint32_t) (0x30 << 8)                                  /* APB high speed prescaler - HCLK/8 */
#define RCC_CFGR_PPRE2_DIV16            (uint32_t) (0x38 << 8)                                  /* APB high speed prescaler - HCLK/16 */
#define RCC_CFGR_RTCPRE_M               (uint32_t) (0x1F << 16)                                 /* HSE division factor for RTC clock mask */
#define RCC_CFGR_RTCPRE(n)              (uint32_t) (n << 16)                                    /* HSE division factor for RTC clock */

#define RCC_AHBRSTR_ETHMACRST          (uint32_t) (1 << 14)                                    /* Ethernet MAC reset */
#define RCC_AHBRSTR_OTGFSRST           (uint32_t) (1 << 12)                                    /* USB OTG FS reset */

#define RCC_APB1RSTR_TIM2RST            (uint32_t) (1 << 0)                                     /* TIM2 reset */
#define RCC_APB1RSTR_TIM3RST            (uint32_t) (1 << 1)                                     /* TIM3 reset */
#define RCC_APB1RSTR_TIM4RST            (uint32_t) (1 << 2)                                     /* TIM4 reset */
#define RCC_APB1RSTR_WWDGRST            (uint32_t) (1 << 11)                                    /* Window watchdog reset */
#define RCC_APB1RSTR_SPI2RST            (uint32_t) (1 << 14)                                    /* SPI2 reset */
#define RCC_APB1RSTR_USART2RST          (uint32_t) (1 << 17)                                    /* USART2 reset */
#define RCC_APB1RSTR_USART3RST          (uint32_t) (1 << 18)                                    /* USART3 reset */
#define RCC_APB1RSTR_I2C1RST            (uint32_t) (1 << 21)                                    /* I2C1 reset */
#define RCC_APB1RSTR_I2C2RST            (uint32_t) (1 << 22)                                    /* I2C2 reset */
#define RCC_APB1RSTR_CAN1RST            (uint32_t) (1 << 25)                                    /* CAN1 reset */
#define RCC_APB1RSTR_BKPRST             (uint32_t) (1 << 27)                                    /* Backup interface reset */
#define RCC_APB1RSTR_PWRRST             (uint32_t) (1 << 28)                                    /* Power interface reset */
#define RCC_APB1RSTR_DACRST             (uint32_t) (1 << 29)                                    /* DAC reset */

#define RCC_APB2RSTR_AFIORST            (uint32_t) (1 << 0)                                     /* Alternate Function I/O reset */
#define RCC_APB2RSTR_IOPARST            (uint32_t) (1 << 2)                                     /* Alternate Function I/O port A reset */
#define RCC_APB2RSTR_IOPBRST            (uint32_t) (1 << 3)                                     /* Alternate Function I/O port B reset */
#define RCC_APB2RSTR_IOPCRST            (uint32_t) (1 << 4)                                     /* Alternate Function I/O port C reset */
#define RCC_APB2RSTR_IOPDRST            (uint32_t) (1 << 5)                                     /* Alternate Function I/O port D reset */
#define RCC_APB2RSTR_IOPERST            (uint32_t) (1 << 6)                                     /* Alternate Function I/O port E reset */
#define RCC_APB2RSTR_ADC1RST            (uint32_t) (1 << 9)                                     /* ADC 1 interface reset */
#define RCC_APB2RSTR_ADC2RST            (uint32_t) (1 << 10)                                    /* ADC 2 interface reset */
#define RCC_APB2RSTR_TIM1RST            (uint32_t) (1 << 11)                                    /* TIM1 reset */
#define RCC_APB2RSTR_SPI1RST            (uint32_t) (1 << 12)                                    /* SPI 1 reset */
#define RCC_APB2RSTR_USART1RST          (uint32_t) (1 << 14)                                    /* USART1 reset */

#define RCC_AHBENR_DMA1EN               (uint16_t) (1 << 0)                                     /* DMA1 clock enable */
#define RCC_AHBENR_SRAMEN               (uint16_t) (1 << 2)                                     /* SRAM interface clock enable */
#define RCC_AHBENR_FLITFEN              (uint16_t) (1 << 4)                                     /* FLITF clock enable */
#define RCC_AHBENR_CRCEN                (uint16_t) (1 << 6)                                     /* CRC clock enable */

#define RCC_APB1ENR_TIM2EN              (uint32_t) (1 << 0)                                     /* TIM2 clock enable */
#define RCC_APB1ENR_TIM3EN              (uint32_t) (1 << 1)                                     /* TIM3 clock enable */
#define RCC_APB1ENR_TIM4EN              (uint32_t) (1 << 2)                                     /* TIM4 clock enable */
#define RCC_APB1ENR_WWDGEN              (uint32_t) (1 << 11)                                    /* Window watchdog clock enable */
#define RCC_APB1ENR_SPI2EN              (uint32_t) (1 << 14)                                    /* SPI2 clock enable */
#define RCC_APB1ENR_USART2EN            (uint32_t) (1 << 17)                                    /* USART2 clock enable */
#define RCC_APB1ENR_USART3EN            (uint32_t) (1 << 18)                                    /* USART3 clock enable */
#define RCC_APB1ENR_I2C1EN              (uint32_t) (1 << 21)                                    /* I2C1 clock enable */
#define RCC_APB1ENR_I2C2EN              (uint32_t) (1 << 22)                                    /* I2C2 clock enable */
#define RCC_APB1ENR_CAN1EN              (uint32_t) (1 << 25)                                    /* CAN1 clock enable */
#define RCC_APB1ENR_BKPEN               (uint32_t) (1 << 27)                                    /* Backup interface clock enable */
#define RCC_APB1ENR_PWREN               (uint32_t) (1 << 28)                                    /* Power interface clock enable */

#define RCC_APB2ENR_AFIOEN              (uint32_t) (1 << 0)                                     /* Alternate Function I/O clock enable */
#define RCC_APB2ENR_IOPAEN              (uint32_t) (1 << 2)                                     /* Alternate Function I/O port A clock enable */
#define RCC_APB2ENR_IOPBEN              (uint32_t) (1 << 3)                                     /* Alternate Function I/O port B clock enable */
#define RCC_APB2ENR_IOPCEN              (uint32_t) (1 << 4)                                     /* Alternate Function I/O port C clock enable */
#define RCC_APB2ENR_IOPDEN              (uint32_t) (1 << 5)                                     /* Alternate Function I/O port D clock enable */
#define RCC_APB2ENR_IOPEEN              (uint32_t) (1 << 6)                                     /* Alternate Function I/O port E clock enable */
#define RCC_APB2ENR_ADC1EN              (uint32_t) (1 << 9)                                     /* ADC 1 interface clock enable */
#define RCC_APB2ENR_ADC2EN              (uint32_t) (1 << 10)                                    /* ADC 2 interface clock enable */
#define RCC_APB2ENR_TIM1EN              (uint32_t) (1 << 11)                                    /* TIM1 clock enable */
#define RCC_APB2ENR_SPI1EN              (uint32_t) (1 << 12)                                    /* SPI1 clock enable */
#define RCC_APB2ENR_USART1EN            (uint32_t) (1 << 14)                                     /* USART1 clock enable */

#define FLASH_ACR_PRFTEN                (uint32_t) (1 << 4)                                     /* Prefetch enable */
#define FLASH_ACR_LATENCY_M             (uint32_t) (3 << 0)                                     /* Latency mask */
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

#define I2C_CR2_FREQ(n)                 (uint32_t) (n << 0)                                     /* I2C clock frequency */
#define I2C_CR2_ITERREN                 (uint32_t) (1 << 8)                                     /* I2C error interrupt enable */
#define I2C_CR2_ITEVTEN                 (uint32_t) (1 << 9)                                     /* I2C event interrupt enable */
#define I2C_CR2_ITBUFEN                 (uint32_t) (1 << 10)                                    /* I2C buffer interrupt enable */
#define I2C_CR2_DMAEN                   (uint32_t) (1 << 11)                                    /* I2C DMA requests enable */
#define I2C_CR2_LAST                    (uint32_t) (1 << 12)                                    /* I2C DMA last transfer */

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

#define GPIO_CR_PIN(n)                  (uint32_t) (4*n)                                        /* Pin bitshift */
#define GPIO_CR_M(n)                    (uint32_t) (0x3 << 4*n)                                 /* Pin mask */

#define GPIO_OSPEED_10M                 (uint32_t) (0x1)                                        /* Output speed 10MHz */
#define GPIO_OSPEED_2M                  (uint32_t) (0x2)                                        /* Output speed 2MHz */
#define GPIO_OSPEED_50M                 (uint32_t) (0x3)                                        /* Output speed 50MHz */

#define GPIO_MODE_IN_ANALOG             (uint32_t) (0x00)                                       /* Input analog mode */
#define GPIO_MODE_IN_FLOAT              (uint32_t) (0x01)                                       /* Input floating point mode */
#define GPIO_MODE_IN_PD                 (uint32_t) (0x0A)                                       /* Input push down mode */
#define GPIO_MODE_IN_PU                 (uint32_t) (0x12)                                       /* Input pull up mode */
#define GPIO_MODE_OUT_PP                (uint32_t) (0x04)                                       /* Output push-pull mode */
#define GPIO_MODE_OUT_OD                (uint32_t) (0x05)                                       /* Output open-drain mode */
#define GPIO_MODE_OUT_ALT_PP            (uint32_t) (0x06)                                       /* Output alternate function push-pull mode */
#define GPIO_MODE_OUT_ALT_OD            (uint32_t) (0x07)                                       /* Output alternate function open-drain mode */

#define GPIO_IDR_PIN(n)                 (uint32_t) (1 << n)                                     /* Input for pin n */

#define GPIO_ODR_PIN(n)                 (uint32_t) (1 << n)                                     /* Output for pin n */

#define GPIO_BSRR_BS(n)                 (uint32_t) (1 << n)                                     /* Set pin n */
#define GPIO_BSRR_BR(n)                 (uint32_t) (1 << (n+16))                                /* Reset pin n */
#define GPIO_BRR_BR(n)                  (uint16_t) (1 << n)                                     /* Reset pin n */
#define GPIO_LCKR_LCK(n)                (uint32_t) (1 << n)                                     /* Lock pin n */

/* DMA */
#define DMA_ISR_GIF(n)                  (uint32_t) (0x1 << ((n-1)*4))                           /* DMA Channel n Global interrupt flag */
#define DMA_ISR_TCIF(n)                 (uint32_t) (0x2 << ((n-1)*4))                           /* DMA Channel n Transfer Complete flag */
#define DMA_ISR_HTIF(n)                 (uint32_t) (0x4 << ((n-1)*4))                           /* DMA Channel n Half Transfer flag */
#define DMA_ISR_TEIF(n)                 (uint32_t) (0x8 << ((n-1)*4))                           /* DMA Channel n Transfer Error flag */

#define DMA_IFCR_CGIF(n)                (uint32_t) (0x1 << ((n-1)*4))                           /* DMA Channel n Global interrupt clear */
#define DMA_IFCR_CTCIF(n)               (uint32_t) (0x2 << ((n-1)*4))                           /* DMA Channel n Transfer Complete clear */
#define DMA_IFCR_CHTIF(n)               (uint32_t) (0x4 << ((n-1)*4))                           /* DMA Channel n Half Transfer clear */
#define DMA_IFCR_CTEIF(n)               (uint32_t) (0x8 << ((n-1)*4))                           /* DMA Channel n Transfer Error clear */

#define DMA_CCR_EN                      (uint16_t) (1 << 0)                                     /* DMA Channel enable */
#define DMA_CCR_TCIE                    (uint16_t) (1 << 1)                                     /* DMA Channel transfer complete interrupt enable */
#define DMA_CCR_HTIE                    (uint16_t) (1 << 2)                                     /* DMA Channel half transfer interrupt enable */
#define DMA_CCR_TEIE                    (uint16_t) (1 << 3)                                     /* DMA Channel transfer error interrupt error enable */
#define DMA_CCR_DIR                     (uint16_t) (1 << 4)                                     /* DMA Channel data transfer direction */
#define DMA_CCR_CIRC                    (uint16_t) (1 << 5)                                     /* DMA Channel circular mode */
#define DMA_CCR_PINC                    (uint16_t) (1 << 6)                                     /* DMA Channel peripheral increment mode */
#define DMA_CCR_MINC                    (uint16_t) (1 << 7)                                     /* DMA Channel Memory increment mode */
#define DMA_CCR_PL                      (uint16_t) (3 << 12)                                    /* DMA Channel Priority level mask */
#define DMA_CCR_MEM2MEM                 (uint16_t) (1 << 14)                                    /* DMA Channel Memory to memory mode */

#endif
