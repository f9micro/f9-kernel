#include <platform/stm32f429/dma2d.h>
#include <platform/stm32f429/rcc.h>

//CR MASK
#define CR_MASK                     ((uint32_t)0xFFFCE0FC)

void __USER_TEXT dma2d_reset(void)
{
	RCC_AHB1PeriphResetCmd(RCC_AHB1ENR_DMA2DEN, 1);
	RCC_AHB1PeriphResetCmd(RCC_AHB1ENR_DMA2DEN, 0);
}

void __USER_TEXT dma2d_config(struct dma2d_cfg *cfg)
{
	uint32_t outgreen = 0;
	uint32_t outred = 0;
	uint32_t outalpha = 0;
	uint32_t pixline = 0;

	//TODO: assertion

	*DMA2D_CR &= (uint32_t)CR_MASK;
	*DMA2D_CR |= (cfg->mode);

	*DMA2D_OPFCCR &= ~(uint32_t)DMA2D_OPFCCR_CM;
	*DMA2D_OPFCCR |= (cfg->cmode);

	if (cfg->cmode == DMA2D_ARGB8888) {
		outgreen = cfg->output_green << 8;
		outred = cfg->output_red << 16;
		outalpha = cfg->output_alpha << 24;
	} else if (cfg->cmode == DMA2D_RGB888) {
		outgreen = cfg->output_green << 8;
		outred = cfg->output_red << 16;
		outalpha = (uint32_t)0x00000000;
	} else if (cfg->cmode == DMA2D_RGB565) {
		outgreen = cfg->output_green << 5;
		outred = cfg->output_red << 11;
		outalpha = (uint32_t)0x00000000;
	} else if (cfg->cmode == DMA2D_ARGB1555) {
		outgreen = cfg->output_green << 5;
		outred = cfg->output_red << 10;
		outalpha = cfg->output_alpha << 15;
	} else {
		// DMA2D_ARGB4444
		outgreen = cfg->output_green << 4;
		outred = cfg->output_red << 8;
		outalpha = cfg->output_alpha << 12;
	}
	*DMA2D_OCOLR |= ((outgreen) | (outred) | (cfg->output_blue) | (outalpha));

	*DMA2D_OMAR = (cfg->output_memory_address);

	*DMA2D_OOR &= ~(uint32_t)DMA2D_OOR_LO;
	*DMA2D_OOR |= (cfg->output_offset);

	pixline = cfg->pixel_per_line << 16;
	*DMA2D_NLR &= ~(DMA2D_NLR_NL | DMA2D_NLR_PL);
	*DMA2D_NLR |= ((cfg->number_of_line) | (pixline));
}

void __USER_TEXT dma2d_start_transfer(void)
{
	*DMA2D_CR |= (uint32_t)DMA2D_CR_START;
}

uint8_t __USER_TEXT dma2d_get_flag(uint32_t flag)
{
	uint8_t bitstatus = 0;

	//TODO: assertion

	if (((*DMA2D_ISR) & flag) != (uint32_t)0)
		bitstatus = 1;
	else
		bitstatus = 0;

	return bitstatus;
}
