#ifndef PLATFORM_STM32F429_I2C_H__
#define PLATFORM_STM32F429_I2C_H__

#include <platform/stm32f429/registers.h>
#include <platform/link.h>

struct i2c_cfg {
	uint32_t clock_speed;
	uint16_t mode;
	uint16_t duty_cycle;
	uint16_t own_address;
	uint16_t ack;
	uint16_t acknowledged_address;
};

#define I2C_Mode_I2C                    ((uint16_t)0x0000)
#define I2C_Mode_SMBusDevice            ((uint16_t)0x0002)
#define I2C_Mode_SMBusHost              ((uint16_t)0x000A)

/* duty cycle */
/* Fast mode Tlow/Thigh = 16/9 */
#define I2C_DutyCycle_16_9              ((uint16_t)0x4000)
/* Fast mode Tlow/Thigh = 2 */
#define I2C_DutyCycle_2                 ((uint16_t)0xBFFF)

/* Ack */
#define I2C_Ack_Enable                  ((uint16_t)0x0400)
#define I2C_Ack_Disable                 ((uint16_t)0x0000)

/* Acknowledged address */
#define I2C_AcknowledgedAddress_7bit    ((uint16_t)0x4000)
#define I2C_AcknowledgedAddress_10bit   ((uint16_t)0xC000)

/* i2c speed */
#define I2C_SPEED                       100000

/* direction */
#define  I2C_Direction_Transmitter      ((uint8_t)0x00)
#define  I2C_Direction_Receiver         ((uint8_t)0x01)

/* SR1 flag */
#define I2C_FLAG_SMBALERT               ((uint32_t)0x10008000)
#define I2C_FLAG_TIMEOUT                ((uint32_t)0x10004000)
#define I2C_FLAG_PECERR                 ((uint32_t)0x10001000)
#define I2C_FLAG_OVR                    ((uint32_t)0x10000800)
#define I2C_FLAG_AF                     ((uint32_t)0x10000400)
#define I2C_FLAG_ARLO                   ((uint32_t)0x10000200)
#define I2C_FLAG_BERR                   ((uint32_t)0x10000100)
#define I2C_FLAG_TXE                    ((uint32_t)0x10000080)
#define I2C_FLAG_RXNE                   ((uint32_t)0x10000040)
#define I2C_FLAG_STOPF                  ((uint32_t)0x10000010)
#define I2C_FLAG_ADD10                  ((uint32_t)0x10000008)
#define I2C_FLAG_BTF                    ((uint32_t)0x10000004)
#define I2C_FLAG_ADDR                   ((uint32_t)0x10000002)
#define I2C_FLAG_SB                     ((uint32_t)0x10000001)

void i2c_reset(uint32_t i2cx);
void i2c_config(uint32_t i2cx, struct i2c_cfg* cfg);
void i2c_cmd(uint32_t i2cx, uint8_t enable);
void i2c_generate_start(uint32_t i2cx, uint8_t enable);
uint8_t i2c_get_flag(uint32_t i2cx, uint32_t flag);
void i2c_acknowledge_config(uint32_t i2cx, uint8_t enable);
void i2c_send_7bit_address(uint32_t i2cx, uint8_t address, uint8_t direction);
void i2c_generate_stop(uint32_t i2cx, uint8_t enable);
void i2c_software_reset_cmd(uint32_t i2cx, uint8_t enable);
void i2c_send(uint32_t i2cx, uint8_t data);
uint8_t i2c_receive(uint32_t i2cx);
#endif
