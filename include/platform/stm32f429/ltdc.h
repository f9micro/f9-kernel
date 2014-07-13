#ifndef PLATFORM_STM32F429_LTDC_H__
#define PLATFORM_STM32F429_LTDC_H__

#include <platform/link.h>
#include <platform/stm32f429/registers.h>

#define LTDC_HSPolarity_AL               ((uint32_t)0x00000000)                  /* Horizontal Synchronization is active low */
#define LTDC_HSPolarity_AH               ((uint32_t)0x80000000)                  /* Horizontal Synchronization is active high */

#define LTDC_VSPolarity_AL               ((uint32_t)0x00000000)                  /* Vertical Synchronization is active low */
#define LTDC_VSPolarity_AH               ((uint32_t)0x40000000)                  /* Vertical Synchronization is active high */

#define LTDC_DEPolarity_AL               ((uint32_t)0x00000000)                  /* Data Enable, is active low */
#define LTDC_DEPolarity_AH               ((uint32_t)0x20000000)                  /* Data Enable, is active high */

#define LTDC_PCPolarity_IPC              ((uint32_t)0x00000000)                  /* input pixel clock */
#define LTDC_PCPolarity_IIPC             ((uint32_t)0x10000000)                  /* inverted input pixel clock */

/* LTDC pixel format */
#define LTDC_Pixelformat_ARGB8888        ((uint32_t)0x00000000)
#define LTDC_Pixelformat_RGB888          ((uint32_t)0x00000001)
#define LTDC_Pixelformat_RGB565          ((uint32_t)0x00000002)
#define LTDC_Pixelformat_ARGB1555        ((uint32_t)0x00000003)
#define LTDC_Pixelformat_ARGB4444        ((uint32_t)0x00000004)
#define LTDC_Pixelformat_L8              ((uint32_t)0x00000005)
#define LTDC_Pixelformat_AL44            ((uint32_t)0x00000006)
#define LTDC_Pixelformat_AL88            ((uint32_t)0x00000007)

/* LTDC blending factor1 */
#define LTDC_BlendingFactor1_CA          ((uint32_t)0x00000400)
#define LTDC_BlendingFactor1_PAxCA       ((uint32_t)0x00000600)

/* LTDC blending factor2 */
#define LTDC_BlendingFactor2_CA          ((uint32_t)0x00000005)
#define LTDC_BlendingFactor2_PAxCA       ((uint32_t)0x00000007)

/* Layer Enable */
#define LTDC_LxCR_LEN                    ((uint32_t) 1 << 0)
/* Color Keying Enable */
#define LTDC_LxCR_COLKEN                 ((uint32_t) 1 << 1)
/* Color Lockup Table Enable */
#define LTDC_LxCR_CLUTEN                 ((uint32_t) 1 << 4)

/* Dither Enable */
#define LTDC_GCR_DTEN                    ((uint32_t)0x00010000)

/* Window Horizontal Start Position */
#define LTDC_LxWHPCR_WHSPOS             ((uint32_t)0x00000FFF)
/* Window Horizontal End Position */
#define LTDC_LxWHPCR_WHEPOS             ((uint32_t)0xFFFF0000)

/* Window Vertical Start Position */
#define LTDC_LxWVPCR_WVSPOS             ((uint32_t)0x00000FFF)
/* Window Vertical End Position */
#define LTDC_LxWVPCR_WVEPOS             ((uint32_t)0xFFFF0000)

/* Pixel Format */
#define LTDC_LxPFCR_PF                  ((uint32_t)0x00000007)

/* Default Color Blue */
#define LTDC_LxDCCR_DCBLUE              ((uint32_t)0x000000FF)
/* Default Color Green */
#define LTDC_LxDCCR_DCGREEN             ((uint32_t)0x0000FF00)
/* Default Color Red */
#define LTDC_LxDCCR_DCRED               ((uint32_t)0x00FF0000)
/* Default Color Alpha */
#define LTDC_LxDCCR_DCALPHA             ((uint32_t)0xFF000000)

/* Constant Alpha */
#define LTDC_LxCACR_CONSTA              ((uint32_t)0x000000FF)

/* Blending Factor 2 */
#define LTDC_LxBFCR_BF2                 ((uint32_t)0x00000007)
/* Blending Factor 1 */
#define LTDC_LxBFCR_BF1                 ((uint32_t)0x00000700)

/* Color Frame Buffer Start Address */
#define LTDC_LxCFBAR_CFBADD             ((uint32_t)0xFFFFFFFF)
/* Color Frame Buffer Line Length */
#define LTDC_LxCFBLR_CFBLL              ((uint32_t)0x00001FFF)
/* Color Frame Buffer Pitch in Bytes */
#define LTDC_LxCFBLR_CFBP               ((uint32_t)0x1FFF0000)
/* Color Frame Buffer Line Number */
#define LTDC_LxCFBLNR_CFBLNBR           ((uint32_t)0x000007FF)

/* Immediately Reload */
#define LTDC_IMReload                   ((uint32_t) 1 << 0)

struct ltdc_cfg {
	uint32_t hs_polarity;
	uint32_t vs_polarity;
	uint32_t de_polarity;
	uint32_t pc_polarity;
	uint32_t horizontal_sync;
	uint32_t vertical_sync;
	uint32_t accumulated_hbp;
	uint32_t accumulated_vbp;
	uint32_t accumulated_active_w;
	uint32_t accumulated_active_h;
	uint32_t total_width;
	uint32_t total_height;
	uint32_t bg_red_value;
	uint32_t bg_green_value;
	uint32_t bg_blue_value;
};

struct ltdc_layer_cfg {
	uint32_t horizontal_start;
	uint32_t horizontal_end;
	uint32_t vertical_start;
	uint32_t vertical_end;
	uint32_t pixel_format;
	uint32_t constant_alpha;
	uint32_t default_blue;
	uint32_t default_green;
	uint32_t default_red;
	uint32_t default_alpha;
	uint32_t blending_factor1;
	uint32_t blending_factor2;
	uint32_t cfb_start_address;
	uint32_t cfb_line_length;
	uint32_t cfb_pitch;
	uint32_t cfb_line_number;
};

void ltdc_config(struct ltdc_cfg *cfg);
void ltdc_layer_config(uint32_t layerx, struct ltdc_layer_cfg *cfg);
void ltdc_reload_config(uint32_t reload);
void ltdc_cmd(uint8_t enable);
void ltdc_layer_cmd(uint32_t layerx, uint8_t enable);
void ltdc_dither_cmd(uint8_t enable);
#endif
