#include <platform/stm32f429/ioe.h>
#include <platform/stm32f429/i2c.h>
#include <platform/stm32f429/gpio.h>
#include <platform/stm32f429/rcc.h>

#define TIMEOUT_MAX 0x3000

uint32_t __USER_DATA ioe_timeout = TIMEOUT_MAX;

void static ioe_gpio_init(void);
void static ioe_i2c_init(void);
uint8_t static ioe_tp_init(void);
uint8_t static ioe_io_af_config(uint8_t pin, uint8_t enable);

uint8_t __USER_TEXT ioe_init(void)
{
	ioe_gpio_init();

	ioe_i2c_init();

	if (ioe_is_operational()) {
		return IOE_NOT_OPERATIONAL;
	}

	ioe_reset();

	ioe_funct(IOE_ADC_FCT, 1);
	ioe_tp_init();

	return IOE_OK;
}

uint8_t __USER_TEXT ioe_reset(void)
{
	uint8_t i = 0;
	/* power down */
	i2c_write_device_reg(IOE_REG_SYS_CTRL1, 0x02);

	/* FIXME: proper delay */
	while (i < 20) /* delay 2 for here */
		i++;

	/* power on */
	i2c_write_device_reg(IOE_REG_SYS_CTRL1, 0x00);

	return IOE_OK;
}

void static __USER_TEXT ioe_i2c_init(void)
{
	struct i2c_cfg i2c_init;

	if ((*I2C_CR1(I2C3_BASE) * I2C_CR1_PE) == 0) {
		i2c_init.mode = I2C_Mode_I2C;
		i2c_init.duty_cycle = I2C_DutyCycle_2;
		i2c_init.own_address = 0x00;
		i2c_init.ack = I2C_Ack_Enable;
		i2c_init.acknowledged_address = I2C_AcknowledgedAddress_7bit;
		i2c_init.clock_speed = I2C_SPEED;

		i2c_config(I2C3_BASE, &i2c_init);

		i2c_cmd(I2C3_BASE, 1);
	}

}

void static __USER_TEXT ioe_gpio_init(void)
{
	struct gpio_cfg gpio_init;

	RCC_APB1PeriphClockCmd(IOE_I2C_CLK, 1);
	RCC_AHB1PeriphClockCmd(IOE_I2C_SCL_GPIO_CLK | IOE_I2C_SDA_GPIO_CLK |
	                       IOE_IT_GPIO_CLK, 1);
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_SYSCFGEN, 1);

	RCC_APB1PeriphResetCmd(IOE_I2C_CLK, 1);
	RCC_APB1PeriphResetCmd(IOE_I2C_CLK, 0);

	gpio_init.pin = 8;
	gpio_init.type = GPIO_MODER_ALT;
	gpio_init.speed = GPIO_OSPEEDR_50M;
	gpio_init.o_type = GPIO_OTYPER_OD;
	gpio_init.pupd = GPIO_PUPDR_NONE;
	gpio_init.port = IOE_I2C_SCL_GPIO_PORT;
	gpio_init.func = af_i2c3;
	gpio_config(&gpio_init);

	gpio_init.port = IOE_I2C_SDA_GPIO_PORT;
	gpio_init.pin = 9;
	gpio_config(&gpio_init);
}

uint8_t static __USER_TEXT ioe_tp_init(void)
{
	uint8_t i = 0;
	/* Enable touch Panel functionality */
	ioe_funct(IOE_TP_FCT, 1);

	/* Select Sample Time, bit number and ADC Reference */
	i2c_write_device_reg(IOE_REG_ADC_CTRL1, 0x49);

	/* FIXME: proper delay */
	while (i < 20) /* delay 2 for here */
		i++;

	/* Select the ADC clock speed: 3.25 MHz */
	i2c_write_device_reg(IOE_REG_ADC_CTRL2, 0x01);

	/* Select TSC pins in non default mode */
	ioe_io_af_config((uint8_t)TOUCH_IO_ALL, 0);

	/* Select 2 nF filter capacitor */
	i2c_write_device_reg(IOE_REG_TP_CFG, 0x9A);

	/* Select single point reading  */
	i2c_write_device_reg(IOE_REG_FIFO_TH, 0x01);

	/* Write 0x01 to clear the FIFO memory content. */
	i2c_write_device_reg(IOE_REG_FIFO_STA, 0x01);

	/* Write 0x00 to put the FIFO back into operation mode  */
	i2c_write_device_reg(IOE_REG_FIFO_STA, 0x00);

	/* set the data format for Z value: 7 fractional part and 1 whole part */
	i2c_write_device_reg(IOE_REG_TP_FRACT_XYZ, 0x01);

	/* set the driving capability of the device for TSC pins: 50mA */
	i2c_write_device_reg(IOE_REG_TP_I_DRIVE, 0x01);

	/* Use no tracking index, touch-panel controller operation mode (XYZ) and enable the TSC */
	i2c_write_device_reg(IOE_REG_TP_CTRL, 0x03);

	/*  Clear all the status pending bits */
	i2c_write_device_reg(IOE_REG_INT_STA, 0xFF);

	/* Initialize the TS structure to their default values */
	/* TP_State.TouchDetected = TP_State.X = TP_State.Y = TP_State.Z = 0; */

	/* All configuration done */
	return IOE_OK;
}

uint8_t static __USER_TEXT ioe_io_af_config(uint8_t pin, uint8_t enable)
{
	uint8_t tmp = 0;

	tmp = i2c_read_device_reg(IOE_REG_GPIO_AF);

	if (enable != 0)
		tmp |= (uint8_t)pin;
	else
		tmp &= ~(uint8_t)pin;

	i2c_write_device_reg(IOE_REG_GPIO_AF, tmp);

	return IOE_OK;
}

uint8_t __USER_TEXT ioe_funct(uint8_t func, uint8_t enable)
{
	uint8_t tmp = 0;

	tmp = i2c_read_device_reg(IOE_REG_SYS_CTRL2);

	if (enable != 0)
		tmp &= ~(uint8_t)func;
	else
		tmp |= (uint8_t)func;

	i2c_write_device_reg(IOE_REG_SYS_CTRL2, tmp);

	return IOE_OK;
}

uint8_t __USER_TEXT ioe_is_operational(void)
{
	if (ioe_read_id() != (uint16_t)STMPE811_ID) {
		if (ioe_timeout == 0)
			return(ioe_timeout_user_callback());
		else
			return IOE_FAILURE;
	} else {
		return IOE_OK;
	}
}

uint16_t __USER_TEXT ioe_read_id(void)
{
	uint16_t tmp = 0;

	tmp = i2c_read_device_reg(0);
	tmp = (uint32_t)(tmp << 8);
	tmp |= (uint32_t)i2c_read_device_reg(1);

	return (uint16_t)tmp;
}

uint8_t __USER_TEXT i2c_read_device_reg(uint8_t addr)
{
	uint8_t tmp = 0;

	i2c_generate_start(I2C3_BASE, 1);
	ioe_timeout = TIMEOUT_MAX;

	while (!i2c_get_flag(I2C3_BASE, I2C_FLAG_SB)) {
		if (ioe_timeout -- == 0)
			return(ioe_timeout_user_callback());
	}

	i2c_acknowledge_config(I2C3_BASE, 0);

	i2c_send_7bit_address(I2C3_BASE, IOE_ADDR, I2C_Direction_Transmitter);

	ioe_timeout = TIMEOUT_MAX;
	while (!i2c_get_flag(I2C3_BASE, I2C_FLAG_ADDR)) {
		if (ioe_timeout -- == 0)
			return(ioe_timeout_user_callback());
	}

	*I2C_SR2(I2C3_BASE); /* Read status register 2 to clear ADDR flag */

	ioe_timeout = TIMEOUT_MAX;
	while (!i2c_get_flag(I2C3_BASE, I2C_FLAG_TXE)) {
		if (ioe_timeout -- == 0)
			return(ioe_timeout_user_callback());
	}

	i2c_send(I2C3_BASE, addr);

	ioe_timeout = TIMEOUT_MAX;
	while ((!i2c_get_flag(I2C3_BASE, I2C_FLAG_TXE)) || (!i2c_get_flag(I2C3_BASE, I2C_FLAG_BTF))) {
		if (ioe_timeout -- == 0)
			return(ioe_timeout_user_callback());
	}

	i2c_generate_start(I2C3_BASE, 1);

	ioe_timeout = TIMEOUT_MAX;
	while (!i2c_get_flag(I2C3_BASE, I2C_FLAG_SB)) {
		if (ioe_timeout -- == 0)
			return(ioe_timeout_user_callback());
	}

	i2c_send_7bit_address(I2C3_BASE, IOE_ADDR, I2C_Direction_Receiver);

	ioe_timeout = TIMEOUT_MAX;
	while (!i2c_get_flag(I2C3_BASE, I2C_FLAG_ADDR)) {
		if (ioe_timeout -- == 0)
			return(ioe_timeout_user_callback());
	}

	*I2C_SR2(I2C3_BASE); /* read status register 2 to clear ADDR flag (2) */

	ioe_timeout = TIMEOUT_MAX;
	while (!i2c_get_flag(I2C3_BASE, I2C_FLAG_RXNE)) {
		if (ioe_timeout -- == 0)
			return(ioe_timeout_user_callback());
	}

	i2c_generate_stop(I2C3_BASE, 1);

	tmp = i2c_receive(I2C3_BASE);

	i2c_acknowledge_config(I2C3_BASE, 1);

	return tmp;
}

uint8_t __USER_TEXT i2c_write_device_reg(uint8_t addr, uint8_t value)
{
	uint32_t read_verif = 0;

	i2c_generate_start(I2C3_BASE, 1);

	ioe_timeout = TIMEOUT_MAX;
	while (!i2c_get_flag(I2C3_BASE, I2C_FLAG_SB)) {
		if (ioe_timeout-- == 0)
			return (ioe_timeout_user_callback());
	}

	i2c_send_7bit_address(I2C3_BASE, IOE_ADDR, I2C_Direction_Transmitter);

	ioe_timeout = TIMEOUT_MAX;
	while (!i2c_get_flag(I2C3_BASE, I2C_FLAG_ADDR)) {
		if (ioe_timeout-- == 0)
			return (ioe_timeout_user_callback());
	}

	*I2C_SR2(I2C3_BASE); /* read status register 2 to clear ADDR flag (1) */

	ioe_timeout = TIMEOUT_MAX;
	while (!i2c_get_flag(I2C3_BASE, I2C_FLAG_TXE)) {
		if (ioe_timeout-- == 0)
			return (ioe_timeout_user_callback());
	}

	i2c_send(I2C3_BASE, addr);

	ioe_timeout = TIMEOUT_MAX;
	while (!i2c_get_flag(I2C3_BASE, I2C_FLAG_TXE)) {
		if (ioe_timeout-- == 0)
			return (ioe_timeout_user_callback());
	}

	i2c_send(I2C3_BASE, value);

	ioe_timeout = TIMEOUT_MAX;
	while ((!i2c_get_flag(I2C3_BASE, I2C_FLAG_TXE)) | (!i2c_get_flag(I2C3_BASE, I2C_FLAG_BTF))) {
		if (ioe_timeout-- == 0)
			return (ioe_timeout_user_callback());
	}

	i2c_generate_stop(I2C3_BASE, 1);

	return read_verif;
}

uint8_t __USER_TEXT ioe_timeout_user_callback(void)
{
	struct i2c_cfg i2c_init;

	i2c_generate_stop(I2C3_BASE, 1);
	i2c_software_reset_cmd(I2C3_BASE, 1);
	i2c_software_reset_cmd(I2C3_BASE, 0);

	ioe_gpio_init();

	i2c_reset(I2C3_BASE);
	i2c_init.mode = I2C_Mode_I2C;
	i2c_init.duty_cycle = I2C_DutyCycle_2;
	i2c_init.own_address = 0x00;
	i2c_init.ack = I2C_Ack_Enable;
	i2c_init.acknowledged_address = I2C_AcknowledgedAddress_7bit;
	i2c_init.clock_speed = I2C_SPEED;

	i2c_cmd(I2C3_BASE, 1);
	i2c_config(I2C3_BASE, &i2c_init);

	return 0;
}

uint16_t __USER_TEXT ioe_tp_get_state(void)
{
	uint16_t status;

	status = (i2c_read_device_reg(IOE_REG_TP_CTRL) & 0x80);
	return status;
}
