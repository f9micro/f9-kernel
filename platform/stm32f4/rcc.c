/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/stm32f4/registers.h>
#include <platform/cortex_m.h>
#include <error.h>

#define HSE_STARTUP_TIMEOUT \
	(uint16_t) (0x0500)	/*!< Time out for HSE start up */
#define PLL_M	8	/*!< PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_M) *
			     PLL_N */
#define PLL_N	336
#define PLL_P	2	/*!< SYSCLK = PLL_VCO / PLL_P */
#define PLL_Q	7	/*!< USB OTG FS, SDIO and RNG Clock =
			     PLL_VCO / PLLQ */

void sys_clock_init(void)
{
	volatile uint32_t startup_count, HSE_status;

	/* Enable the FPU */
	*SCB_CPACR |= (0xf << 20);
	/* Enable floating point state preservation */
	*FPU_CCR |= FPU_CCR_ASPEN;

	/* Reset clock registers */
	/* Set HSION bit */
	*RCC_CR |= (uint32_t) 0x00000001;

	/* Reset CFGR register */
	*RCC_CFGR = 0x00000000;

	/* Reset HSEON, CSSON and PLLON bits */
	*RCC_CR &= (uint32_t) 0xFEF6FFFF;

	/* Reset PLLCFGR register */
	*RCC_PLLCFGR = 0x24003010;

	/* Reset HSEBYP bit */
	*RCC_CR &= (uint32_t) 0xFFFBFFFF;

	/* Disable all interrupts */
	*RCC_CIR = 0x00000000;

	/* Set up the clock */
	startup_count = 0;
	HSE_status = 0;

	/* Enable HSE */
	*RCC_CR |= RCC_CR_HSEON;

	/* Wait till HSE is ready and if Time out is reached exit */
	do {
		HSE_status = *RCC_CR & RCC_CR_HSERDY;
		startup_count++;
	} while ((HSE_status == 0) && (startup_count != HSE_STARTUP_TIMEOUT));

	if ((*RCC_CR & RCC_CR_HSERDY) != 0) {
		HSE_status = (uint32_t) 0x01;
	} else {
		HSE_status = (uint32_t) 0x00;
	}

	if (HSE_status == (uint32_t) 0x01) {
		/* Enable high performance mode:
		   system frequency is up to 168 MHz */
		*RCC_APB1ENR |= RCC_APB1ENR_PWREN;
		*PWR_CR |= PWR_CR_VOS;

		/* HCLK = SYSCLK / 1 */
		*RCC_CFGR |= RCC_CFGR_HPRE_DIV1;

		/* PCLK2 = HCLK / 2 */
		*RCC_CFGR |= RCC_CFGR_PPRE2_DIV2;

		/* PCLK1 = HCLK / 4 */
		*RCC_CFGR |= RCC_CFGR_PPRE1_DIV4;

		/* Configure the main PLL */
		/* PLL Options - See RM0090 Reference Manual pg. 95 */
		*RCC_PLLCFGR = PLL_M | (PLL_N << 6) |
		               (((PLL_P >> 1) - 1) << 16) |
		               (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);

		/* Enable the main PLL */

		*RCC_CR |= RCC_CR_PLLON;

		/* Wait till the main PLL is ready */
		while ((*RCC_CR & RCC_CR_PLLRDY) == 0)
			/* wait */ ;

		/* Configure Flash prefetch, Instruction cache,
		 * Data cache and wait state
		 */
#if 0
		*FLASH_ACR = FLASH_ACR_ICEN | FLASH_ACR_DCEN |
		             FLASH_ACR_LATENCY_5WS;
#endif
		*FLASH_ACR = FLASH_ACR_LATENCY(5);

		/* Select the main PLL as system clock source */
		*RCC_CFGR &= (uint32_t)((uint32_t) ~(RCC_CFGR_SW_M));
		*RCC_CFGR |= RCC_CFGR_SW_PLL;

		/* Wait till the main PLL is used as system clock source */
		while ((*RCC_CFGR & (uint32_t) RCC_CFGR_SWS_M) !=
		       RCC_CFGR_SWS_PLL)
			/* wait */ ;
	} else {
		/* If HSE fails to start-up, the application will have
		 * wrong clock configuration.
		 */
		panic("Time out for waiting HSE Ready");
	}

	/* Enable the CCM RAM clock */
	*RCC_AHB1ENR |= (1 << 20);

	/* Enable Bus and Usage Faults */
	*SCB_SHCSR |= SCB_SHCSR_BUSFAULTENA;
	*SCB_SHCSR |= SCB_SHCSR_USEFAULTENA;
}
