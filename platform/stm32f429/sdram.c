#include <platform/stm32f429/sdram.h>
#include <platform/stm32f429/rcc.h>
#include <platform/stm32f429/gpio.h>
#include <platform/stm32f429/fmc.h>

void static sdram_gpio_init(void);
void static sdram_init_seq(void);

void __USER_TEXT sdram_init(void)
{
	struct fmc_sdram_timing_cfg fs_timing_init;
	struct fmc_sdram_cfg fs_init;

	sdram_gpio_init();

	RCC_AHB3PeriphClockCmd(RCC_AHB3ENR_FMCEN, 1);

	fs_timing_init.lta_delay = 2; /* 2 clock cycles */
	fs_timing_init.esr_delay = 7; /* 70ns */
	fs_timing_init.sr_time = 4; /* 42ns */
	fs_timing_init.rc_delay = 7; /* 70 */
	fs_timing_init.wr_time = 2; /* 1+ 7ns */
	fs_timing_init.rp_delay = 2; /* 20ns */
	fs_timing_init.rcd_delay = 2; /* 20ns */

	fs_init.bank = FMC_Bank2_SDRAM;
	fs_init.column_bits_number = FMC_ColumnBits_Number_8b;
	fs_init.row_bits_number = FMC_RowBits_Number_12b;
	fs_init.sdmemory_data_width = SDRAM_MEMORY_WIDTH;
	fs_init.internal_bank_number = FMC_InternalBank_Number_4;
	fs_init.cas_latency = SDRAM_CAS_LATENCY;
	fs_init.write_protection = FMC_Write_Protection_Disable;
	fs_init.sd_clock_period = SDCLOCK_PERIOD;
	fs_init.readburst = SDRAM_READBURST;
	fs_init.readpipe_delay = FMC_ReadPipe_Delay_1;
	fs_init.timing = &fs_timing_init;

	fmc_sdram_config(&fs_init);

	sdram_init_seq();
}

void static __USER_TEXT sdram_gpio_init(void)
{
	struct gpio_cfg gpio_init;

	RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN | \
	                       RCC_AHB1ENR_GPIOEEN | RCC_AHB1ENR_GPIOFEN | RCC_AHB1ENR_GPIOGEN, 1);

	/* GPIOB */
	gpio_init.port = GPIOB;
	gpio_init.type = GPIO_MODER_ALT;
	gpio_init.speed = GPIO_OSPEEDR_50M;
	gpio_init.pupd = GPIO_PUPDR_NONE;
	gpio_init.o_type = GPIO_OTYPER_PP;
	gpio_init.func = af_fmc;
	gpio_init.pin = 5;
	gpio_config(&gpio_init);
	gpio_init.pin = 6;
	gpio_config(&gpio_init);

	/* GPIOC */
	gpio_init.port = GPIOC;
	gpio_init.pin = 0;
	gpio_config(&gpio_init);

	/* GPIOD */
	gpio_init.port = GPIOD;
	gpio_init.pin = 0;
	gpio_config(&gpio_init);
	gpio_init.pin = 1;
	gpio_config(&gpio_init);
	gpio_init.pin = 8;
	gpio_config(&gpio_init);
	gpio_init.pin = 9;
	gpio_config(&gpio_init);
	gpio_init.pin = 10;
	gpio_config(&gpio_init);
	gpio_init.pin = 14;
	gpio_config(&gpio_init);
	gpio_init.pin = 15;
	gpio_config(&gpio_init);

	/* GPIOE */
	gpio_init.port = GPIOE;
	gpio_init.pin = 0;
	gpio_config(&gpio_init);
	gpio_init.pin = 1;
	gpio_config(&gpio_init);
	gpio_init.pin = 7;
	gpio_config(&gpio_init);
	gpio_init.pin = 8;
	gpio_config(&gpio_init);
	gpio_init.pin = 9;
	gpio_config(&gpio_init);
	gpio_init.pin = 10;
	gpio_config(&gpio_init);
	gpio_init.pin = 11;
	gpio_config(&gpio_init);
	gpio_init.pin = 12;
	gpio_config(&gpio_init);
	gpio_init.pin = 13;
	gpio_config(&gpio_init);
	gpio_init.pin = 14;
	gpio_config(&gpio_init);
	gpio_init.pin = 15;
	gpio_config(&gpio_init);

	/* GPIOF */
	gpio_init.port = GPIOF;
	gpio_init.pin = 0;
	gpio_config(&gpio_init);
	gpio_init.pin = 1;
	gpio_config(&gpio_init);
	gpio_init.pin = 2;
	gpio_config(&gpio_init);
	gpio_init.pin = 3;
	gpio_config(&gpio_init);
	gpio_init.pin = 4;
	gpio_config(&gpio_init);
	gpio_init.pin = 5;
	gpio_config(&gpio_init);
	gpio_init.pin = 11;
	gpio_config(&gpio_init);
	gpio_init.pin = 12;
	gpio_config(&gpio_init);
	gpio_init.pin = 13;
	gpio_config(&gpio_init);
	gpio_init.pin = 14;
	gpio_config(&gpio_init);
	gpio_init.pin = 15;
	gpio_config(&gpio_init);

	/* GPIOG */
	gpio_init.port = GPIOG;
	gpio_init.pin = 0;
	gpio_config(&gpio_init);
	gpio_init.pin = 1;
	gpio_config(&gpio_init);
	gpio_init.pin = 4;
	gpio_config(&gpio_init);
	gpio_init.pin = 5;
	gpio_config(&gpio_init);
	gpio_init.pin = 8;
	gpio_config(&gpio_init);
	gpio_init.pin = 15;
	gpio_config(&gpio_init);

}

void static __USER_TEXT sdram_init_seq(void)
{
	uint32_t i;
	uint32_t tmpr = 0;
	struct fmc_sdram_cmd fs_cmd;

	fs_cmd.mode = FMC_Command_Mode_CLK_Enabled;
	fs_cmd.target = FMC_Command_Target_bank2;
	fs_cmd.auto_refresh_number = 1;
	fs_cmd.mode_register_definition = 0;

	while (fmc_get_flag(FMC_Bank2_SDRAM, FMC_FLAG_BUSY) != 0) ;

	fmc_sdram_cmd_init(&fs_cmd);

	/* delay 100ms */
	/* FIXME: a proper delay */
	i = 0;
	while (i < 100)
		i++;

	fs_cmd.mode = FMC_Command_Mode_PALL;
	fs_cmd.target = FMC_Command_Target_bank2;
	fs_cmd.auto_refresh_number = 1;
	fs_cmd.mode_register_definition = 0;

	while (fmc_get_flag(FMC_Bank2_SDRAM, FMC_FLAG_BUSY) != 0) ;

	fmc_sdram_cmd_init(&fs_cmd);

	fs_cmd.mode = FMC_Command_Mode_AutoRefresh;
	fs_cmd.target = FMC_Command_Target_bank2;
	fs_cmd.auto_refresh_number = 4;
	fs_cmd.mode_register_definition = 0;

	while (fmc_get_flag(FMC_Bank2_SDRAM, FMC_FLAG_BUSY) != 0) ;

	fmc_sdram_cmd_init(&fs_cmd);

	while (fmc_get_flag(FMC_Bank2_SDRAM, FMC_FLAG_BUSY) != 0) ;

	fmc_sdram_cmd_init(&fs_cmd);

	tmpr = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2          |
	       SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
	       SDRAM_MODEREG_CAS_LATENCY_3           |
	       SDRAM_MODEREG_OPERATING_MODE_STANDARD |
	       SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	fs_cmd.mode = FMC_Command_Mode_LoadMode;
	fs_cmd.target = FMC_Command_Target_bank2;
	fs_cmd.auto_refresh_number = 1;
	fs_cmd.mode_register_definition = tmpr;

	while (fmc_get_flag(FMC_Bank2_SDRAM, FMC_FLAG_BUSY) != 0) ;

	fmc_sdram_cmd_init(&fs_cmd);

	fmc_set_refresh_count(1386);

	while (fmc_get_flag(FMC_Bank2_SDRAM, FMC_FLAG_BUSY) != 0) ;

}
