/* Copyright (c) 2015 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <user_interrupt.h>
#include <interrupt_ipc.h>
#include <l4io.h>
#include <l4/ipc.h>
#include <l4/pager.h>
#include <thread.h>

__USER_TEXT
static void *__interrupt_handler_thread(void *arg)
{
	do {
		L4_Msg_t msg;
		L4_MsgTag_t tag;
		irq_handler_t handler;
		L4_Word_t action;
		L4_ThreadId_t recv_tid = {
			.raw = TID_TO_GLOBALID(THREAD_INTERRUPT)};

		tag = L4_Receive(recv_tid);

		L4_MsgStore(tag, &msg);
		handler = (irq_handler_t)L4_MsgWord(&msg, IRQ_IPC_HANDLER);
		action = L4_MsgWord(&msg, IRQ_IPC_ACTION);

		switch (action) {
		case USER_IRQ_ENABLE:
			handler();
			break;
		case USER_IRQ_FREE:
			return NULL;
		}
	} while(1);

}

__USER_TEXT
static void __irq_msg(
	L4_Msg_t *out_msg, L4_ThreadId_t tid,
	unsigned int irq, irq_handler_t handler,
	L4_Word_t action, uint16_t priority)
{
	L4_Word_t irq_data[IRQ_IPC_MSG_NUM];

	L4_MsgClear(out_msg);

	irq_data[IRQ_IPC_IRQN] = (L4_Word_t)irq;
	irq_data[IRQ_IPC_TID] = (L4_Word_t)tid.raw;
	irq_data[IRQ_IPC_HANDLER] = (L4_Word_t)handler;
	irq_data[IRQ_IPC_ACTION] = (L4_Word_t)action;
	irq_data[IRQ_IPC_PRIORITY] = (L4_Word_t)priority;

	/* Create msg for irq request */
	L4_MsgPut(out_msg, USER_INTERRUPT_LABEL,
              IRQ_IPC_MSG_NUM, irq_data, 0, NULL);
}

__USER_TEXT
static L4_Word_t __request_irq(L4_Msg_t *msg)
{
	L4_MsgTag_t ret;
	L4_ThreadId_t send_tid = {
		.raw = TID_TO_GLOBALID(THREAD_IRQ_REQUEST)};

	/* Load msg to registers */
	L4_MsgLoad(msg);

	/* register irq in kernel */
	ret = L4_Send(send_tid);

	return ret.raw;
}

__USER_TEXT
L4_Word_t request_irq(int irq, irq_handler_t handler, uint16_t priority)
{
	L4_Msg_t msg;
	L4_ThreadId_t tid;

	/* Create thread for interrupt handler */
	tid = pager_create_thread();
	pager_start_thread(tid, __interrupt_handler_thread, NULL);
	__irq_msg(&msg, tid, irq, handler, USER_IRQ_ENABLE, priority);

	return __request_irq(&msg);
}

__USER_TEXT
L4_Word_t enable_irq(int irq)
{
	L4_Msg_t msg;

	__irq_msg(&msg, L4_nilthread, irq, NULL, USER_IRQ_ENABLE, -1);

	return __request_irq(&msg);
}

__USER_TEXT
L4_Word_t disable_irq(int irq)
{
	L4_Msg_t msg;

	__irq_msg(&msg, L4_nilthread, irq, NULL, USER_IRQ_DISABLE, -1);

	return __request_irq(&msg);
}

__USER_TEXT
L4_Word_t free_irq(int irq)
{
	L4_Msg_t msg;

	__irq_msg(&msg, L4_nilthread, irq, NULL, USER_IRQ_FREE, -1);

	return __request_irq(&msg);
}

