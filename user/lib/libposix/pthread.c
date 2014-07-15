#include <libposix/pthread.h>
#include <l4/ipc.h>
#include <l4/utcb.h>
#include <platform/link.h>

#define STACK_SIZE 256

static void __USER_TEXT start_thread(L4_ThreadId_t t, L4_Word_t ip,
                                     L4_Word_t sp, L4_Word_t stack_size)
{
	L4_Msg_t msg;

	L4_MsgClear(&msg);
	L4_MsgAppendWord(&msg, ip);
	L4_MsgAppendWord(&msg, sp);
	L4_MsgAppendWord(&msg, stack_size);
	L4_MsgLoad(&msg);

	L4_Send(t);
}

__USER_TEXT int pthread_create(pthread_t *restrict thread,
                   const pthread_attr_t *restrict attr,
                   void *(*start_routine)(void*), void *restrict arg)
{
	/* FIXME: Get valid free memory space */
	L4_Word_t free_mem = NULL;
	L4_ThreadId_t myself = L4_MyGlobalId();
	L4_ThreadId_t child;

	/* FIXME: This should be pack into parent thread structure */
	static L4_Word_t last_thread;

	child.raw = myself.raw + (++last_thread << 14);

	/* TODO: Handle attr */
	if(!attr) {
	}

	/* TODO: pthread_sigmask, sigpending, sigaltstack */
	/* TODO: fenv */
	/* TODO: Initilize cpu clock to 0(pthread_getcpuclockid) */
	/* TODO: Update thread structure, fill in new thread id */

	L4_ThreadControl(child, myself, L4_nilthread, myself, (void *) free_mem);
	free_mem += UTCB_SIZE + STACK_SIZE;

	start_thread(child, (L4_Word_t)start_routine, free_mem, STACK_SIZE);

	/* TODO: Error tag return */
	return 0;
}

