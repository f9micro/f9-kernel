#ifndef PLATFORM_STM32F429_FMC_H__
#define PLATFORM_STM32F429_FMC_H__

#include <platform/link.h>
#include <platform/stm32f429/registers.h>

struct fmc_sdram_cfg {
	uint32_t bank;
	uint32_t column_bits_number;
	uint32_t row_bits_number;
	uint32_t sdmemory_data_width;
	uint32_t internal_bank_number;
	uint32_t cas_latency;
	uint32_t write_protection;
	uint32_t sd_clock_period;
	uint32_t readburst;
	uint32_t readpipe_delay;
	struct fmc_sdram_timing_cfg* timing;
};

struct fmc_sdram_timing_cfg {
	uint32_t lta_delay;
	uint32_t esr_delay;
	uint32_t sr_time;
	uint32_t rc_delay;
	uint32_t wr_time;
	uint32_t rp_delay;
	uint32_t rcd_delay;
};

struct fmc_sdram_cmd {
	uint32_t mode;
	uint32_t target;
	uint32_t auto_refresh_number;
	uint32_t mode_register_definition;
};

#define FMC_Bank1_SDRAM                    ((uint32_t)0x00000000)
#define FMC_Bank2_SDRAM                    ((uint32_t)0x00000001)

#define FMC_ColumnBits_Number_8b           ((uint32_t)0x00000000)
#define FMC_ColumnBits_Number_9b           ((uint32_t)0x00000001)
#define FMC_ColumnBits_Number_10b          ((uint32_t)0x00000002)
#define FMC_ColumnBits_Number_11b          ((uint32_t)0x00000003)

#define FMC_RowBits_Number_11b             ((uint32_t)0x00000000)
#define FMC_RowBits_Number_12b             ((uint32_t)0x00000004)
#define FMC_RowBits_Number_13b             ((uint32_t)0x00000008)

#define FMC_SDMemory_Width_8b                ((uint32_t)0x00000000)
#define FMC_SDMemory_Width_16b               ((uint32_t)0x00000010)
#define FMC_SDMemory_Width_32b               ((uint32_t)0x00000020)

#define FMC_InternalBank_Number_2          ((uint32_t)0x00000000)
#define FMC_InternalBank_Number_4          ((uint32_t)0x00000040)

#define FMC_CAS_Latency_1                  ((uint32_t)0x00000080)
#define FMC_CAS_Latency_2                  ((uint32_t)0x00000100)
#define FMC_CAS_Latency_3                  ((uint32_t)0x00000180)

#define FMC_Write_Protection_Disable       ((uint32_t)0x00000000)
#define FMC_Write_Protection_Enable        ((uint32_t)0x00000200)

#define FMC_SDClock_Disable                ((uint32_t)0x00000000)
#define FMC_SDClock_Period_2               ((uint32_t)0x00000800)
#define FMC_SDClock_Period_3               ((uint32_t)0x00000C00)

#define FMC_FLAG_REFRESH                   ((uint32_t)0x00000001)
#define FMC_FLAG_BUSY                      ((uint32_t)0x00000020)

#define FMC_Read_Burst_Disable             ((uint32_t)0x00000000)
#define FMC_Read_Burst_Enable              ((uint32_t)0x00001000)

#define FMC_ReadPipe_Delay_0               ((uint32_t)0x00000000)
#define FMC_ReadPipe_Delay_1               ((uint32_t)0x00002000)
#define FMC_ReadPipe_Delay_2               ((uint32_t)0x00004000)

#define FMC_Command_Mode_normal            ((uint32_t)0x00000000)
#define FMC_Command_Mode_CLK_Enabled       ((uint32_t)0x00000001)
#define FMC_Command_Mode_PALL              ((uint32_t)0x00000002)
#define FMC_Command_Mode_AutoRefresh       ((uint32_t)0x00000003)
#define FMC_Command_Mode_LoadMode          ((uint32_t)0x00000004)
#define FMC_Command_Mode_Selfrefresh       ((uint32_t)0x00000005)
#define FMC_Command_Mode_PowerDown         ((uint32_t)0x00000006)

#define FMC_Command_Target_bank2           ((uint32_t)0x00000008)
#define FMC_Command_Target_bank1           ((uint32_t)0x00000010)
#define FMC_Command_Target_bank1_2         ((uint32_t)0x00000018)

void fmc_sdram_config(struct fmc_sdram_cfg *cfg);
uint8_t fmc_get_flag(uint32_t bank, uint32_t flag);
void fmc_sdram_cmd_init(struct fmc_sdram_cmd *cmd);
void fmc_set_refresh_count(uint32_t count);

#endif
