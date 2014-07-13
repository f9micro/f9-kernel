#ifndef PLATFORM_STM32F429_SDRAM_H__
#define PLATFORM_STM32F429_SDRAM_H__

#include <platform/link.h>
#include <platform/stm32f429/registers.h>

#define SDRAM_MEMORY_WIDTH    FMC_SDMemory_Width_16b
#define SDRAM_CAS_LATENCY    FMC_CAS_Latency_3
#define SDCLOCK_PERIOD    FMC_SDClock_Period_2
#define SDRAM_READBURST    FMC_Read_Burst_Disable

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

void sdram_init(void);

#endif
