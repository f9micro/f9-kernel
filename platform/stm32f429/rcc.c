/* Copyright (c) 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define RCC_OFFSET              (RCC_BASE - PERIPH_BASE)
#define CR_OFFSET               (RCC_OFFSET + 0x00)
/* Alias word address of PLLSAION bit */
#define PLLSAION_BitNumber      0x1C
#define CR_PLLSAION_BB          (PERIPH_BB_BASE + (CR_OFFSET * 32) + (PLLSAION_BitNumber * 4))

void __USER_TEXT RCC_AHB3PeriphClockCmd(uint32_t rcc_AHB3, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0)
		*RCC_AHB3ENR |= rcc_AHB3;
	else
		*RCC_AHB3ENR &= ~rcc_AHB3;
}

void __USER_TEXT RCC_PLLSAIConfig(uint32_t pllsain, uint32_t pllsaiq, uint32_t pllsair)
{
	/* TODO: assertion */

	*RCC_PLLSAICFGR = (pllsain << 6) | (pllsaiq << 24) | (pllsair << 28);
}

void __USER_TEXT RCC_LTDCCLKDivConfig(uint32_t div)
{
	uint32_t tmpreg = 0;

	/* TODO: assertion */

	tmpreg = *RCC_DCKCFGR;

	tmpreg &= ~~RCC_DCKCFGR_PLLSAIDIVR;

	tmpreg |= div;

	*RCC_DCKCFGR = tmpreg;
}

void __USER_TEXT RCC_PLLSAICmd(uint32_t enable)
{
	/* TODO: assertion */

	*(volatile uint32_t *)CR_PLLSAION_BB = enable;
}

