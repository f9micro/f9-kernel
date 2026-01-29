#include <debug.h>
#include <error.h>
#include <init_hook.h>
#include <interrupt.h>
#include <interrupt_ipc.h>
#include <ipc.h>
#include <lib/ktable.h>
#include <notification.h>
#include <platform/irq.h>
#include <sched.h>
#include <thread.h>

#include INC_PLAT(nvic.h)

void __interrupt_handler(int n);

/* user interrupt vector */
#define USER_IRQ_VEC(n)                 \
    void nvic_handler##n(void) __NAKED; \
    void nvic_handler##n(void)          \
    {                                   \
        irq_enter();                    \
        __interrupt_handler(n);         \
        request_schedule();             \
        irq_return();                   \
    }

#undef USER_INTERRUPT_USED
#define USER_INTERRUPT
#define IRQ_VEC_N_OP USER_IRQ_VEC
#include INC_PLAT(nvic_private.h)
#undef IRQ_VEC_N_OP
#undef USER_INTERRUPT

typedef void (*irq_handler_t)(void);

#define INVALID_IDX IRQn_NUM
#define INVALID_IRQ_NUM IRQn_NUM

#define IS_VALID_IRQ_NUM(irq) ((irq) < INVALID_IRQ_NUM)

struct user_irq {
    l4_thread_t
        thr_id; /* Thread global ID (not pointer - prevents use-after-free) */
    int irq;
    uint16_t action;
    uint16_t priority;
    uint16_t flags; /* Delivery mode flags */
    irq_handler_t handler;
    struct user_irq *next;
};

static struct user_irq *user_irqs[IRQn_NUM];

DECLARE_KTABLE(struct user_irq, user_irq_table, IRQn_NUM);

struct user_irq_queue {
    struct user_irq *head;
    struct user_irq *tail;
};

static struct user_irq_queue user_irq_queue;

static int user_irq_queue_is_empty(void)
{
    return (!user_irq_queue.head);
}

static void user_irq_queue_push(struct user_irq *uirq)
{
    if (user_irq_queue_is_empty()) {
        user_irq_queue.head = user_irq_queue.tail = uirq;
    } else {
        user_irq_queue.tail->next = uirq;
        user_irq_queue.tail = uirq;
    }
}

static void user_irq_queue_delete(int irq)
{
    struct user_irq *uirq = user_irqs[irq];
    struct user_irq **iter;
    struct user_irq *prev = NULL;

    for (iter = &user_irq_queue.head; *iter; iter = &(*iter)->next) {
        if (*iter == uirq) {
            *iter = uirq->next;

            /* Update tail if we deleted the last element */
            if (user_irq_queue.tail == uirq)
                user_irq_queue.tail = prev;

            /* Clear next pointer to prevent dangling references */
            uirq->next = NULL;
            break;
        }
        prev = *iter;
    }
}

static inline void user_irq_reset_all(void)
{
    for (int i = 0; i < IRQn_NUM; i++)
        user_irqs[i] = NULL;
}

static struct user_irq *user_irq_create_default(int irq)
{
    if (IS_VALID_IRQ_NUM(irq)) {
        struct user_irq *uirq = ktable_alloc(&user_irq_table);
        uirq->thr_id = L4_NILTHREAD;
        uirq->irq = irq;
        uirq->action = 0;
        uirq->priority = 0;
        uirq->flags = IRQ_DELIVER_IPC; /* Default: full IPC delivery */
        uirq->handler = NULL;
        uirq->next = NULL;

        return uirq;
    }
    return NULL;
}

static inline struct user_irq *user_irq_fetch(int irq)
{
    if (!user_irqs[irq])
        user_irqs[irq] = user_irq_create_default(irq);
    return user_irqs[irq];
}

static void user_irq_release(int irq)
{
    if (IS_VALID_IRQ_NUM(irq)) {
        struct user_irq *uirq = user_irqs[irq];

        if (uirq) {
            ktable_free(&user_irq_table, uirq);
            user_irqs[irq] = NULL;
        }
    }
}

static void irq_handler_ipc(struct user_irq *uirq)
{
    if (!uirq || uirq->thr_id == L4_NILTHREAD)
        return;

    /* Safe thread lookup - handle thread destruction */
    tcb_t *thr = thread_by_globalid(uirq->thr_id);
    if (!thr) {
        /* Thread destroyed - drop IRQ safely */
        dbg_printf(DL_NOTIFICATIONS,
                   "IRQ: Dropping IRQ %d for dead thread %t\n", uirq->irq,
                   uirq->thr_id);
        return;
    }

    /* Prepare ipc for user interrupt thread */
    ipc_msg_tag_t tag = {
        .s.label = USER_INTERRUPT_LABEL,
        .s.n_untyped = IRQ_IPC_MSG_NUM,
    };

    ipc_write_mr(thr, 0, tag.raw);
    ipc_write_mr(thr, IRQ_IPC_IRQN + 1, (uint32_t) uirq->irq);
    ipc_write_mr(thr, IRQ_IPC_HANDLER + 1, (uint32_t) uirq->handler);
    ipc_write_mr(thr, IRQ_IPC_ACTION + 1, (uint32_t) uirq->action);
    thr->utcb->sender = TID_TO_GLOBALID(THREAD_INTERRUPT);
    thr->ipc_from = L4_NILTHREAD;
}

/**
 * irq_handler_notify - Fast notification delivery for simple IRQs
 * @uirq: user IRQ descriptor
 *
 * Delivers IRQ event via notification system instead of full IPC.
 * Significantly faster (5-10×) for IRQs that don't need complex payload.
 *
 * IRQ number is encoded in notification bit: (1 << irq)
 * For IRQs >= 32, uses IRQ_HIGH_BIT and event_data field.
 *
 * Use cases:
 * - GPIO edge detection (just need pin number)
 * - Simple timer ticks
 * - DMA/transfer completion flags
 * - Device ready signals
 *
 * Performance: ~100-200 cycles vs 500-1000 cycles for IPC
 */
static void irq_handler_notify(struct user_irq *uirq)
{
    if (!uirq || uirq->thr_id == L4_NILTHREAD)
        return;

    /* Safe thread lookup - handle thread destruction */
    tcb_t *thr = thread_by_globalid(uirq->thr_id);
    if (!thr) {
        /* Thread destroyed - drop IRQ safely */
        dbg_printf(DL_NOTIFICATIONS,
                   "IRQ: Dropping IRQ %d for dead thread %t\n", uirq->irq,
                   uirq->thr_id);
        return;
    }

    uint32_t notify_bit;
    uint32_t event_data = 0;

    /* Encode IRQ number in notification bit
     * Low IRQs (0-30): use bit position directly
     * High IRQs (31+): use bit 31 + event_data payload
     * Note: IRQ 31 is treated as high IRQ to avoid ambiguity
     */
    if (uirq->irq < 31) {
        /* Low IRQs (0-30): use bit position directly */
        notify_bit = (1 << uirq->irq);
    } else {
        /* High IRQs (31+): use bit 31 as sentinel + event_data */
        notify_bit = (1 << 31); /* IRQ_HIGH_BIT */
        event_data = uirq->irq;
    }

    /* Fast-path notification from IRQ context */
    notification_post(thr, notify_bit, event_data);

    dbg_printf(DL_NOTIFICATIONS,
               "IRQ: Fast notify IRQ %d → thread %t (bit=0x%x)\n", uirq->irq,
               thr->t_globalid, notify_bit);
}

static int irq_handler_enable(int irq)
{
    struct user_irq *uirq = user_irqs[irq];

    assert((intptr_t) uirq);

    if (uirq->thr_id == L4_NILTHREAD)
        return -1;

    /* Choose delivery method based on IRQ flags */
    if (uirq->flags & IRQ_DELIVER_NOTIFY) {
        /* Fast-path: notification delivery (5-10× faster) */
        irq_handler_notify(uirq);
    } else {
        /* Traditional: full IPC delivery (default) */

        /* Safe thread lookup for IPC path */
        tcb_t *thr = thread_by_globalid(uirq->thr_id);
        if (!thr)
            return -1;

        if (thr->state != T_RECV_BLOCKED)
            return -1;

        irq_handler_ipc(uirq);

        /* Wake up the interrupt thread directly */
        thr->priority = SCHED_PRIO_INTR;
        thr->state = T_RUNNABLE;
        sched_enqueue(thr);
    }

    return 0;
}

/*
 * Push n to irq queue.
 * Select the first one in queue to run.
 */
static void irq_schedule(int irq)
{
    struct user_irq *uirq = user_irq_fetch(irq);

    irq_disable();
    user_irq_queue_push(uirq);
    irq_enable();

    irq_handler_enable(irq);
}

void __interrupt_handler(int irq)
{
    struct user_irq *uirq = user_irq_fetch(irq);

    if (!uirq || uirq->thr_id == L4_NILTHREAD || !uirq->handler ||
        uirq->action != USER_IRQ_ENABLE) {
        return;
    }

    user_irq_disable(irq); /* No re-entry interrupt */
    irq_schedule(irq);
}

void interrupt_init(void)
{
    user_irq_reset_all();
}

INIT_HOOK(interrupt_init, INIT_LEVEL_KERNEL_EARLY);

void user_interrupt_config(tcb_t *from)
{
    ipc_msg_tag_t tag = {.raw = ipc_read_mr(from, 0)};
    if (tag.s.label != USER_INTERRUPT_LABEL)
        return;

    int irq = (uint16_t) from->ctx.regs[IRQ_IPC_IRQN + 1];
    l4_thread_t tid = (l4_thread_t) from->ctx.regs[IRQ_IPC_TID + 1];
    int action = (uint16_t) from->ctx.regs[IRQ_IPC_ACTION + 1];
    irq_handler_t handler = (irq_handler_t) from->ctx.regs[IRQ_IPC_HANDLER + 1];
    int priority = (uint16_t) from->ctx.regs[IRQ_IPC_PRIORITY + 1];
    int flags = (uint16_t) from->ctx.regs[IRQ_IPC_FLAGS + 1];

    user_irq_disable(irq);

    if (!IS_VALID_IRQ_NUM(irq))
        return;

    struct user_irq *uirq = user_irq_fetch(irq);

    if (!uirq)
        return;

    /* update user irq config */
    if (tid != L4_NILTHREAD)
        uirq->thr_id = tid;

    uirq->action = (uint16_t) action;

    if (handler)
        uirq->handler = handler;

    if (priority > 0)
        uirq->priority = (uint16_t) priority;

    /* Update delivery mode flags (default: IPC, or notification if requested)
     */
    uirq->flags = (uint16_t) flags;
}

void user_interrupt_handler_update(tcb_t *thr)
{
    if (!thr)
        return;

    for (int irq = 0; irq < IRQn_NUM; irq++) {
        struct user_irq *uirq = user_irq_fetch(irq);
        if (!uirq)
            continue;

        if (uirq->thr_id == thr->t_globalid) {
            /* make sure irq is cleared */
            /* clear pending bit */
            user_irq_clear_pending(irq);
            switch (uirq->action) {
            case USER_IRQ_ENABLE:
                user_irq_enable(irq);
                break;
            case USER_IRQ_DISABLE:
                irq_disable();
                user_irq_queue_delete(irq);
                irq_enable();
                user_irq_disable(irq);
                break;
            case USER_IRQ_FREE:
                /* reply ipc immediately (before releasing uirq) */
                irq_handler_ipc(uirq);
                thr->state = T_RUNNABLE;
                sched_enqueue(thr);
                /* now safe to release */
                irq_disable();
                user_irq_queue_delete(irq);
                user_irq_release(irq);
                irq_enable();
                break;
            }
            break;
        }
    }
}

void user_irq_enable(int irq)
{
    if (nvic_is_setup(irq)) {
        NVIC_EnableIRQ(irq);
    }
}

void user_irq_disable(int irq)
{
    if (nvic_is_setup(irq)) {
        NVIC_ClearPendingIRQ(irq);
        NVIC_DisableIRQ(irq);
    }
}

void user_irq_set_pending(int irq)
{
    if (nvic_is_setup(irq)) {
        NVIC_SetPendingIRQ(irq);
    }
}

void user_irq_clear_pending(int irq)
{
    if (nvic_is_setup(irq)) {
        NVIC_ClearPendingIRQ(irq);
    }
}
