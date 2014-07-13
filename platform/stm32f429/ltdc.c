#include <platform/stm32f429/ltdc.h>

#define GCR_MASK                    ((uint32_t)0x0FFE888F)

void __USER_TEXT ltdc_config(struct ltdc_cfg *cfg)
{
	uint32_t horizontalsync = 0;
	uint32_t accumulatedHBP = 0;
	uint32_t accumulatedactiveW = 0;
	uint32_t totalwidth = 0;
	uint32_t backgreen = 0;
	uint32_t backred = 0;

	/* TODO: assertion */

	/* synchronization size */
	*LTDC_SSCR &= ~(LTDC_SSCR_VSH | LTDC_SSCR_HSW);
	horizontalsync = (cfg->horizontal_sync << 16);
	*LTDC_SSCR |= (horizontalsync | cfg->vertical_sync);

	/* accumulated back porch */
	*LTDC_BPCR &= ~(LTDC_BPCR_AVBP | LTDC_BPCR_AHBP);
	accumulatedHBP = (cfg->accumulated_hbp << 16);
	*LTDC_BPCR |= (accumulatedHBP | cfg->accumulated_vbp);

	/* accumulated active width */
	*LTDC_AWCR &= ~(LTDC_AWCR_AAH | LTDC_AWCR_AAW);
	accumulatedactiveW = (cfg->accumulated_active_w << 16);
	*LTDC_AWCR |= (accumulatedactiveW | cfg->accumulated_active_h);

	/* total width */
	*LTDC_TWCR &= ~(LTDC_TWCR_TOTALH | LTDC_TWCR_TOTALW);
	totalwidth = (cfg->total_width << 16);
	*LTDC_TWCR |= (totalwidth | cfg->total_height);

	*LTDC_GCR &= (uint32_t)GCR_MASK;
	*LTDC_GCR |= (uint32_t)(cfg->hs_polarity | cfg->vs_polarity | \
	                        cfg->de_polarity | cfg->pc_polarity);

	/* background color */
	backgreen = (cfg->bg_green_value << 8);
	backred = (cfg->bg_red_value << 16);

	*LTDC_BCCR &= ~(LTDC_BCCR_BCBLUE | LTDC_BCCR_BCGREEN | LTDC_BCCR_BCRED);
	*LTDC_BCCR |= (backred | backgreen | cfg->bg_blue_value);
}

void __USER_TEXT ltdc_layer_config(uint32_t layerx, struct ltdc_layer_cfg *cfg)
{
	uint32_t whsppos = 0;
	uint32_t wvsppos = 0;
	uint32_t dcgreen = 0;
	uint32_t dcred = 0;
	uint32_t dcalpha = 0;
	uint32_t cfbp = 0;

	/* TODO: assertion */

	/* horizontal start and end position */
	whsppos = cfg->horizontal_end << 16;
	*LTDC_Layer_WHPCR(layerx) &= ~(LTDC_LxWHPCR_WHSPOS | LTDC_LxWHPCR_WHEPOS);
	*LTDC_Layer_WHPCR(layerx) = (cfg->horizontal_start | whsppos);

	/* vertical start and end position */
	wvsppos = cfg->vertical_end << 16;
	*LTDC_Layer_WVPCR(layerx) &= ~(LTDC_LxWVPCR_WVSPOS | LTDC_LxWVPCR_WVEPOS);
	*LTDC_Layer_WVPCR(layerx) = (cfg->vertical_start | wvsppos);

	/* pixel format */
	*LTDC_Layer_PFCR(layerx) &= ~(LTDC_LxPFCR_PF);
	*LTDC_Layer_PFCR(layerx) = (cfg->pixel_format);

	/* default color */
	dcgreen = (cfg->default_green << 8);
	dcred = (cfg->default_red << 16);
	dcalpha = (cfg->default_alpha << 24);
	*LTDC_Layer_DCCR(layerx) &= ~(LTDC_LxDCCR_DCBLUE | LTDC_LxDCCR_DCGREEN | LTDC_LxDCCR_DCRED | LTDC_LxDCCR_DCALPHA);
	*LTDC_Layer_DCCR(layerx) = (cfg->default_blue | dcgreen | dcred | dcalpha);

	/* constant alpha value */
	*LTDC_Layer_CACR(layerx) &= ~(LTDC_LxCACR_CONSTA);
	*LTDC_Layer_CACR(layerx) = (cfg->constant_alpha);

	/* blending factors */
	*LTDC_Layer_BFCR(layerx) &= ~(LTDC_LxBFCR_BF2 | LTDC_LxBFCR_BF1);
	*LTDC_Layer_BFCR(layerx) = (cfg->blending_factor1 | cfg->blending_factor2);

	/* color frame buffer start address */
	*LTDC_Layer_CFBAR(layerx) &= ~(LTDC_LxCFBAR_CFBADD);
	*LTDC_Layer_CFBAR(layerx) = (cfg->cfb_start_address);

	/* color frame buffer pitch in byte */
	cfbp = (cfg->cfb_pitch << 16);
	*LTDC_Layer_CFBLR(layerx) &= ~(LTDC_LxCFBLR_CFBLL | LTDC_LxCFBLR_CFBP);
	*LTDC_Layer_CFBLR(layerx) = (cfg->cfb_line_length | cfbp);

	/* color frame buffer line number */
	*LTDC_Layer_CFBLNR(layerx) &= ~(LTDC_LxCFBLNR_CFBLNBR);
	*LTDC_Layer_CFBLNR(layerx) = (cfg->cfb_line_number);
}

void __USER_TEXT ltdc_reload_config(uint32_t reload)
{
	/* TODO: assertion */

	*LTDC_SRCR = (uint32_t)reload;
}

void __USER_TEXT ltdc_cmd(uint8_t enable)
{

	if (enable != 0) {
		*LTDC_GCR |= (uint32_t)LTDC_GCR_LTDCEN;
	} else {
		*LTDC_GCR &= ~(uint32_t)LTDC_GCR_LTDCEN;
	}
}

void __USER_TEXT ltdc_layer_cmd(uint32_t layerx, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0) {
		*LTDC_Layer_CR(layerx) |= (uint32_t)LTDC_LxCR_LEN;
	} else {
		*LTDC_Layer_CR(layerx) &= ~(uint32_t)LTDC_LxCR_LEN;
	}
}

void __USER_TEXT ltdc_dither_cmd(uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0) {
		*LTDC_GCR |= (uint32_t)LTDC_GCR_DTEN;
	} else {
		*LTDC_GCR &= ~(uint32_t)LTDC_GCR_DTEN;
	}
}
