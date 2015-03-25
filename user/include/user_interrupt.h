#ifndef _USER_INTERRUPT_H_
#define _USER_INTERRUPT_H_
#include INC_PLAT(nvic.h)

#include <l4/thread.h>
#include <l4/kip.h>
#include <l4/utcb.h>
#include <l4/ipc.h>
#include <types.h>
#include <user_runtime.h>
#include <interrupt_ipc.h>

#define IRQ_STACK_SIZE	512

typedef void (*irq_handler_t)(void);

__USER_TEXT
L4_Word_t request_irq(int irq, irq_handler_t handler, uint16_t priority);

__USER_TEXT
L4_Word_t enable_irq(int irq);

__USER_TEXT
L4_Word_t disable_irq(int irq);

__USER_TEXT
L4_Word_t free_irq(int irq);

#endif
