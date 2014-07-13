#include <platform/stm32f429/lcd.h>
#include <platform/stm32f429/gpio.h>
#include <platform/stm32f429/spi.h>
#include <platform/stm32f429/rcc.h>
#include <platform/stm32f429/sdram.h>
#include <platform/stm32f429/ltdc.h>
#include <platform/stm32f429/dma2d.h>

static __USER_DATA uint16_t current_Tcolor = 0x0000;
static __USER_DATA uint16_t current_Bcolor = 0xFFFF;
static __USER_DATA uint32_t current_layer = LCD_BACKGROUND_LAYER;
static __USER_DATA uint32_t current_frame_buffer = LCD_FRAME_BUFFER;

void __USER_TEXT lcd_init(void)
{
	struct ltdc_cfg ltdc_cfg;

	lcd_ctrllines_init();
	/* reset & then set */
	lcd_chipselect(0);
	lcd_chipselect(1);

	lcd_spi_config();

	lcd_poweron();

	RCC_APB2PeriphClockCmd(RCC_APB2ENR_LTDCEN, 1);

	RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_DMA2DEN, 1);

	lcd_af_gpio_init();

	sdram_init();

	ltdc_cfg.hs_polarity = LTDC_HSPolarity_AL;
	ltdc_cfg.vs_polarity = LTDC_VSPolarity_AL;
	ltdc_cfg.de_polarity = LTDC_DEPolarity_AL;
	ltdc_cfg.pc_polarity = LTDC_PCPolarity_IPC;

	ltdc_cfg.bg_red_value = 0;
	ltdc_cfg.bg_green_value = 0;
	ltdc_cfg.bg_blue_value = 0;

	RCC_PLLSAIConfig(192, 7, 4);
	RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div8);

	RCC_PLLSAICmd(1);

	while (RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == 0) ;

	ltdc_cfg.horizontal_sync = 9;
	ltdc_cfg.vertical_sync = 1;
	ltdc_cfg.accumulated_hbp = 29;
	ltdc_cfg.accumulated_vbp = 3;
	ltdc_cfg.accumulated_active_w = 269;
	ltdc_cfg.accumulated_active_h = 323;
	ltdc_cfg.total_width = 279;
	ltdc_cfg.total_height = 327;

	ltdc_config(&ltdc_cfg);
}

void __USER_TEXT lcd_write_cmd(uint8_t lcdreg)
{
	lcd_ctrllines_write(LCD_WRX_GPIO_PORT, LCD_WRX_PIN, 0);

	lcd_chipselect(0);
	spi_i2s_send(LCD_SPI, lcdreg);

	while (spi_i2s_get_flag(LCD_SPI, SPI_I2S_FLAG_TXE) == 0);

	while (spi_i2s_get_flag(LCD_SPI, SPI_I2S_FLAG_BSY) != 0);

	lcd_chipselect(1);
}

void __USER_TEXT lcd_write_data(uint8_t value)
{
	lcd_ctrllines_write(LCD_WRX_GPIO_PORT, LCD_WRX_PIN, 1);

	lcd_chipselect(0);
	spi_i2s_send(LCD_SPI, value);

	while (spi_i2s_get_flag(LCD_SPI, SPI_I2S_FLAG_TXE) == 0);

	while (spi_i2s_get_flag(LCD_SPI, SPI_I2S_FLAG_BSY) != 0);

	lcd_chipselect(1);
}

void __USER_TEXT lcd_poweron(void)
{
	uint32_t i;
	lcd_write_cmd(0xCA);
	lcd_write_data(0xC3);
	lcd_write_data(0x08);
	lcd_write_data(0x50);
	lcd_write_cmd(0xCF); /* LCD_POWERB */
	lcd_write_data(0x00);
	lcd_write_data(0xC1);
	lcd_write_data(0x30);
	lcd_write_cmd(0xED); /* LCD_POWER_SEQ */
	lcd_write_data(0x64);
	lcd_write_data(0x03);
	lcd_write_data(0x12);
	lcd_write_data(0x81);
	lcd_write_cmd(0xE8); /* LCD_DTCA */
	lcd_write_data(0x85);
	lcd_write_data(0x00);
	lcd_write_data(0x78);
	lcd_write_cmd(0xCB); /* LCD_POWERA */
	lcd_write_data(0x39);
	lcd_write_data(0x2C);
	lcd_write_data(0x00);
	lcd_write_data(0x34);
	lcd_write_data(0x02);
	lcd_write_cmd(0xF7); /* LCD_PRC */
	lcd_write_data(0x20);
	lcd_write_cmd(0xEA); /* LCD_DTCB */
	lcd_write_data(0x00);
	lcd_write_data(0x00);
	lcd_write_cmd(0xB1); /* LCD_FRC */
	lcd_write_data(0x00);
	lcd_write_data(0x1B);
	lcd_write_cmd(0xB6); /* LCD_DFC */
	lcd_write_data(0x0A);
	lcd_write_data(0xA2);
	lcd_write_cmd(0xC0); /* LCD_Power1 */
	lcd_write_data(0x10);
	lcd_write_cmd(0xC1); /* LCD_Power2 */
	lcd_write_data(0x10);
	lcd_write_cmd(0xC5); /* LCD_VCOM1 */
	lcd_write_data(0x45);
	lcd_write_data(0x15);
	lcd_write_cmd(0xC7); /* LCD_VCOM2 */
	lcd_write_data(0x90);
	lcd_write_cmd(0x36); /* LCD_MAC */
	lcd_write_data(0xC8);
	lcd_write_cmd(0xF2); /* LCD_3GAMMA_EN */
	lcd_write_data(0x00);
	lcd_write_cmd(0xB0); /* LCD_RGB_INTERFACE */
	lcd_write_data(0xC2);
	lcd_write_cmd(0xB6); /* LCD_DFC */
	lcd_write_data(0x0A);
	lcd_write_data(0xA7);
	lcd_write_data(0x27);
	lcd_write_data(0x04);

	/* colomn address set */
	lcd_write_cmd(0x2A); /* LCD_COLUMN_ADDR */
	lcd_write_data(0x00);
	lcd_write_data(0x00);
	lcd_write_data(0x00);
	lcd_write_data(0xEF);
	/* Page Address Set */
	lcd_write_cmd(0x2B); /* LCD_PAGE_ADDR */
	lcd_write_data(0x00);
	lcd_write_data(0x00);
	lcd_write_data(0x01);
	lcd_write_data(0x3F);
	lcd_write_cmd(0xF6); /* LCD_INTERFACE */
	lcd_write_data(0x01);
	lcd_write_data(0x00);
	lcd_write_data(0x06);

	lcd_write_cmd(0x2C); /* LCD_GRAM */
	/* L4_Sleep(200); */
	/* L4_Sleep unable to be used at here, so I use while temporary */
	/* TODO: A proper delay */
	i = 0;
	while (i < 400)
		i++;

	lcd_write_cmd(0x26); /* LCD_GAMMA */
	lcd_write_data(0x01);

	lcd_write_cmd(0xE0); /* LCD_PGAMMA */
	lcd_write_data(0x0F);
	lcd_write_data(0x29);
	lcd_write_data(0x24);
	lcd_write_data(0x0C);
	lcd_write_data(0x0E);
	lcd_write_data(0x09);
	lcd_write_data(0x4E);
	lcd_write_data(0x78);
	lcd_write_data(0x3C);
	lcd_write_data(0x09);
	lcd_write_data(0x13);
	lcd_write_data(0x05);
	lcd_write_data(0x17);
	lcd_write_data(0x11);
	lcd_write_data(0x00);
	lcd_write_cmd(0xE1); /* LCD_NGAMMA */
	lcd_write_data(0x00);
	lcd_write_data(0x16);
	lcd_write_data(0x1B);
	lcd_write_data(0x04);
	lcd_write_data(0x11);
	lcd_write_data(0x07);
	lcd_write_data(0x31);
	lcd_write_data(0x33);
	lcd_write_data(0x42);
	lcd_write_data(0x05);
	lcd_write_data(0x0C);
	lcd_write_data(0x0A);
	lcd_write_data(0x28);
	lcd_write_data(0x2F);
	lcd_write_data(0x0F);

	lcd_write_cmd(0x11); /* LCD_SLEEP_OUT */
	/* L4_Sleep(200); */
	/* L4_Sleep unable to be used at here, so I use while temporary */
	/* TODO: A proper delay */
	i = 0;
	while (i < 400)
		i++;
	lcd_write_cmd(0x29); /* LCD_DISPLAY_ON */
	/* GRAM start writing */
	lcd_write_cmd(0x2C); /* LCD_GRAM */
}

void __USER_TEXT lcd_display_off(void)
{
	lcd_write_cmd(0x28);
}

void __USER_TEXT lcd_spi_config(void)
{
	struct spi_cfg spi_init;
	struct gpio_cfg gpio_init;
	/* Enable clock */
	RCC_AHB1PeriphClockCmd(LCD_SPI_SCK_GPIO_CLK | LCD_SPI_MISO_GPIO_CLK | LCD_SPI_MOSI_GPIO_CLK, 1);

	RCC_APB2PeriphClockCmd(LCD_SPI_CLK, 1);

	gpio_init.port = LCD_SPI_SCK_GPIO_PORT;
	gpio_init.pin = LCD_SPI_SCK_PIN;
	gpio_init.speed = GPIO_OSPEEDR_25M;
	gpio_init.pupd = GPIO_PUPDR_DOWN;
	gpio_init.type = GPIO_MODER_ALT;
	gpio_init.func = af_spi5;
	gpio_init.o_type = GPIO_OTYPER_PP;
	gpio_config(&gpio_init);

	gpio_init.port = LCD_SPI_MISO_GPIO_PORT;
	gpio_init.pin = LCD_SPI_MISO_PIN;
	gpio_config(&gpio_init);

	gpio_init.port = LCD_SPI_MOSI_GPIO_PORT;
	gpio_init.pin = LCD_SPI_MOSI_PIN;
	gpio_config(&gpio_init);

	spi_i2s_reset(LCD_SPI);

	if ((*(SPI_CR1(LCD_SPI)) & (SPI_CR1_SPE)) == 0) {
		spi_init.direction = (uint16_t)0x0000; /* 2 lines full duplex */
		spi_init.mode = (uint16_t) 0x0104; /* master */
		spi_init.size = (uint16_t) 0x0000; /* 8b */
		spi_init.cpol = (uint16_t) 0x0000; /* low */
		spi_init.cpha = (uint16_t) 0x0000; /* 1Edge */
		spi_init.nss = (uint16_t) 0x0200; /* soft */
		spi_init.baudrate = (uint16_t) 0x0018; /* prescaler 16 */
		spi_init.firstbit = (uint16_t) 0x0000; /* MSB */
		spi_init.crcpolynomial = 7;
		spi_config(LCD_SPI, &spi_init);
		/* Enable L3GD20_SPI */
		spi_cmd(LCD_SPI, 1);
	}
}

void __USER_TEXT lcd_ctrllines_write(uint8_t port, uint8_t pin, uint8_t reset)
{
	gpio_writebit(port, pin, reset);
}

void __USER_TEXT lcd_ctrllines_init(void)
{
	RCC_AHB1PeriphClockCmd(LCD_NCS_GPIO_CLK | LCD_WRX_GPIO_CLK, 1);
	gpio_config_output(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, GPIO_PUPDR_NONE, GPIO_OSPEEDR_50M);
	gpio_config_output(LCD_WRX_GPIO_PORT, LCD_WRX_PIN, GPIO_PUPDR_NONE, GPIO_OSPEEDR_50M);

	lcd_ctrllines_write(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, 1);
}

void __USER_TEXT lcd_chipselect(uint8_t enable)
{
	if (enable == 0)
		*GPIO_BSRR(LCD_NCS_GPIO_PORT) = GPIO_BSRR_BR(LCD_NCS_PIN);
	else
		*GPIO_BSRR(LCD_NCS_GPIO_PORT) = GPIO_BSRR_BS(LCD_NCS_PIN);
}

void __USER_TEXT lcd_af_gpio_init(void)
{
	struct gpio_cfg gpio_init;

	RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | \
	                       RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN | \
	                       RCC_AHB1ENR_GPIOFEN | RCC_AHB1ENR_GPIOGEN, 1);

	/* GPIOA */
	gpio_init.port = GPIOA;
	gpio_init.speed = GPIO_OSPEEDR_50M;
	gpio_init.pupd = GPIO_PUPDR_NONE;
	gpio_init.type = GPIO_MODER_ALT;
	gpio_init.func = af_ltdc;
	gpio_init.o_type = GPIO_OTYPER_PP;
	gpio_init.pin = 3;
	gpio_config(&gpio_init);
	gpio_init.pin = 4;
	gpio_config(&gpio_init);
	gpio_init.pin = 6;
	gpio_config(&gpio_init);
	gpio_init.pin = 11;
	gpio_config(&gpio_init);
	gpio_init.pin = 12;
	gpio_config(&gpio_init);

	/* GPIOB */
	gpio_init.port = GPIOB;
	gpio_init.func = 0x09;
	gpio_init.pin = 0;
	gpio_config(&gpio_init);
	gpio_init.pin = 1;
	gpio_config(&gpio_init);
	gpio_init.func = af_ltdc;
	gpio_init.pin = 8;
	gpio_config(&gpio_init);
	gpio_init.pin = 9;
	gpio_config(&gpio_init);
	gpio_init.pin = 10;
	gpio_config(&gpio_init);
	gpio_init.pin = 11;
	gpio_config(&gpio_init);

	/* GPIOC */
	gpio_init.port = GPIOC;
	gpio_init.pin = 6;
	gpio_config(&gpio_init);
	gpio_init.pin = 7;
	gpio_config(&gpio_init);
	gpio_init.pin = 10;
	gpio_config(&gpio_init);

	/* GPIOD */
	gpio_init.port = GPIOD;
	gpio_init.pin = 3;
	gpio_config(&gpio_init);
	gpio_init.pin = 6;
	gpio_config(&gpio_init);

	/* GPIOF */
	gpio_init.port = GPIOF;
	gpio_init.pin = 10;
	gpio_config(&gpio_init);

	/* GPIOG */
	gpio_init.port = GPIOG;
	gpio_init.pin = 6;
	gpio_config(&gpio_init);
	gpio_init.pin = 7;
	gpio_config(&gpio_init);
	gpio_init.func = 0x09;
	gpio_init.pin = 10;
	gpio_config(&gpio_init);
	gpio_init.func = af_ltdc;
	gpio_init.pin = 11;
	gpio_config(&gpio_init);
	gpio_init.func = 0x09;
	gpio_init.pin = 12;
	gpio_config(&gpio_init);

}

void __USER_TEXT lcd_layer_init(void)
{
	struct ltdc_layer_cfg layer_init;

	/* Initialize Layer1 */
	layer_init.horizontal_start = 30;
	layer_init.horizontal_end = (LCD_PIXEL_WIDTH + 30 - 1);
	layer_init.vertical_start = 4;
	layer_init.vertical_end = (LCD_PIXEL_HEIGHT + 4 - 1);

	layer_init.pixel_format = LTDC_Pixelformat_RGB565;
	layer_init.constant_alpha = 255;

	layer_init.default_blue = 0;
	layer_init.default_green = 0;
	layer_init.default_red = 0;
	layer_init.default_alpha = 0;

	layer_init.blending_factor1 = LTDC_BlendingFactor1_CA;
	layer_init.blending_factor2 = LTDC_BlendingFactor2_CA;

	layer_init.cfb_line_length = ((LCD_PIXEL_WIDTH * 2) + 3);
	layer_init.cfb_pitch = (LCD_PIXEL_WIDTH * 2);
	layer_init.cfb_line_number = LCD_PIXEL_HEIGHT;
	layer_init.cfb_start_address = LCD_FRAME_BUFFER;

	ltdc_layer_config(LTDC_Layer1, &layer_init);

	/* Initialize Layer2 */
	layer_init.cfb_start_address = LCD_FRAME_BUFFER + BUFFER_OFFSET;

	layer_init.blending_factor1 = LTDC_BlendingFactor1_PAxCA;
	layer_init.blending_factor2 = LTDC_BlendingFactor2_PAxCA;

	ltdc_layer_config(LTDC_Layer2, &layer_init);

	/* ltdc configuration reload */
	ltdc_reload_config(LTDC_IMReload);

	/* Enable Layers */
	ltdc_layer_cmd(LTDC_Layer1, 1);
	ltdc_layer_cmd(LTDC_Layer2, 1);

	/* ltdc configuration reload */
	ltdc_reload_config(LTDC_IMReload);

	/* dithering activation */
	ltdc_dither_cmd(1);
}

void __USER_TEXT lcd_set_text_color(uint16_t color)
{
	current_Tcolor = color;
}

void __USER_TEXT lcd_set_back_color(uint16_t color)
{
	current_Bcolor = color;
}

void __USER_TEXT lcd_set_layer(uint32_t layerx)
{
	if (layerx == LCD_BACKGROUND_LAYER) {
		current_frame_buffer = LCD_FRAME_BUFFER;
		current_layer = LCD_BACKGROUND_LAYER;
	} else {
		current_frame_buffer = LCD_FRAME_BUFFER + BUFFER_OFFSET;
		current_layer = LCD_FOREGROUND_LAYER;
	}
}

void __USER_TEXT lcd_clear(uint16_t color)
{
	uint32_t index = 0;

	for (index = 0x00; index < BUFFER_OFFSET; index++) {
		*(volatile uint16_t *)(current_frame_buffer + (2 * index)) = color;
	}
}

void __USER_TEXT lcd_draw_line(uint16_t x, uint16_t y, uint16_t length, uint8_t direction)
{
	struct dma2d_cfg dma2d_init;
	uint32_t Xaddr = 0;
	uint16_t red = 0, green = 0, blue = 0;

	Xaddr = current_frame_buffer + 2 * (LCD_PIXEL_WIDTH * y + x);

	red = (0xF800 & current_Tcolor) >> 11;
	blue = 0x001F & current_Tcolor;
	green = (0x07E0 & current_Tcolor) >> 5;

	/* Initialize DMA2D */
	dma2d_reset();
	dma2d_init.mode = DMA2D_R2M;
	dma2d_init.cmode = DMA2D_RGB565;
	dma2d_init.output_green = green;
	dma2d_init.output_blue = blue;
	dma2d_init.output_red = red;
	dma2d_init.output_alpha = 0x0F;
	dma2d_init.output_memory_address = Xaddr;

	if (direction == LCD_DIR_HORIZONTAL) {
		dma2d_init.output_offset = 0;
		dma2d_init.number_of_line = 1;
		dma2d_init.pixel_per_line = length;
	} else {
		dma2d_init.output_offset = LCD_PIXEL_WIDTH - 1;
		dma2d_init.number_of_line = length;
		dma2d_init.pixel_per_line = 1;
	}
	dma2d_config(&dma2d_init);

	dma2d_start_transfer();

	while (dma2d_get_flag(DMA2D_FLAG_TC) == 0) ;
}

void __USER_TEXT lcd_draw_rect(uint16_t x, uint16_t y, uint16_t height, uint16_t width)
{
	lcd_draw_line(x, y, width, LCD_DIR_HORIZONTAL);
	lcd_draw_line(x, (y + height), width, LCD_DIR_HORIZONTAL);

	lcd_draw_line(x, y, height, LCD_DIR_VERTICAL);
	lcd_draw_line((x + width), y, height, LCD_DIR_VERTICAL);
}

void __USER_TEXT lcd_fill_rect(uint16_t x, uint16_t y, uint16_t height, uint16_t width)
{
	struct dma2d_cfg dma2d_init;

	uint32_t Xaddr = 0;
	uint16_t red = 0, green = 0, blue = 0;

	Xaddr = current_frame_buffer + 2 * (LCD_PIXEL_WIDTH * y + x);

	red = (0xF800 & current_Tcolor) >> 11;
	blue = 0x001F & current_Tcolor;
	green = (0x07E0 & current_Tcolor) >> 5;

	dma2d_reset();
	dma2d_init.mode = DMA2D_R2M;
	dma2d_init.cmode = DMA2D_RGB565;
	dma2d_init.output_green = green;
	dma2d_init.output_blue = blue;
	dma2d_init.output_red = red;
	dma2d_init.output_alpha = 0x0F;
	dma2d_init.output_memory_address = Xaddr;
	dma2d_init.output_offset = (LCD_PIXEL_WIDTH - width);
	dma2d_init.number_of_line = height;
	dma2d_init.pixel_per_line = width;
	dma2d_config(&dma2d_init);

	dma2d_start_transfer();

	while (dma2d_get_flag(DMA2D_FLAG_TC) == 0) ;

	lcd_set_text_color(current_Tcolor);
}
