#include <thread.h>
#include <ipc.h>
#include <platform/irq.h>
#include <interrupt.h>
#include <interrupt_ipc.h>
#include <debug.h>
#include <init_hook.h>
#include <error.h>
#include <lib/ktable.h>

#include INC_PLAT(nvic.h)

void __interrupt_handler(int n);

/* user interrupt vector */
#define USER_IRQ_VEC(n)						\
	void nvic_handler##n(void) __NAKED;			\
	void nvic_handler##n(void)				\
	{							\
		irq_enter();					\
		__interrupt_handler(n);				\
		request_schedule();				\
		irq_return();					\
	}

#undef USER_INTERRUPT_USED
#define USER_INTERRUPT
#define IRQ_VEC_N_OP	USER_IRQ_VEC
#include INC_PLAT(nvic_private.h)
#undef IRQ_VEC_N_OP
#undef USER_INTERRUPT

typedef void (*irq_handler_t)(void);

#define INVALID_IDX		IRQn_NUM
#define INVALID_IRQ_NUM	IRQn_NUM

#define IS_VALID_IRQ_NUM(irq) ((irq) < INVALID_IRQ_NUM)

struct user_irq {
	tcb_t *thr;
	int irq;
	uint16_t action;
	uint16_t priority;
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
	return (user_irq_queue.head == NULL);
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

static struct user_irq *user_irq_queue_pop(void)
{
	if (user_irq_queue_is_empty())
		return NULL;

	struct user_irq *uirq = user_irq_queue.head;
	user_irq_queue.head = uirq->next;
	uirq->next = NULL;

	return uirq;
}

static void user_irq_queue_delete(int irq)
{
	struct user_irq *uirq = user_irqs[irq];
	for (struct user_irq **iter = &user_irq_queue.head ; *iter != NULL ;
	     iter = &(*iter)->next) {
		if (*iter == uirq) {
			*iter = uirq->next;
			break;
		}
	}
}

static inline void user_irq_reset_all(void)
{
	for (int i = 0 ; i < IRQn_NUM ; i++)
		user_irqs[i] = NULL;
}

static struct user_irq *user_irq_create_default(int irq)
{
	if (IS_VALID_IRQ_NUM(irq)) {
		struct user_irq *uirq = ktable_alloc(&user_irq_table);
		uirq->thr = NULL;
		uirq->irq = irq;
		uirq->action = 0;
		uirq->priority = 0;
		uirq->handler = NULL;
		uirq->next = NULL;

		return uirq;
	}
	return NULL;
}

static inline struct user_irq *user_irq_fetch(int irq)
{
	if (!user_irqs[irq])
		user_irqs[irq] =  user_irq_create_default(irq);
	return user_irqs[irq];
}

static void user_irq_release(int irq)
{
	if (IS_VALID_IRQ_NUM(irq)) {
		struct user_irq *uirq = user_irqs[irq];

		if (uirq != NULL) {
			ktable_free(&user_irq_table, uirq);
			user_irqs[irq] = NULL;
		}
	}
}

static void irq_handler_ipc(struct user_irq *uirq)
{
	if (uirq == NULL || uirq->thr == NULL)
		return;

	/* Prepare ipc for user interrupt thread */
	tcb_t *thr = uirq->thr;
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

static int irq_handler_enable(int irq)
{
	struct user_irq *uirq = user_irqs[irq];

	assert(uirq != NULL);

	if (uirq->thr == NULL)
		return -1;

	tcb_t *thr = uirq->thr;

	if (thr->state != T_RECV_BLOCKED)
		return -1;

	irq_handler_ipc(uirq);

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

static tcb_t *irq_handler_sched(struct sched_slot *slot)
{
	tcb_t *thr = NULL;

	irq_disable();
	struct user_irq *uirq = user_irq_queue_pop();

	if (uirq != NULL && (thr = uirq->thr) != NULL &&
	    thr->state == T_RECV_BLOCKED) {
		thr->state = T_RUNNABLE;
		sched_slot_dispatch(SSI_INTR_THREAD, thr);
	}

	irq_enable();

	return thr;
}

void __interrupt_handler(int irq)
{
	struct user_irq *uirq = user_irq_fetch(irq);

	if (uirq == NULL ||
	    uirq->thr == NULL ||
	    uirq->handler == NULL ||
	    uirq->action != USER_IRQ_ENABLE) {
		return;
	}

	user_irq_disable(irq); /* No re-entry interrupt */
	irq_schedule(irq);
}

void interrupt_init(void)
{
	user_irq_reset_all();
	sched_slot_set_handler(SSI_INTR_THREAD, irq_handler_sched);
}

INIT_HOOK(interrupt_init, INIT_LEVEL_KERNEL_EARLY);

void user_interrupt_config(tcb_t *from)
{
	ipc_msg_tag_t tag = { .raw = ipc_read_mr(from, 0) };
	if (tag.s.label != USER_INTERRUPT_LABEL)
		return;

	int irq = (uint16_t) from->ctx.regs[IRQ_IPC_IRQN + 1];
	l4_thread_t tid = (l4_thread_t) from->ctx.regs[IRQ_IPC_TID + 1];
	int action = (uint16_t) from->ctx.regs[IRQ_IPC_ACTION + 1];
	irq_handler_t handler = (irq_handler_t)
	                        from->ctx.regs[IRQ_IPC_HANDLER + 1];
	int priority = (uint16_t) from->ctx.regs[IRQ_IPC_PRIORITY + 1];

	user_irq_disable(irq);

	if (!IS_VALID_IRQ_NUM(irq))
		return;

	struct user_irq *uirq = user_irq_fetch(irq);

	if (uirq == NULL)
		return;

	/* update user irq config */
	if (tid != L4_NILTHREAD)
		uirq->thr = thread_by_globalid(tid);

	uirq->action = (uint16_t) action;

	if (handler != NULL)
		uirq->handler = handler;

	if (priority > 0)
		uirq->priority = (uint16_t)priority;
}

void user_interrupt_handler_update(tcb_t *thr)
{
	if (thr == NULL)
		return;

	for (int irq = 0 ; irq < IRQn_NUM ; irq++) {
		struct user_irq *uirq = user_irq_fetch(irq);
		if (uirq == NULL)
			continue;

		if (uirq->thr == thr) {
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
				irq_disable();
				user_irq_queue_delete(irq);
				user_irq_release(irq);
				irq_enable();
				/* reply ipc immediately */
				irq_handler_ipc(uirq);
				thr->state = T_RUNNABLE;
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
