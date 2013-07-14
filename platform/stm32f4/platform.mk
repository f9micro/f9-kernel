VPATH += $(PLATFORM_SRC)/stm32f4/
SRCS += \
	discoveryf4.c \
	gpio.c \
	rcc.c \
	usart.c \
	nvic.c \
	systick.c

INCLUDES += -I$(PLATFORM_INC)/stm32f4/

define platform-flash-command
	st-flash write $1 0x8000000
endef
