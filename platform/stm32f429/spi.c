#include <platform/stm32f429/spi.h>
#include <platform/stm32f429/rcc.h>

#define CR1_CLEAR_MASK           ((uint16_t)0x3040)
#define SPI_I2SCFGR_I2SMOD       ((uint16_t)0x0800)

void __USER_TEXT spi_config(uint32_t spix, struct spi_cfg *spi_init)
{
	uint16_t tmpreg = 0;

	/* TODO: assertion */

	tmpreg = *SPI_CR1(spix);
	tmpreg &= CR1_CLEAR_MASK;

	tmpreg |= (uint16_t)((uint32_t)spi_init->direction | spi_init->mode |
	                     spi_init->size | spi_init->cpol | spi_init->cpha |
	                     spi_init->nss | spi_init->baudrate |
	                     spi_init->firstbit);

	*SPI_CR1(spix) = tmpreg;
	*SPI_I2SCFGR(spix) &= (uint16_t)~((uint16_t)SPI_I2SCFGR_I2SMOD);
	*SPI_CRCPR(spix) = spi_init->crcpolynomial;
}

void __USER_TEXT spi_cmd(uint32_t spix, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0)
		*SPI_CR1(spix) |= SPI_CR1_SPE;
	else
		*SPI_CR1(spix) &= (uint16_t)~((uint16_t)SPI_CR1_SPE);
}

void __USER_TEXT spi_i2s_send(uint32_t spix, uint16_t data)
{
	/* TODO: assertion */

	*SPI_DR(spix) = data;
}

void __USER_TEXT spi_i2s_reset(uint32_t spix)
{
	/* TODO: assertion */

	/* TODO: SPI2,SPI3,SPI4,SPI6 */
	if (spix == SPI1_BASE) {
		/* SPI1 */
	} else if (spix == SPI5_BASE) {
		RCC_APB2PeriphResetCmd(RCC_APB2RSTR_SPI5RST, 1);
		RCC_APB2PeriphResetCmd(RCC_APB2RSTR_SPI5RST, 0);
	}
}

uint8_t __USER_TEXT spi_i2s_get_flag(uint32_t spix, uint16_t flag)
{
	uint8_t bitstatus = 0;
	/* TODO: assertion */

	if (((*SPI_SR(spix)) & flag) != (uint16_t)0)
		bitstatus = 1;
	else
		bitstatus = 0;
	return bitstatus;
}
