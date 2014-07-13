#include <platform/stm32f429/i2c.h>
#include <platform/stm32f429/rcc.h>

/* I2C register mask */
#define CR1_CLEAR_MASK           ((uint16_t)0xFBF5)
/* Flag mask */
#define FLAG_MASK                ((uint32_t)0x00FFFFFF)

void __USER_TEXT i2c_reset(uint32_t i2cx)
{
	/* TODO: assertion */

	if (i2cx == I2C1_BASE) {
		RCC_APB1PeriphResetCmd(RCC_APB1RSTR_I2C1RST, 1);
		RCC_APB1PeriphResetCmd(RCC_APB1RSTR_I2C1RST, 0);
	} else if (i2cx == I2C2_BASE) {
		RCC_APB1PeriphResetCmd(RCC_APB1RSTR_I2C2RST, 1);
		RCC_APB1PeriphResetCmd(RCC_APB1RSTR_I2C2RST, 0);
	} else if (i2cx == I2C3_BASE) {
		RCC_APB1PeriphResetCmd(RCC_APB1RSTR_I2C2RST, 1);
		RCC_APB1PeriphResetCmd(RCC_APB1RSTR_I2C2RST, 0);
	}
}

void __USER_TEXT i2c_config(uint32_t i2cx, struct i2c_cfg* cfg)
{
	uint16_t tmpreg = 0, freqrange = 0;
	uint16_t result = 0x04;
	uint32_t pclk1 = 8000000;
	struct rcc_clocks clocks;
	/* TODO: assertion */

	tmpreg = *I2C_CR2(i2cx);
	tmpreg &= (uint16_t)~((uint16_t)I2C_CR2_FREQ_ALL);

	RCC_GetClocksFreq(&clocks);
	pclk1 = clocks.pclk1_freq;

	freqrange = (uint16_t)(pclk1 / 1000000);
	tmpreg |= freqrange;

	*I2C_CR2(i2cx) = tmpreg;

	*I2C_CR1(i2cx) &= (uint16_t)~((uint16_t)I2C_CR1_PE);
	tmpreg = 0;

	if (cfg->clock_speed <= 100000) {
		result = (uint16_t)(pclk1 / (cfg->clock_speed << 1));

		if (result < 0x04)
			result = 0x04;

		tmpreg |= result;
		*I2C_TRISE(i2cx) = freqrange + 1;
	} else {
		if (cfg->duty_cycle == I2C_DutyCycle_2) {
			result = (uint16_t)(pclk1 / (cfg->clock_speed * 3));
		} else {
			result = (uint16_t)(pclk1 / (cfg->clock_speed * 25));
			result |= I2C_DutyCycle_16_9;
		}

		if ((I2C_CCR_CCR(result)) == 0)
			result |= (uint16_t)0x0001;

		tmpreg |= (uint16_t)(result | I2C_CCR_FS);
		*I2C_TRISE(i2cx) = (uint16_t)(((freqrange * (uint16_t)300) / (uint16_t)1000) + (uint16_t)1);
	}
	*I2C_CCR(i2cx) = tmpreg;
	*I2C_CR1(i2cx) |= I2C_CR1_PE;

	tmpreg = *I2C_CR1(i2cx);
	tmpreg &= CR1_CLEAR_MASK;

	tmpreg |= (uint16_t)((uint32_t)cfg->mode | cfg->ack);
	*I2C_CR1(i2cx) = tmpreg;

	*I2C_OAR1(i2cx) = (cfg->acknowledged_address | cfg->own_address);
}

void __USER_TEXT i2c_cmd(uint32_t i2cx, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0)
		*I2C_CR1(i2cx) |= I2C_CR1_PE;
	else
		*I2C_CR1(i2cx) &= (uint16_t)~((uint16_t)I2C_CR1_PE);
}

void __USER_TEXT i2c_generate_start(uint32_t i2cx, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0)
		*I2C_CR1(i2cx) |= I2C_CR1_START;
	else
		*I2C_CR1(i2cx) &= (uint16_t)~((uint16_t)I2C_CR1_START);
}

uint8_t __USER_TEXT i2c_get_flag(uint32_t i2cx, uint32_t flag)
{
	uint8_t bitstatus = 0;
	volatile uint32_t i2creg = 0, i2cxbase = 0;

	/* TODO: assertion */

	i2cxbase = (uint32_t)i2cx;
	i2creg = flag >> 28;

	flag &= FLAG_MASK;

	if (i2creg != 0) {
		i2cxbase += 0x14;
	} else {
		flag = (uint32_t)(flag >> 16);
		i2cxbase += 0x18;
	}

	if (((*(volatile uint32_t *)i2cxbase) & flag) != (uint32_t)0)
		bitstatus = 1;
	else
		bitstatus = 0;

	return bitstatus;
}

void __USER_TEXT i2c_acknowledge_config(uint32_t i2cx, uint8_t enable)
{
	/* TODO: assertion */
	if (enable != 0)
		*I2C_CR1(i2cx) |= I2C_CR1_ACK;
	else
		*I2C_CR1(i2cx) &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
}

void __USER_TEXT i2c_send_7bit_address(uint32_t i2cx, uint8_t address, uint8_t direction)
{
	/* TODO: assertion */

	if (direction != I2C_Direction_Transmitter)
		address |= I2C_OAR1_ADD(0);
	else
		address &= (uint8_t)~((uint8_t)I2C_OAR1_ADD(0));

	*I2C_DR(i2cx) = address;
}

void __USER_TEXT i2c_generate_stop(uint32_t i2cx, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0)
		*I2C_CR1(i2cx) |= I2C_CR1_STOP;
	else
		*I2C_CR1(i2cx) &= (uint16_t)~((uint16_t)I2C_CR1_STOP);
}

void __USER_TEXT i2c_software_reset_cmd(uint32_t i2cx, uint8_t enable)
{
	/* TODO: assertion */

	if (enable != 0)
		*I2C_CR1(i2cx) |= I2C_CR1_SWRST;
	else
		*I2C_CR1(i2cx) &= (uint16_t)~((uint16_t)I2C_CR1_SWRST);
}

void __USER_TEXT i2c_send(uint32_t i2cx, uint8_t data)
{
	/* TODO: assertion */

	*I2C_DR(i2cx) = data;
}

uint8_t __USER_TEXT i2c_receive(uint32_t i2cx)
{
	/* TODO: assertion */

	return (uint8_t)(*I2C_DR(i2cx));
}
