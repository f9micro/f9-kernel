/* Copyright (c) 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include INC_PLAT(rcc.h)
#include <platform/cortex_m.h>
#include <error.h>
#include INC_PLAT(rcc.c)

#define HSE_STARTUP_TIMEOUT \
	(uint16_t) (0x0500)	/*!< Time out for HSE start up */

#if defined(STM32F4X)
	#define PLL_M	8	/*!< PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_M) * PLL_N */
	#define PLL_N	336
	#define PLL_P	2	/*!< SYSCLK = PLL_VCO / PLL_P */
	#define PLL_Q	7	/*!< USB OTG FS, SDIO and RNG Clock = PLL_VCO / PLLQ */

static __USER_DATA uint8_t APBAHBPrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};
#elif defined(STM32F1X)
	#define PLL_MUL	6
#endif

/* RCC Flag Mask */
#define FLAG_MASK                 ((uint8_t)0x1F)


void sys_clock_init(void)
{
	volatile uint32_t startup_count, HSE_status;

#if defined(STM32F4X) && defined(CONFIG_FPU)
	/* Enable the FPU */
	*SCB_CPACR |= (0xf << 20);
	/* Enable floating point state preservation */
	*FPU_CCR |= FPU_CCR_ASPEN;
#endif
	/* Reset clock registers */
	/* Set HSION bit */
	*RCC_CR |= (uint32_t) 0x00000001;

#if defined(STM32F1X)
	*RCC_CFGR &= (uint32_t)0xF0FF0000;
#else
	/* Reset CFGR register */
	*RCC_CFGR = 0x00000000;
#endif
	/* Reset HSEON, CSSON and PLLON bits */
	*RCC_CR &= (uint32_t) 0xFEF6FFFF;

#if defined(STM32F4X)
	/* Reset PLLCFGR register */
	*RCC_PLLCFGR = 0x24003010;
#elif defined(STM32F1X)
	*RCC_CFGR &= (uint32_t)0xFF80FFFF;
#endif
	/* Reset HSEBYP bit */
	*RCC_CR &= (uint32_t) 0xFFFBFFFF;

	/* Disable all interrupts */
#if defined(STM32F4X)
	*RCC_CIR = 0x00000000;
#elif defined(STM32F1X)
	*RCC_CIR = 0x009F0000;
#endif

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
#if defined(STM32F4X)
		*PWR_CR |= PWR_CR_VOS;
#endif
		/* HCLK = SYSCLK / 1 */
		*RCC_CFGR |= RCC_CFGR_HPRE_DIV1;

		/* PCLK2 = HCLK / 2 */
		*RCC_CFGR |= RCC_CFGR_PPRE2_DIV2;

		/* PCLK1 = HCLK / 4 */
		*RCC_CFGR |= RCC_CFGR_PPRE1_DIV4;

		/* Configure the main PLL */
#if defined(STM32F4X)
		/* PLL Options - See RM0090 Reference Manual pg. 95 */
		*RCC_PLLCFGR = PLL_M | (PLL_N << 6) |
		               (((PLL_P >> 1) - 1) << 16) |
		               (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);
#elif defined(STM32F1X)
		*RCC_CFGR |= PLL_MUL << 18;
#endif
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
#if defined(STM32F4X)
		*FLASH_ACR = FLASH_ACR_LATENCY(5);
#elif defined(STM32F1X)
		*FLASH_ACR = FLASH_ACR_LATENCY(1);
#endif
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
#if defined(STM32F4X)
	/* Enable the CCM RAM clock */
	*RCC_AHB1ENR |= (1 << 20);
#endif
	/* Enable Bus and Usage Faults */
	*SCB_SHCSR |= SCB_SHCSR_BUSFAULTENA;
	*SCB_SHCSR |= SCB_SHCSR_USEFAULTENA;
}

#if defined(STM32F4X)
void __USER_TEXT RCC_AHB1PeriphClockCmd(uint32_t rcc_AHB1, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0)
		*RCC_AHB1ENR |= rcc_AHB1;
	else
		*RCC_AHB1ENR &= ~rcc_AHB1;

#elif defined(STM32F1X)
void __USER_TEXT RCC_AHBPeriphClockCmd(uint32_t rcc_AHB, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0)
		*RCC_AHBENR |= rcc_AHB;
	else
		*RCC_AHBENR &= ~rcc_AHB;

#endif
}

#if defined(STM32F4X)
void __USER_TEXT RCC_AHB1PeriphResetCmd(uint32_t rcc_AHB1, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0)
		*RCC_AHB1RSTR |= rcc_AHB1;
	else
		*RCC_AHB1RSTR &= ~rcc_AHB1;

#elif defined(STM32F1X)
void __USER_TEXT RCC_AHBPeriphResetCmd(uint32_t rcc_AHB, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0)
		*RCC_AHBRSTR |= rcc_AHB;
	else
		*RCC_AHBRSTR &= ~rcc_AHB;

#endif
}

void __USER_TEXT RCC_APB1PeriphClockCmd(uint32_t rcc_APB1, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0)
		*RCC_APB1ENR |= rcc_APB1;
	else
		*RCC_APB1ENR &= ~rcc_APB1;
}

void __USER_TEXT RCC_APB1PeriphResetCmd(uint32_t rcc_APB1, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0)
		*RCC_APB1RSTR |= rcc_APB1;
	else
		*RCC_APB1RSTR &= ~rcc_APB1;
}

void __USER_TEXT RCC_APB2PeriphClockCmd(uint32_t rcc_APB2, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0)
		*RCC_APB2ENR |= rcc_APB2;
	else
		*RCC_APB2ENR &= ~rcc_APB2;
}

void __USER_TEXT RCC_APB2PeriphResetCmd(uint32_t rcc_APB2, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0)
		*RCC_APB2RSTR |= rcc_APB2;
	else
		*RCC_APB2RSTR &= ~rcc_APB2;
}

uint8_t __USER_TEXT RCC_GetFlagStatus(uint8_t flag)
{
	uint32_t tmp = 0;
	uint32_t statusreg = 0;
	uint8_t bitstatus = 0;

	/* TODO: assertion */

	tmp = flag >> 5;
	if (tmp == 1)
		statusreg = *RCC_CR;
	else if (tmp == 2)
		statusreg = *RCC_BDCR;
	else
		statusreg = *RCC_CSR;

	tmp = flag & FLAG_MASK;
	if ((statusreg & ((uint32_t)1 << tmp)) != (uint32_t)0)
		bitstatus = 1;
	else
		bitstatus = 0;

	return bitstatus;
}

void __USER_TEXT RCC_GetClocksFreq(struct rcc_clocks *clock)
{
#if defined(STM32F4X)
	uint32_t tmp = 0, presc = 0, pllvco = 0, pllp = 2, pllsource = 0, pllm = 2;

	tmp = *RCC_CFGR & RCC_CFGR_SWS_M;

	switch (tmp) {
	case 0x00:
		clock->sysclk_freq = HSI_VALUE;
		break;
	case 0x04:
		clock->sysclk_freq = HSE_VALUE;
		break;
	case 0x08:
		pllsource = (*RCC_PLLCFGR & RCC_PLLCFGR_PLLSRC_HSE) >> 22;
		pllm = *RCC_PLLCFGR & RCC_PLLCFGR_PLLM;

		if (pllsource != 0)
			pllvco = (HSE_VALUE / pllm) * ((*RCC_PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
		else
			pllvco = (HSI_VALUE / pllm) * ((*RCC_PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);

		pllp = (((*RCC_PLLCFGR & RCC_PLLCFGR_PLLP) >> 16) + 1) * 2;
		clock->sysclk_freq = pllvco / pllp;
		break;
	default:
		clock->sysclk_freq = HSI_VALUE;
		break;
	}

	tmp = *RCC_CFGR & RCC_CFGR_HPRE_M;
	tmp = tmp >> 4;
	presc = APBAHBPrescTable[tmp];

	clock->hclk_freq = clock->sysclk_freq >> presc;

	tmp = *RCC_CFGR & RCC_CFGR_PPRE1_M;
	tmp = tmp >> 10;
	presc = APBAHBPrescTable[tmp];

	clock->pclk1_freq = clock->hclk_freq >> presc;

	tmp = *RCC_CFGR & RCC_CFGR_PPRE2_M;
	tmp = tmp >> 13;
	presc = APBAHBPrescTable[tmp];

	clock->pclk2_freq = clock->hclk_freq >> presc;
#endif
}
