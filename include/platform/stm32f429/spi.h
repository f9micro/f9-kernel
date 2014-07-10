#ifndef PLATFORM_STM32F429_SPI_H__
#define PLATFORM_STM32F429_SPI_H__

#include <platform/link.h>
#include <platform/stm32f429/registers.h>

#define SPI_I2S_FLAG_RXNE               (uint16_t) (1 << 0)
#define SPI_I2S_FLAG_TXE                (uint16_t) (1 << 1)
#define I2S_FLAG_CHSIDE                 (uint16_t) (1 << 2)
#define I2S_FLAG_UDR                    (uint16_t) (1 << 3)
#define SPI_FLAG_CRCERR                 (uint16_t) (1 << 4)
#define SPI_FLAG_MODF                   (uint16_t) (1 << 5)
#define SPI_I2S_FLAG_OVR                (uint16_t) (1 << 6)
#define SPI_I2S_FLAG_BSY                (uint16_t) (1 << 7)
#define SPI_I2S_FLAG_TIFRFE             (uint16_t) (1 << 8)


struct spi_cfg {
	uint16_t direction;
	uint16_t mode;
	uint16_t size;
	uint16_t cpol;
	uint16_t cpha;
	uint16_t nss;
	uint16_t baudrate;
	uint16_t firstbit;
	uint16_t crcpolynomial;
};

void spi_config(uint32_t spix, struct spi_cfg *spi_init);
void spi_cmd(uint32_t spix, uint8_t enable);
void spi_i2s_send(uint32_t spix, uint16_t data);
void spi_i2s_reset(uint32_t spix);
uint8_t spi_i2s_get_flag(uint32_t spix, uint16_t flag);

#endif
