#ifndef USER_EXTI_H_
#define USER_EXTI_H_

#include <platform/link.h>

__USER_TEXT
void exti_config(uint32_t line, uint32_t mode, uint32_t trigger_type);

__USER_TEXT
void exti_enable(uint32_t line, uint32_t mode);

__USER_TEXT
void exti_disable(uint32_t line);

__USER_TEXT
void exti_launch_sw_interrupt(uint32_t line);

__USER_TEXT
void exti_clear(uint32_t line);

#endif
