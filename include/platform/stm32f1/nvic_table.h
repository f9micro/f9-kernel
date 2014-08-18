/* NVIC platform-speific table generator */

#define IRQ_HANDLER_FUNC(n) \
        nvic_handler##n,

#define IRQ_VEC_N_OP    IRQ_HANDLER_FUNC
#include "platform/stm32f1/nvic_private.h"
#undef IRQ_VEC_N_OP
