#ifndef INTERRUPT_IPC_H_
#define INTERRUPT_IPC_H_

/* interrupt ipc message */
enum {
    IRQ_IPC_IRQN = 0,
    IRQ_IPC_TID = 1,
    IRQ_IPC_HANDLER = 2,
    IRQ_IPC_ACTION = 3,
    IRQ_IPC_PRIORITY = 4,
    IRQ_IPC_FLAGS = 5
};

#define IRQ_IPC_MSG_NUM IRQ_IPC_FLAGS

/* irq actions */
enum { USER_IRQ_ENABLE = 0, USER_IRQ_DISABLE = 1, USER_IRQ_FREE = 2 };

/* IRQ delivery mode flags */
#define IRQ_DELIVER_IPC 0x0000    /* Full IPC delivery (default) */
#define IRQ_DELIVER_NOTIFY 0x0001 /* Fast notification delivery */

#define USER_INTERRUPT_LABEL 0x928

#endif
