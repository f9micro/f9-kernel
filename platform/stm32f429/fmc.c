#include <platform/stm32f429/fmc.h>

void __USER_TEXT fmc_sdram_config(struct fmc_sdram_cfg *cfg)
{
	uint32_t tmpr1 = 0;
	uint32_t tmpr2 = 0;
	uint32_t tmpr3 = 0;
	uint32_t tmpr4 = 0;

	/* TODO: assertion */

	tmpr1 = (uint32_t)cfg->column_bits_number |
	        cfg->row_bits_number |
	        cfg->sdmemory_data_width |
	        cfg->internal_bank_number |
	        cfg->cas_latency |
	        cfg->write_protection |
	        cfg->sd_clock_period |
	        cfg->readburst |
	        cfg->readpipe_delay;

	if (cfg->bank == FMC_Bank1_SDRAM) {
		*FMC_Bank5_6_SDCR(cfg->bank) = tmpr1;
	} else {
		tmpr3 = (uint32_t)cfg->sd_clock_period |
		        cfg->readburst |
		        cfg->readpipe_delay;

		*FMC_Bank5_6_SDCR(FMC_Bank1_SDRAM) = tmpr3;
		*FMC_Bank5_6_SDCR(cfg->bank) = tmpr1;
	}

	if (cfg->bank == FMC_Bank1_SDRAM) {
		tmpr2 = (uint32_t)((cfg->timing->lta_delay) - 1) |
		        (((cfg->timing->esr_delay) - 1) << 4) |
		        (((cfg->timing->sr_time) - 1) << 8) |
		        (((cfg->timing->rc_delay) - 1) << 12) |
		        (((cfg->timing->wr_time) - 1) << 16) |
		        (((cfg->timing->rp_delay) - 1) << 20) |
		        (((cfg->timing->rcd_delay) - 1) << 24);

		*FMC_Bank5_6_SDTR(cfg->bank) = tmpr2;
	} else {
		tmpr2 = (uint32_t)((cfg->timing->lta_delay) - 1) |
		        (((cfg->timing->esr_delay) - 1) << 4) |
		        (((cfg->timing->sr_time) - 1) << 8) |
		        (((cfg->timing->wr_time) - 1) << 16);

		tmpr4 = (uint32_t)(((cfg->timing->rc_delay) - 1) << 12) |
		        (((cfg->timing->rp_delay) - 1) << 20);

		*FMC_Bank5_6_SDTR(FMC_Bank1_SDRAM) = tmpr4;
		*FMC_Bank5_6_SDTR(cfg->bank) = tmpr2;
	}

}

uint8_t __USER_TEXT fmc_get_flag(uint32_t bank, uint32_t flag)
{
	uint8_t bitstatus = 0;
	uint32_t tmpsr = 0x00000000;

	/* TODO: assertion */

	if (bank == 999) {
		/* TODO: bank2_nand, bank3_nand, bank4_pccard */
	} else
		tmpsr = *FMC_Bank5_6_SDSR;

	if ((tmpsr & flag) != flag)
		bitstatus = 0;
	else
		bitstatus = 1;

	return bitstatus;
}

void __USER_TEXT fmc_sdram_cmd_init(struct fmc_sdram_cmd *cmd)
{
	uint32_t tmpr = 0x0;

	/* TODO: assertion */

	tmpr = (uint32_t)(cmd->mode | cmd->target |
	                  (((cmd->auto_refresh_number) - 1) << 5) |
	                  ((cmd->mode_register_definition) << 9));

	*FMC_Bank5_6_SDCMR = tmpr;
}

void __USER_TEXT fmc_set_refresh_count(uint32_t count)
{
	/* TODO: assertion */

	*FMC_Bank5_6_SDRTR |= (count << 1);
}
