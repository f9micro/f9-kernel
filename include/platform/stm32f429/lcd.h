#ifndef PLATFORM_STM32F429_LCD_H__
#define PLATFORM_STM32F429_LCD_H__

#include <platform/link.h>
#include <platform/stm32f429/registers.h>

#define LCD_PIXEL_WIDTH     ((uint16_t)240)
#define LCD_PIXEL_HEIGHT    ((uint16_t)320)

#define LCD_FRAME_BUFFER	((uint32_t)0xD0000000)
#define BUFFER_OFFSET		((uint32_t)0x50000)

#define LCD_BACKGROUND_LAYER     0x0000
#define LCD_FOREGROUND_LAYER     0x0001

#define LCD_NCS_PIN			2
#define LCD_NCS_GPIO_PORT	GPIOC
#define LCD_NCS_GPIO_CLK	RCC_AHB1ENR_GPIOCEN
#define LCD_NCS_PIN			2

#define LCD_WRX_GPIO_PORT	GPIOD
#define LCD_WRX_GPIO_CLK	RCC_AHB1ENR_GPIODEN
#define LCD_WRX_PIN			13

#define LCD_SPI_SCK_PIN		7
#define LCD_SPI_SCK_GPIO_PORT	GPIOF
#define LCD_SPI_SCK_GPIO_CLK	RCC_AHB1ENR_GPIOFEN
#define LCD_SPI_MISO_PIN	8
#define LCD_SPI_MISO_GPIO_PORT	GPIOF
#define LCD_SPI_MISO_GPIO_CLK	RCC_AHB1ENR_GPIOFEN
#define LCD_SPI_MOSI_PIN	9
#define LCD_SPI_MOSI_GPIO_PORT	GPIOF
#define LCD_SPI_MOSI_GPIO_CLK	RCC_AHB1ENR_GPIOFEN
#define LCD_SPI                 SPI5_BASE
#define LCD_SPI_CLK             RCC_APB2ENR_SPI5EN

/* LCD COLOR */
#define LCD_COLOR_WHITE          0xFFFF
#define LCD_COLOR_BLACK          0x0000
#define LCD_COLOR_GREY           0xF7DE
#define LCD_COLOR_BLUE           0x001F
#define LCD_COLOR_BLUE2          0x051F
#define LCD_COLOR_RED            0xF800
#define LCD_COLOR_MAGENTA        0xF81F
#define LCD_COLOR_GREEN          0x07E0
#define LCD_COLOR_CYAN           0x7FFF
#define LCD_COLOR_YELLOW         0xFFE0

/* LCD Direction */
#define LCD_DIR_HORIZONTAL       0x0000
#define LCD_DIR_VERTICAL         0x0001

void lcd_init(void);
void lcd_write_cmd(uint8_t lcdreg);
void lcd_write_data(uint8_t value);
void lcd_poweron(void);
void lcd_display_off(void);
void lcd_spi_config(void);
void lcd_ctrllines_write(uint8_t port, uint8_t pin, uint8_t reset);
void lcd_ctrllines_init(void);
void lcd_chipselect(uint8_t enable);
void lcd_af_gpio_init(void);
void lcd_layer_init(void);
void lcd_set_text_color(uint16_t color);
void lcd_set_back_color(uint16_t color);
void lcd_set_layer(uint32_t layerx);

void lcd_clear(uint16_t color);
void lcd_draw_line(uint16_t x, uint16_t y, uint16_t length, uint8_t direction);
void lcd_draw_rect(uint16_t x, uint16_t y, uint16_t height, uint16_t width);
void lcd_fill_rect(uint16_t x, uint16_t y, uint16_t height, uint16_t width);
#endif

