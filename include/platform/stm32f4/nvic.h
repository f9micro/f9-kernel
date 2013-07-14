/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __PLATFORM_STM32F4_NVIC_H__
#define __PLATFORM_STM32F4_NVIC_H__

#include <platform/stm32f4/registers.h>
#include <platform/cortex_m.h>

#define IRQ_ENABLE	1
#define IRQ_DISABLE	0

typedef enum IRQn {
	/******  Cortex-M4 Processor Exceptions Numbers ****************************************************************/
	NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                                          */
	MemoryManagement_IRQn       = -12,    /*!< 4 Cortex-M4 Memory Management Interrupt                           */
	BusFault_IRQn               = -11,    /*!< 5 Cortex-M4 Bus Fault Interrupt                                   */
	UsageFault_IRQn             = -10,    /*!< 6 Cortex-M4 Usage Fault Interrupt                                 */
	SVCall_IRQn                 = -5,     /*!< 11 Cortex-M4 SV Call Interrupt                                    */
	DebugMonitor_IRQn           = -4,     /*!< 12 Cortex-M4 Debug Monitor Interrupt                              */
	PendSV_IRQn                 = -2,     /*!< 14 Cortex-M4 Pend SV Interrupt                                    */
	SysTick_IRQn                = -1,     /*!< 15 Cortex-M4 System Tick Interrupt                                */
	/******  STM32 specific Interrupt Numbers **********************************************************************/
	WWDG_IRQn                   = 0,      /*!< Window WatchDog Interrupt                                         */
	PVD_IRQn                    = 1,      /*!< PVD through EXTI Line detection Interrupt                         */
	TAMP_STAMP_IRQn             = 2,      /*!< Tamper and TimeStamp interrupts through the EXTI line             */
	RTC_WKUP_IRQn               = 3,      /*!< RTC Wakeup interrupt through the EXTI line                        */
	FLASH_IRQn                  = 4,      /*!< FLASH global Interrupt                                            */
	RCC_IRQn                    = 5,      /*!< RCC global Interrupt                                              */
	EXTI0_IRQn                  = 6,      /*!< EXTI Line0 Interrupt                                              */
	EXTI1_IRQn                  = 7,      /*!< EXTI Line1 Interrupt                                              */
	EXTI2_IRQn                  = 8,      /*!< EXTI Line2 Interrupt                                              */
	EXTI3_IRQn                  = 9,      /*!< EXTI Line3 Interrupt                                              */
	EXTI4_IRQn                  = 10,     /*!< EXTI Line4 Interrupt                                              */
	DMA1_Stream0_IRQn           = 11,     /*!< DMA1 Stream 0 global Interrupt                                    */
	DMA1_Stream1_IRQn           = 12,     /*!< DMA1 Stream 1 global Interrupt                                    */
	DMA1_Stream2_IRQn           = 13,     /*!< DMA1 Stream 2 global Interrupt                                    */
	DMA1_Stream3_IRQn           = 14,     /*!< DMA1 Stream 3 global Interrupt                                    */
	DMA1_Stream4_IRQn           = 15,     /*!< DMA1 Stream 4 global Interrupt                                    */
	DMA1_Stream5_IRQn           = 16,     /*!< DMA1 Stream 5 global Interrupt                                    */
	DMA1_Stream6_IRQn           = 17,     /*!< DMA1 Stream 6 global Interrupt                                    */
	ADC_IRQn                    = 18,     /*!< ADC1, ADC2 and ADC3 global Interrupts                             */
	CAN1_TX_IRQn                = 19,     /*!< CAN1 TX Interrupt                                                 */
	CAN1_RX0_IRQn               = 20,     /*!< CAN1 RX0 Interrupt                                                */
	CAN1_RX1_IRQn               = 21,     /*!< CAN1 RX1 Interrupt                                                */
	CAN1_SCE_IRQn               = 22,     /*!< CAN1 SCE Interrupt                                                */
	EXTI9_5_IRQn                = 23,     /*!< External Line[9:5] Interrupts                                     */
	TIM1_BRK_TIM9_IRQn          = 24,     /*!< TIM1 Break interrupt and TIM9 global interrupt                    */
	TIM1_UP_TIM10_IRQn          = 25,     /*!< TIM1 Update Interrupt and TIM10 global interrupt                  */
	TIM1_TRG_COM_TIM11_IRQn     = 26,     /*!< TIM1 Trigger and Commutation Interrupt and TIM11 global interrupt */
	TIM1_CC_IRQn                = 27,     /*!< TIM1 Capture Compare Interrupt                                    */
	TIM2_IRQn                   = 28,     /*!< TIM2 global Interrupt                                             */
	TIM3_IRQn                   = 29,     /*!< TIM3 global Interrupt                                             */
	TIM4_IRQn                   = 30,     /*!< TIM4 global Interrupt                                             */
	I2C1_EV_IRQn                = 31,     /*!< I2C1 Event Interrupt                                              */
	I2C1_ER_IRQn                = 32,     /*!< I2C1 Error Interrupt                                              */
	I2C2_EV_IRQn                = 33,     /*!< I2C2 Event Interrupt                                              */
	I2C2_ER_IRQn                = 34,     /*!< I2C2 Error Interrupt                                              */
	SPI1_IRQn                   = 35,     /*!< SPI1 global Interrupt                                             */
	SPI2_IRQn                   = 36,     /*!< SPI2 global Interrupt                                             */
	USART1_IRQn                 = 37,     /*!< USART1 global Interrupt                                           */
	USART2_IRQn                 = 38,     /*!< USART2 global Interrupt                                           */
	USART3_IRQn                 = 39,     /*!< USART3 global Interrupt                                           */
	EXTI15_10_IRQn              = 40,     /*!< External Line[15:10] Interrupts                                   */
	RTC_Alarm_IRQn              = 41,     /*!< RTC Alarm (A and B) through EXTI Line Interrupt                   */
	OTG_FS_WKUP_IRQn            = 42,     /*!< USB OTG FS Wakeup through EXTI line interrupt                     */
	TIM8_BRK_TIM12_IRQn         = 43,     /*!< TIM8 Break Interrupt and TIM12 global interrupt                   */
	TIM8_UP_TIM13_IRQn          = 44,     /*!< TIM8 Update Interrupt and TIM13 global interrupt                  */
	TIM8_TRG_COM_TIM14_IRQn     = 45,     /*!< TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt */
	TIM8_CC_IRQn                = 46,     /*!< TIM8 Capture Compare Interrupt                                    */
	DMA1_Stream7_IRQn           = 47,     /*!< DMA1 Stream7 Interrupt                                            */
	FSMC_IRQn                   = 48,     /*!< FSMC global Interrupt                                             */
	SDIO_IRQn                   = 49,     /*!< SDIO global Interrupt                                             */
	TIM5_IRQn                   = 50,     /*!< TIM5 global Interrupt                                             */
	SPI3_IRQn                   = 51,     /*!< SPI3 global Interrupt                                             */
	UART4_IRQn                  = 52,     /*!< UART4 global Interrupt                                            */
	UART5_IRQn                  = 53,     /*!< UART5 global Interrupt                                            */
	TIM6_DAC_IRQn               = 54,     /*!< TIM6 global and DAC1&2 underrun error  interrupts                 */
	TIM7_IRQn                   = 55,     /*!< TIM7 global interrupt                                             */
	DMA2_Stream0_IRQn           = 56,     /*!< DMA2 Stream 0 global Interrupt                                    */
	DMA2_Stream1_IRQn           = 57,     /*!< DMA2 Stream 1 global Interrupt                                    */
	DMA2_Stream2_IRQn           = 58,     /*!< DMA2 Stream 2 global Interrupt                                    */
	DMA2_Stream3_IRQn           = 59,     /*!< DMA2 Stream 3 global Interrupt                                    */
	DMA2_Stream4_IRQn           = 60,     /*!< DMA2 Stream 4 global Interrupt                                    */
	ETH_IRQn                    = 61,     /*!< Ethernet global Interrupt                                         */
	ETH_WKUP_IRQn               = 62,     /*!< Ethernet Wakeup through EXTI line Interrupt                       */
	CAN2_TX_IRQn                = 63,     /*!< CAN2 TX Interrupt                                                 */
	CAN2_RX0_IRQn               = 64,     /*!< CAN2 RX0 Interrupt                                                */
	CAN2_RX1_IRQn               = 65,     /*!< CAN2 RX1 Interrupt                                                */
	CAN2_SCE_IRQn               = 66,     /*!< CAN2 SCE Interrupt                                                */
	OTG_FS_IRQn                 = 67,     /*!< USB OTG FS global Interrupt                                       */
	DMA2_Stream5_IRQn           = 68,     /*!< DMA2 Stream 5 global interrupt                                    */
	DMA2_Stream6_IRQn           = 69,     /*!< DMA2 Stream 6 global interrupt                                    */
	DMA2_Stream7_IRQn           = 70,     /*!< DMA2 Stream 7 global interrupt                                    */
	USART6_IRQn                 = 71,     /*!< USART6 global interrupt                                           */
	I2C3_EV_IRQn                = 72,     /*!< I2C3 event interrupt                                              */
	I2C3_ER_IRQn                = 73,     /*!< I2C3 error interrupt                                              */
	OTG_HS_EP1_OUT_IRQn         = 74,     /*!< USB OTG HS End Point 1 Out global interrupt                       */
	OTG_HS_EP1_IN_IRQn          = 75,     /*!< USB OTG HS End Point 1 In global interrupt                        */
	OTG_HS_WKUP_IRQn            = 76,     /*!< USB OTG HS Wakeup through EXTI interrupt                          */
	OTG_HS_IRQn                 = 77,     /*!< USB OTG HS global interrupt                                       */
	DCMI_IRQn                   = 78,     /*!< DCMI global interrupt                                             */
	CRYP_IRQn                   = 79,     /*!< CRYP crypto global interrupt                                      */
	HASH_RNG_IRQn               = 80,      /*!< Hash and Rng global interrupt                                     */
	FPU_IRQn                    = 81      /*!< FPU global interrupt                                              */
} IRQn_Type;

#define MAX_IRQn FPU_IRQn

#define IRQ_VEC_N_HANDLER_DECLARE(n) \
	void nvic_handler##n ();

IRQ_VEC_N_HANDLER_DECLARE(0);
IRQ_VEC_N_HANDLER_DECLARE(1);
IRQ_VEC_N_HANDLER_DECLARE(2);
IRQ_VEC_N_HANDLER_DECLARE(3);
IRQ_VEC_N_HANDLER_DECLARE(4);
IRQ_VEC_N_HANDLER_DECLARE(5);
IRQ_VEC_N_HANDLER_DECLARE(6);
IRQ_VEC_N_HANDLER_DECLARE(7);
IRQ_VEC_N_HANDLER_DECLARE(8);
IRQ_VEC_N_HANDLER_DECLARE(9);
IRQ_VEC_N_HANDLER_DECLARE(10);
IRQ_VEC_N_HANDLER_DECLARE(11);
IRQ_VEC_N_HANDLER_DECLARE(12);
IRQ_VEC_N_HANDLER_DECLARE(13);
IRQ_VEC_N_HANDLER_DECLARE(14);
IRQ_VEC_N_HANDLER_DECLARE(15);
IRQ_VEC_N_HANDLER_DECLARE(16);
IRQ_VEC_N_HANDLER_DECLARE(17);
IRQ_VEC_N_HANDLER_DECLARE(18);
IRQ_VEC_N_HANDLER_DECLARE(19);
IRQ_VEC_N_HANDLER_DECLARE(20);
IRQ_VEC_N_HANDLER_DECLARE(21);
IRQ_VEC_N_HANDLER_DECLARE(22);
IRQ_VEC_N_HANDLER_DECLARE(23);
IRQ_VEC_N_HANDLER_DECLARE(24);
IRQ_VEC_N_HANDLER_DECLARE(25);
IRQ_VEC_N_HANDLER_DECLARE(26);
IRQ_VEC_N_HANDLER_DECLARE(27);
IRQ_VEC_N_HANDLER_DECLARE(28);
IRQ_VEC_N_HANDLER_DECLARE(29);
IRQ_VEC_N_HANDLER_DECLARE(30);
IRQ_VEC_N_HANDLER_DECLARE(31);
IRQ_VEC_N_HANDLER_DECLARE(32);
IRQ_VEC_N_HANDLER_DECLARE(33);
IRQ_VEC_N_HANDLER_DECLARE(34);
IRQ_VEC_N_HANDLER_DECLARE(35);
IRQ_VEC_N_HANDLER_DECLARE(36);
IRQ_VEC_N_HANDLER_DECLARE(37);
IRQ_VEC_N_HANDLER_DECLARE(38);
IRQ_VEC_N_HANDLER_DECLARE(39);
IRQ_VEC_N_HANDLER_DECLARE(40);
IRQ_VEC_N_HANDLER_DECLARE(41);
IRQ_VEC_N_HANDLER_DECLARE(42);
IRQ_VEC_N_HANDLER_DECLARE(43);
IRQ_VEC_N_HANDLER_DECLARE(44);
IRQ_VEC_N_HANDLER_DECLARE(45);
IRQ_VEC_N_HANDLER_DECLARE(46);
IRQ_VEC_N_HANDLER_DECLARE(47);
IRQ_VEC_N_HANDLER_DECLARE(48);
IRQ_VEC_N_HANDLER_DECLARE(49);
IRQ_VEC_N_HANDLER_DECLARE(50);
IRQ_VEC_N_HANDLER_DECLARE(51);
IRQ_VEC_N_HANDLER_DECLARE(52);
IRQ_VEC_N_HANDLER_DECLARE(53);
IRQ_VEC_N_HANDLER_DECLARE(54);
IRQ_VEC_N_HANDLER_DECLARE(55);
IRQ_VEC_N_HANDLER_DECLARE(56);
IRQ_VEC_N_HANDLER_DECLARE(57);
IRQ_VEC_N_HANDLER_DECLARE(58);
IRQ_VEC_N_HANDLER_DECLARE(59);
IRQ_VEC_N_HANDLER_DECLARE(60);
IRQ_VEC_N_HANDLER_DECLARE(61);
IRQ_VEC_N_HANDLER_DECLARE(62);
IRQ_VEC_N_HANDLER_DECLARE(63);
IRQ_VEC_N_HANDLER_DECLARE(64);
IRQ_VEC_N_HANDLER_DECLARE(65);
IRQ_VEC_N_HANDLER_DECLARE(66);
IRQ_VEC_N_HANDLER_DECLARE(67);
IRQ_VEC_N_HANDLER_DECLARE(68);
IRQ_VEC_N_HANDLER_DECLARE(69);
IRQ_VEC_N_HANDLER_DECLARE(70);
IRQ_VEC_N_HANDLER_DECLARE(71);
IRQ_VEC_N_HANDLER_DECLARE(72);
IRQ_VEC_N_HANDLER_DECLARE(73);
IRQ_VEC_N_HANDLER_DECLARE(74);
IRQ_VEC_N_HANDLER_DECLARE(75);
IRQ_VEC_N_HANDLER_DECLARE(76);
IRQ_VEC_N_HANDLER_DECLARE(77);
IRQ_VEC_N_HANDLER_DECLARE(78);
IRQ_VEC_N_HANDLER_DECLARE(79);
IRQ_VEC_N_HANDLER_DECLARE(80);
IRQ_VEC_N_HANDLER_DECLARE(81);

/* NVIC table */
#define WWDG_IRQHandler			nvic_handler0
#define PVD_IRQHandler			nvic_handler1
#define TAMP_STAMP_IRQHandler		nvic_handler2
#define RTC_WKUP_IRQHandler		nvic_handler3
#define FLASH_IRQHandler		nvic_handler4
#define RCC_IRQHandler			nvic_handler5
#define EXTI0_IRQHandler		nvic_handler6
#define EXTI1_IRQHandler		nvic_handler7
#define EXTI2_IRQHandler		nvic_handler8
#define EXTI3_IRQHandler		nvic_handler9
#define EXTI4_IRQHandler		nvic_handler10
#define DMA1_Stream0_IRQHandler		nvic_handler11
#define DMA1_Stream1_IRQHandler		nvic_handler12
#define DMA1_Stream2_IRQHandler		nvic_handler13
#define DMA1_Stream3_IRQHandler		nvic_handler14
#define DMA1_Stream4_IRQHandler		nvic_handler15
#define DMA1_Stream5_IRQHandler		nvic_handler16
#define DMA1_Stream6_IRQHandler		nvic_handler17
#define ADC_IRQHandler			nvic_handler18
#define CAN1_TX_IRQHandler		nvic_handler19
#define CAN1_RX0_IRQHandler		nvic_handler20
#define CAN1_RX1_IRQHandler		nvic_handler21
#define CAN1_SCE_IRQHandler		nvic_handler22
#define EXTI9_5_IRQHandler		nvic_handler23
#define TIM1_BRK_TIM9_IRQHandler	nvic_handler24
#define TIM1_UP_TIM10_IRQHandler	nvic_handler25
#define TIM1_TRG_COM_TIM11_IRQHandler	nvic_handler26
#define TIM1_CC_IRQHandler		nvic_handler27
#define TIM2_IRQHandler			nvic_handler28
#define TIM3_IRQHandler			nvic_handler29
#define TIM4_IRQHandler			nvic_handler30
#define I2C1_EV_IRQHandler		nvic_handler31
#define I2C1_ER_IRQHandler		nvic_handler32
#define I2C2_EV_IRQHandler		nvic_handler33
#define I2C2_ER_IRQHandler		nvic_handler34
#define SPI1_IRQHandler			nvic_handler35
#define SPI2_IRQHandler			nvic_handler36
#define USART1_IRQHandler		nvic_handler37
#define USART2_IRQHandler		nvic_handler38
#define USART3_IRQHandler		nvic_handler39
#define EXTI15_10_IRQHandler		nvic_handler40
#define RTC_Alarm_IRQHandler		nvic_handler41
#define OTG_FS_WKUP_IRQHandler		nvic_handler42
#define TIM8_BRK_TIM12_IRQHandler	nvic_handler43
#define TIM8_UP_TIM13_IRQHandler	nvic_handler44
#define TIM8_TRG_COM_TIM14_IRQHandler	nvic_handler45
#define TIM8_CC_IRQHandler		nvic_handler46
#define DMA1_Stream7_IRQHandler		nvic_handler47
#define FSMC_IRQHandler			nvic_handler48
#define SDIO_IRQHandler			nvic_handler49
#define TIM5_IRQHandler			nvic_handler50
#define SPI3_IRQHandler			nvic_handler51
#define UART4_IRQHandler		nvic_handler52
#define UART5_IRQHandler		nvic_handler53
#define TIM6_DAC_IRQHandler		nvic_handler54
#define TIM7_IRQHandler			nvic_handler55
#define DMA2_Stream0_IRQHandler		nvic_handler56
#define DMA2_Stream1_IRQHandler		nvic_handler57
#define DMA2_Stream2_IRQHandler		nvic_handler58
#define DMA2_Stream3_IRQHandler		nvic_handler59
#define DMA2_Stream4_IRQHandler		nvic_handler60
#define ETH_IRQHandler			nvic_handler61
#define ETH_WKUP_IRQHandler		nvic_handler62
#define CAN2_TX_IRQHandler		nvic_handler63
#define CAN2_RX0_IRQHandler		nvic_handler64
#define CAN2_RX1_IRQHandler		nvic_handler65
#define CAN2_SCE_IRQHandler		nvic_handler66
#define OTG_FS_IRQHandler		nvic_handler67
#define DMA2_Stream5_IRQHandler		nvic_handler68
#define DMA2_Stream6_IRQHandler		nvic_handler69
#define DMA2_Stream7_IRQHandler		nvic_handler70
#define USART6_IRQHandler		nvic_handler71
#define I2C3_EV_IRQHandler		nvic_handler72
#define I2C3_ER_IRQHandler		nvic_handler73
#define OTG_HS_EP1_OUT_IRQHandler	nvic_handler74
#define OTG_HS_EP1_IN_IRQHandler	nvic_handler75
#define OTG_HS_WKUP_IRQHandler		nvic_handler76
#define OTG_HS_IRQHandler		nvic_handler77
#define DCMI_IRQHandler			nvic_handler78
#define CRYP_IRQHandler			nvic_handler79
#define HASH_RNG_IRQHandler		nvic_handler80
#define FPU_IRQHandler			nvic_handler81

/* platform-specific NVIC table: BEGIN */
#define PLAT_NVIC_ENTRIES \
	WWDG_IRQHandler,		/* 0 Window WatchDog */ \
	PVD_IRQHandler,			/* 1 PVD through EXTI Line detection */ \
	TAMP_STAMP_IRQHandler,		/* 2 Tamper and TimeStamps through the EXTI line */ \
	RTC_WKUP_IRQHandler,		/* 3 RTC Wakeup through the EXTI line */ \
	FLASH_IRQHandler,		/* 4 FLASH */ \
	RCC_IRQHandler,			/* 5 RCC */ \
	EXTI0_IRQHandler,		/* 6 EXTI Line0 */ \
	EXTI1_IRQHandler,		/* 7 EXTI Line1 */ \
	EXTI2_IRQHandler,		/* 8 EXTI Line2 */ \
	EXTI3_IRQHandler,		/* 9 EXTI Line3 */ \
	EXTI4_IRQHandler,		/* 10 EXTI Line4 */ \
	DMA1_Stream0_IRQHandler,	/* 11 DMA1 Stream 0 */ \
	DMA1_Stream1_IRQHandler,	/* 12 DMA1 Stream 1 */ \
	DMA1_Stream2_IRQHandler,	/* 13 DMA1 Stream 2 */ \
	DMA1_Stream3_IRQHandler,	/* 14 DMA1 Stream 3 */ \
	DMA1_Stream4_IRQHandler,	/* 15 DMA1 Stream 4 */ \
	DMA1_Stream5_IRQHandler,	/* 16 DMA1 Stream 5 */ \
	DMA1_Stream6_IRQHandler,	/* 17 DMA1 Stream 6 */ \
	ADC_IRQHandler,			/* 18 ADC1, ADC2 and ADC3s */ \
	CAN1_TX_IRQHandler,		/* 19 CAN1 TX */ \
	CAN1_RX0_IRQHandler,		/* 20 CAN1 RX0 */ \
	CAN1_RX1_IRQHandler,		/* 21 CAN1 RX1 */ \
	CAN1_SCE_IRQHandler,		/* 22 CAN1 SCE */ \
	EXTI9_5_IRQHandler,		/* 23 External Line[9:5]s */ \
	TIM1_BRK_TIM9_IRQHandler,	/* 24 TIM1 Break and TIM9 */ \
	TIM1_UP_TIM10_IRQHandler,	/* 25 TIM1 Update and TIM10 */ \
	TIM1_TRG_COM_TIM11_IRQHandler,	/* 26 TIM1 Trigger and Commutation and TIM11 */ \
	TIM1_CC_IRQHandler,		/* 27 TIM1 Capture Compare */ \
	TIM2_IRQHandler,		/* 28 TIM2 */ \
	TIM3_IRQHandler,		/* 29 TIM3 */ \
	TIM4_IRQHandler,		/* 30 TIM4 */ \
	I2C1_EV_IRQHandler,		/* 31 I2C1 Event */ \
	I2C1_ER_IRQHandler,		/* 32 I2C1 Error */ \
	I2C2_EV_IRQHandler,		/* 33 I2C2 Event */ \
	I2C2_ER_IRQHandler,		/* 34 I2C2 Error */ \
	SPI1_IRQHandler,		/* 35 SPI1 */ \
	SPI2_IRQHandler,		/* 36 SPI2 */ \
	USART1_IRQHandler,		/* 37 USART1 */ \
	USART2_IRQHandler,		/* 38 USART2 */ \
	USART3_IRQHandler,		/* 39 USART3 */ \
	EXTI15_10_IRQHandler,		/* 40 External Line[15:10]s */ \
	RTC_Alarm_IRQHandler,		/* 41 RTC Alarm (A and B) through EXTI Line */ \
	OTG_FS_WKUP_IRQHandler,		/* 42 USB OTG FS Wakeup through EXTI line */ \
	TIM8_BRK_TIM12_IRQHandler,	/* 43 TIM8 Break and TIM12 */ \
	TIM8_UP_TIM13_IRQHandler,	/* 44 TIM8 Update and TIM13 */ \
	TIM8_TRG_COM_TIM14_IRQHandler,	/* 45 TIM8 Trigger and Commutation and TIM14 */ \
	TIM8_CC_IRQHandler,		/* 46 TIM8 Capture Compare */ \
	DMA1_Stream7_IRQHandler,	/* 47 DMA1 Stream7 */ \
	FSMC_IRQHandler,		/* 48 FSMC */ \
	SDIO_IRQHandler,		/* 49 SDIO */ \
	TIM5_IRQHandler,		/* 50 TIM5 */ \
	SPI3_IRQHandler,		/* 51 SPI3 */ \
	UART4_IRQHandler,		/* 52 UART4 */ \
	UART5_IRQHandler,		/* 53 UART5 */ \
	TIM6_DAC_IRQHandler,		/* 54 TIM6 and DAC1&2 underrun errors */ \
	TIM7_IRQHandler,		/* 55 TIM7 */ \
	DMA2_Stream0_IRQHandler,	/* 56 DMA2 Stream 0 */ \
	DMA2_Stream1_IRQHandler,	/* 57 DMA2 Stream 1 */ \
	DMA2_Stream2_IRQHandler,	/* 58 DMA2 Stream 2 */ \
	DMA2_Stream3_IRQHandler,	/* 59 DMA2 Stream 3 */ \
	DMA2_Stream4_IRQHandler,	/* 60 DMA2 Stream 4 */ \
	ETH_IRQHandler,			/* 61 Ethernet */ \
	ETH_WKUP_IRQHandler,		/* 62 Ethernet Wakeup through EXTI line */ \
	CAN2_TX_IRQHandler,		/* 63 CAN2 TX */ \
	CAN2_RX0_IRQHandler,		/* 64 CAN2 RX0 */ \
	CAN2_RX1_IRQHandler,		/* 65 CAN2 RX1 */ \
	CAN2_SCE_IRQHandler,		/* 66 CAN2 SCE */ \
	OTG_FS_IRQHandler,		/* 67 USB OTG FS */ \
	DMA2_Stream5_IRQHandler,	/* 68 DMA2 Stream 5 */ \
	DMA2_Stream6_IRQHandler,	/* 69 DMA2 Stream 6 */ \
	DMA2_Stream7_IRQHandler,	/* 70 DMA2 Stream 7 */ \
	USART6_IRQHandler,		/* 71 USART6 */ \
	I2C3_EV_IRQHandler,		/* 72 I2C3 event */ \
	I2C3_ER_IRQHandler,		/* 73 I2C3 error */ \
	OTG_HS_EP1_OUT_IRQHandler,	/* 74 USB OTG HS End Point 1 Out */ \
	OTG_HS_EP1_IN_IRQHandler,	/* 75 USB OTG HS End Point 1 In */ \
	OTG_HS_WKUP_IRQHandler,		/* 76 USB OTG HS Wakeup through EXTI */ \
	OTG_HS_IRQHandler,		/* 77 USB OTG HS */ \
	DCMI_IRQHandler,		/* 78 DCMI */ \
	CRYP_IRQHandler,		/* 79 CRYP crypto */ \
	HASH_RNG_IRQHandler,		/* 80 Hash and Rng */ \
	FPU_IRQHandler,			/* FPU */
/* platform-specific NVIC table: END */

typedef struct {
	volatile uint32_t ISER[8];	/*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register */
	uint32_t RESERVED0[24];
	volatile uint32_t ICER[8];	/*!< Offset: 0x080 (R/W)  Interrupt Clear Enable Register */
	uint32_t RSERVED1[24];
	volatile uint32_t ISPR[8];	/*!< Offset: 0x100 (R/W)  Interrupt Set Pending Register */
	uint32_t RESERVED2[24];
	volatile uint32_t ICPR[8];	/*!< Offset: 0x180 (R/W)  Interrupt Clear Pending Register */
	uint32_t RESERVED3[24];
	volatile uint32_t IABR[8];	/*!< Offset: 0x200 (R/W)  Interrupt Active bit Register */
	uint32_t RESERVED4[56];
	volatile uint8_t  IP[240];	/*!< Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide) */
	uint32_t RESERVED5[644];
	volatile uint32_t STIR;		/*!< Offset: 0xE00 ( /W)  Software Trigger Interrupt Register */
} NVIC_Type;

typedef void (*PFN_ISR)(uint32_t input);

#define NVIC_PriorityGroup_0         ((uint32_t)0x700) /* 0 bits for pre-emption priority
                                                            4 bits for subpriority */
#define NVIC_PriorityGroup_1         ((uint32_t)0x600) /* 1 bits for pre-emption priority
                                                            3 bits for subpriority */
#define NVIC_PriorityGroup_2         ((uint32_t)0x500) /* 2 bits for pre-emption priority
                                                            2 bits for subpriority */
#define NVIC_PriorityGroup_3         ((uint32_t)0x400) /* 3 bits for pre-emption priority
                                                            1 bits for subpriority */
#define NVIC_PriorityGroup_4         ((uint32_t)0x300) /* 4 bits for pre-emption priority
                                                            0 bits for subpriority */
#define NVIC \
	((NVIC_Type *) NVIC_BASE)


void NVIC_SetPriority(IRQn_Type IRQn, uint8_t group_priority, uint8_t sub_priority);
void NVIC_PriorityGroupConfig(uint32_t NVIC_PriorityGroup);
void NVIC_intAttached(IRQn_Type IRQn, PFN_ISR handler, uint32_t handler_input);

inline static void  NVIC_EnableIRQ(IRQn_Type IRQn)
{
	NVIC->ISER[((uint32_t) IRQn >> 0x5)] = (1 << ((uint32_t)(IRQn) & 0x1f));
}

inline static void NVIC_DisableIRQ(IRQn_Type IRQn)
{
	NVIC->ICER[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F));
}

inline static uint32_t NVIC_GetPendingIRQ(IRQn_Type IRQn)
{
	return (uint32_t) ((NVIC->ISPR[(uint32_t)(IRQn) >> 5] &
	                   (1 << ((uint32_t)(IRQn) & 0x1F))) ? 1 : 0);
}

inline static void NVIC_SetPendingIRQ(IRQn_Type IRQn)
{
	NVIC->ISPR[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F));
}

inline static void NVIC_ClearPendingIRQ(IRQn_Type IRQn)
{
	NVIC->ICPR[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F));
}

inline static uint32_t NVIC_GetActive(IRQn_Type IRQn)
{
	return (uint32_t)((NVIC->IABR[(uint32_t)(IRQn) >> 5] &
	                  (1 << ((uint32_t)(IRQn) & 0x1F))) ? 1 : 0);
}

#endif /* __PLATFORM_STM32F4_NVIC_H__ */
