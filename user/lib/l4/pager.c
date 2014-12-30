/* Copyright (c) 2014 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4io.h>
#include <l4/ipc.h>
#include <l4/utcb.h>
#include <l4/pager.h>
#include <l4/kip_types.h>

#define STACK_SIZE 0x200

typedef void *thr_handler_t(void *);

struct thread_node {
	L4_Word_t base;			/* stack + utcb */
	L4_ThreadId_t tid;
};

static inline void use_thread_node(struct thread_node *node)
{
	node->base |= 0x1;
}

static inline void free_thread_node(struct thread_node *node)
{
	node->base &= ~0x1;
}

#define IS_THREAD_NODE_USED(node) \
	((node)->base & 0x1)

#define THREAD_NODE_BASE(node) \
	((node)->base & ~0x1)

struct thread_pool {
	struct thread_node *all_nodes;
	L4_Word_t node_num;
	L4_ThreadId_t pager_tid;
};

static inline int find_thread_index(struct thread_pool *pool, L4_ThreadId_t tid)
{
	return ((tid.raw - (pool->pager_tid).raw) >> 14) - 1;
}

static inline L4_Word_t user_fpage_number(user_fpage_t *fpages)
{
	int num = 0;

	while (fpages[num].base != 0 || fpages[num].size != 0)
		num++;

	return num;
}

__USER_TEXT
static struct thread_pool *init_thread_pool(L4_Word_t res_base,
        L4_Word_t res_size,
        L4_Word_t heap_base,
        L4_Word_t heap_size)
{
	L4_Word_t node_num, i;
	L4_Word_t num1, num2;
	struct thread_pool *pool;
	struct thread_node *nodes;

	num1 = (heap_size - sizeof(struct thread_pool)) / sizeof(struct thread_node);
	num2 = (res_size) / (UTCB_SIZE + STACK_SIZE);

	node_num = (num1 < num2) ? num1 : num2;
	node_num = (node_num > THREAD_MAX_NUM) ? THREAD_MAX_NUM : node_num;

	pool = (struct thread_pool *)heap_base;
	nodes = (struct thread_node *)(heap_base + sizeof(struct thread_pool));

	pool->node_num = node_num;
	pool->all_nodes = nodes;
	pool->pager_tid = L4_MyGlobalId();

	if (res_base & 0x1) {
		printf("unalign res base\n");
		return NULL;
	}

	for (i = 1 ; i < node_num ; i++) {
		nodes[i].base = res_base;
		nodes[i].tid.raw = 0;
		res_base += (UTCB_SIZE + STACK_SIZE);
	}

	return pool;
}

__USER_TEXT
static L4_Word_t fetch_free_thread_index(struct thread_pool *pool)
{
	int i;
	struct thread_node *node;

	for (i = 1 ; i < pool->node_num ; i++) {
		node = &pool->all_nodes[i];
		if (!IS_THREAD_NODE_USED(node))
			break;
	}

	if (i == pool->node_num)
		return 0;

	return i;
}

__USER_TEXT
static struct thread_node *find_thread_node(struct thread_pool *pool,
        L4_ThreadId_t tid)
{
	int i;

	for (i = 1 ; i < pool->node_num ; i++) {
		if (pool->all_nodes[i].tid.raw == tid.raw &&
		    IS_THREAD_NODE_USED(&pool->all_nodes[i])) {
			return &pool->all_nodes[i];
		}
	}

	return NULL;
}

__USER_TEXT
static void release_thread(struct thread_pool *pool, L4_ThreadId_t tid)
{
	L4_Word_t idx;

	idx = find_thread_index(pool, tid);

	if (idx >= pool->node_num) {
		printf("tid %p not found.\n", tid);
		return;
	}

	free_thread_node(&pool->all_nodes[idx]);
	L4_ThreadControl(tid, L4_nilthread, L4_nilthread,
	                 L4_nilthread, (void *) - 1);

}

__USER_TEXT
void thread_container(kip_t *kip_ptr, utcb_t *utcb_ptr,
                      L4_Word_t entry, L4_Word_t entry_arg)
{
	L4_Msg_t msg;
	((thr_handler_t *)entry)((void *)entry_arg);

	L4_MsgClear(&msg);
	L4_Set_MsgLabel(&msg, PAGER_REQUEST_LABEL);
	L4_MsgAppendWord(&msg, THREAD_FREE);
	L4_MsgLoad(&msg);
	L4_Call(L4_Pager());
}

__USER_TEXT
static void start_thread(L4_ThreadId_t t,
                         L4_Word_t entry, L4_Word_t entry_arg,
                         L4_Word_t sp, L4_Word_t stack_size)
{
	L4_Msg_t msg;

	L4_MsgClear(&msg);
	L4_MsgAppendWord(&msg, (L4_Word_t)thread_container);
	L4_MsgAppendWord(&msg, sp);
	L4_MsgAppendWord(&msg, stack_size);
	L4_MsgAppendWord(&msg, entry);
	L4_MsgAppendWord(&msg, entry_arg);
	L4_MsgLoad(&msg);

	L4_Send(t);
}

__USER_TEXT
static L4_ThreadId_t __thread_create(struct thread_pool *pool)
{
	L4_ThreadId_t child, myself;
	L4_Word_t thr_idx;
	L4_Word_t free_mem;
	struct thread_node *node;

	thr_idx = fetch_free_thread_index(pool);
	if (thr_idx == 0) {
		printf("No free res for new thread\n");
		child.raw = 0;
		return child;
	}

	child.raw = (pool->pager_tid).raw + (thr_idx << 14);
	node = &pool->all_nodes[thr_idx];
	node->tid = child;
	use_thread_node(node);

	myself = L4_MyGlobalId();
	free_mem = (L4_Word_t)THREAD_NODE_BASE(node);

	L4_ThreadControl(child, myself, L4_nilthread, myself, (void *)free_mem);

	return child;
}

__USER_TEXT
static L4_Word_t __thread_start(struct thread_pool *pool, L4_ThreadId_t tid,
                                L4_Word_t entry,
                                L4_Word_t entry_arg)
{
	struct thread_node *node;
	L4_Word_t stack;

	node = find_thread_node(pool, tid);

	if (node == NULL)
		return (L4_Word_t) - 1;

	stack = (L4_Word_t)THREAD_NODE_BASE(node) + UTCB_SIZE + STACK_SIZE;
	start_thread(tid, entry, entry_arg, stack, STACK_SIZE);

	return 0;
}

__USER_TEXT
L4_ThreadId_t pager_create_thread(void)
{
	L4_ThreadId_t tid;
	L4_Msg_t msg;
	L4_MsgTag_t tag;

	L4_MsgClear(&msg);
	L4_Set_Label(&msg.tag, PAGER_REQUEST_LABEL);
	L4_MsgAppendWord(&msg, THREAD_CREATE);

	L4_MsgLoad(&msg);
	tag = L4_Call(L4_Pager());

	if (L4_Label(tag) == PAGER_REPLY_LABEL)
		L4_StoreMR(1, &tid.raw);
	else
		tid.raw = 0;

	return tid;
}

__USER_TEXT
L4_Word_t pager_start_thread(L4_ThreadId_t tid, void * (*thr_routine)(void *),
                             void *arg)
{
	L4_Msg_t msg;
	L4_MsgTag_t tag;
	L4_Word_t ret;

	L4_MsgClear(&msg);
	L4_Set_Label(&msg.tag, PAGER_REQUEST_LABEL);
	L4_MsgAppendWord(&msg, THREAD_START);
	L4_MsgAppendWord(&msg, (L4_Word_t)tid.raw);
	L4_MsgAppendWord(&msg, (L4_Word_t)thr_routine);
	/* TODO: Ignore arg now */

	L4_MsgLoad(&msg);
	tag = L4_Call(L4_Pager());

	if (L4_Label(tag) == PAGER_REPLY_LABEL) {
		L4_StoreMR(1, &ret);
	} else {
		ret = -1;
	}

	return ret;
}

__USER_TEXT
void pager_thread(user_struct *user,
                  void * (*entry_main)(void *))
{
	L4_Word_t fpage_num;
	L4_ThreadId_t main_tid;
	struct thread_pool *pool;

	fpage_num = user_fpage_number(user->fpages);

	if (fpage_num < 2) {
		/* TODO: fpage 0: stack + tcb, fpage 1: heap */
		printf("pager: There is no enough fpages\n");
		return;
	}

	pool = init_thread_pool(user->fpages[RES_FPAGE].base,
	                        user->fpages[RES_FPAGE].size,
	                        user->fpages[HEAP_FPAGE].base,
	                        user->fpages[HEAP_FPAGE].size);

	/* Create main entry thread */
	main_tid = __thread_create(pool);
	__thread_start(pool, main_tid, (L4_Word_t)entry_main, 0);

	/* Handle ipc request */
	while (1) {
		L4_Msg_t msg;
		L4_ThreadId_t request_tid;
		L4_MsgTag_t tag;
		L4_Word_t req;

		tag = L4_Wait(&request_tid);
		if (!L4_IpcSucceeded(tag)) {			/* TODO: error message */
			printf("Invalid tag\n");
			continue;
		}

		if (L4_Label(tag) != PAGER_REQUEST_LABEL) {
			printf("Invalid label = %p\n", L4_Label(tag));
			continue;
		}

		L4_MsgStore(tag, &msg);
		req = L4_MsgWord(&msg, 0);

		switch (req) {
		case THREAD_CREATE: {
				L4_ThreadId_t tid;
				tid = __thread_create(pool);

				/* return tid back */
				L4_MsgClear(&msg);
				L4_Set_Label(&msg.tag, PAGER_REPLY_LABEL);
				L4_MsgAppendWord(&msg, tid.raw);
				L4_MsgLoad(&msg);
				L4_Send(request_tid);
			}
			break;
		case THREAD_START: {
				L4_Word_t entry;
				L4_ThreadId_t tid;
				L4_Word_t ret;

				tid.raw = L4_MsgWord(&msg, 1);
				entry = L4_MsgWord(&msg, 2);
				/* TODO : ignore entry argument now */

				ret = __thread_start(pool, tid, entry, 0);

				L4_MsgClear(&msg);
				L4_Set_Label(&msg.tag, PAGER_REPLY_LABEL);
				L4_MsgAppendWord(&msg, ret);
				L4_MsgLoad(&msg);
				L4_Send(request_tid);
			}
			break;
		case THREAD_FREE:
			release_thread(pool, request_tid);
			break;
		case THREAD_WAIT:
			break;
		}
	}
}
