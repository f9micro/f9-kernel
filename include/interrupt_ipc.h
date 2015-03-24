#ifndef INTERRUPT_IPC_H_
#define INTERRUPT_IPC_H_

/* interrupt ipc message */
enum {
    IRQ_IPC_IRQN = 0,
    IRQ_IPC_TID = 1,
    IRQ_IPC_HANDLER = 2,
    IRQ_IPC_ACTION = 3,
    IRQ_IPC_PRIORITY = 4
};

#define IRQ_IPC_MSG_NUM	IRQ_IPC_PRIORITY

/* irq actions */
enum {
    USER_IRQ_ENABLE = 0,
    USER_IRQ_DISABLE = 1,
    USER_IRQ_FREE = 2
};

#define USER_INTERRUPT_LABEL	0x928

#endif
