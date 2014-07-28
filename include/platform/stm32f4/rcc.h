#ifndef PLATFORM_STM32F4_RCC_H__
#define PLATFORM_STM32F4_RCC_H__

#include <platform/stm32f4/registers.h>
struct rcc_clocks {
	uint32_t sysclk_freq;
	uint32_t hclk_freq;
	uint32_t pclk1_freq;
	uint32_t pclk2_freq;
};

/* RCC_FLAG definition */
#define RCC_FLAG_HSIRDY                  ((uint8_t)0x21)
#define RCC_FLAG_HSERDY                  ((uint8_t)0x31)
#define RCC_FLAG_PLLRDY                  ((uint8_t)0x39)
#define RCC_FLAG_PLLI2SRDY               ((uint8_t)0x3B)
#define RCC_FLAG_PLLSAIRDY               ((uint8_t)0x3D)
#define RCC_FLAG_LSERDY                  ((uint8_t)0x41)
#define RCC_FLAG_LSIRDY                  ((uint8_t)0x61)
#define RCC_FLAG_BORRST                  ((uint8_t)0x79)
#define RCC_FLAG_PINRST                  ((uint8_t)0x7A)
#define RCC_FLAG_PORRST                  ((uint8_t)0x7B)
#define RCC_FLAG_SFTRST                  ((uint8_t)0x7C)
#define RCC_FLAG_IWDGRST                 ((uint8_t)0x7D)
#define RCC_FLAG_WWDGRST                 ((uint8_t)0x7E)
#define RCC_FLAG_LPWRRST                 ((uint8_t)0x7F)

void sys_clock_init(void);
void RCC_AHB1PeriphClockCmd(uint32_t rcc_AHB1, uint8_t enable);
void RCC_AHB1PeriphResetCmd(uint32_t rcc_AHB1, uint8_t enable);
void RCC_APB1PeriphClockCmd(uint32_t rcc_APB1, uint8_t enable);
void RCC_APB1PeriphResetCmd(uint32_t rcc_APB1, uint8_t enable);
void RCC_APB2PeriphClockCmd(uint32_t rcc_APB2, uint8_t enable);
void RCC_APB2PeriphResetCmd(uint32_t rcc_APB2, uint8_t enable);
uint8_t RCC_GetFlagStatus(uint8_t flag);
void RCC_GetClocksFreq(struct rcc_clocks* clock);

#endif
