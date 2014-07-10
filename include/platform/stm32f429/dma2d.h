#ifndef PLATFORM_STM32F429_DMA2D_H__
#define PLATFORM_STM32F429_DMA2D_H__

#include <platform/stm32f429/registers.h>
#include <platform/link.h>

/* CR Register */
#define DMA2D_CR_START                     (uint32_t) (1 << 0)
#define DMA2D_CR_SUSP                      (uint32_t) (1 << 1)
#define DMA2D_CR_ABORT                     (uint32_t) (1 << 2)
#define DMA2D_CR_TEIE                      (uint32_t) (1 << 8)
#define DMA2D_CR_TCIE                      (uint32_t) (1 << 9)
#define DMA2D_CR_TWIE                      (uint32_t) (1 << 10)
#define DMA2D_CR_CAEIE                     (uint32_t) (1 << 11)
#define DMA2D_CR_CTCIE                     (uint32_t) (1 << 12)
#define DMA2D_CR_CEIE                      (uint32_t) (1 << 13)
#define DMA2D_CR_MODE                      ((uint32_t)0x00030000)
/* Color Mode */
#define DMA2D_OPFCCR_CM                    ((uint32_t)0x00000007)
/* Line Offset */
#define DMA2D_OOR_LO                       ((uint32_t)0x00003FFF)
/* Number of lines */
#define DMA2D_NLR_NL                       ((uint32_t)0x0000FFFF)
/* Pixel Per Lines */
#define DMA2D_NLR_PL                       ((uint32_t)0x3FFF0000)


/* MODE */
#define DMA2D_M2M                          ((uint32_t)0x00000000)
#define DMA2D_M2M_PFC                      ((uint32_t)0x00010000)
#define DMA2D_M2M_BLEND                    ((uint32_t)0x00020000)
#define DMA2D_R2M                          ((uint32_t)0x00030000)
/* CMODE */
#define DMA2D_ARGB8888                     ((uint32_t)0x00000000)
#define DMA2D_RGB888                       ((uint32_t)0x00000001)
#define DMA2D_RGB565                       ((uint32_t)0x00000002)
#define DMA2D_ARGB1555                     ((uint32_t)0x00000003)
#define DMA2D_ARGB4444                     ((uint32_t)0x00000004)

/* FLAG */
#define DMA2D_FLAG_CE                      ((uint32_t) 1 << 5)
#define DMA2D_FLAG_CTC                     ((uint32_t) 1 << 4)
#define DMA2D_FLAG_CAE                     ((uint32_t) 1 << 3)
#define DMA2D_FLAG_TW                      ((uint32_t) 1 << 2)
#define DMA2D_FLAG_TC                      ((uint32_t) 1 << 1)
#define DMA2D_FLAG_TE                      ((uint32_t) 1 << 0)

struct dma2d_cfg {
	uint32_t mode;
	uint32_t cmode;
	uint32_t output_blue;
	uint32_t output_green;
	uint32_t output_red;
	uint32_t output_alpha;
	uint32_t output_memory_address;
	uint32_t output_offset;
	uint32_t number_of_line;
	uint32_t pixel_per_line;
};

void dma2d_reset(void);
void dma2d_config(struct dma2d_cfg *cfg);
void dma2d_start_transfer(void);
uint8_t dma2d_get_flag(uint32_t flag);
#endif
