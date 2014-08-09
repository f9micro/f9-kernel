#include <libposix/pthread.h>
#include <l4/ipc.h>
#include <l4/utcb.h>
#include <l4io.h>
#include <platform/link.h>

#define STACK_SIZE 256
#define MAX_THREAD 128

struct pthread_i {
	L4_ThreadId_t tid;
	L4_Word_t *free_mem;
	int last_thread;
};

__USER_DATA struct pthread_i parray[MAX_THREAD];
__USER_DATA int current_thread = 0;

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


__USER_TEXT void _pthread_create(L4_ThreadId_t tid, L4_Word_t *free_mem)
{
	parray[current_thread].tid = tid;
	parray[current_thread].free_mem = free_mem;
	parray[current_thread].last_thread = 0;
	current_thread++;
}

__USER_TEXT int pthread_create(pthread_t *restrict thread,
                               const pthread_attr_t *restrict attr,
                               void *(*start_routine)(void*), void *restrict arg)
{
	int index;
	L4_ThreadId_t myself = L4_MyGlobalId();
	L4_ThreadId_t child;

	for(index = 0; index < current_thread; index++) {
		if(parray[index].tid.raw == myself.raw) {
			break;
		}

		if(index == current_thread - 1) {
			return -1;
		}
	}

	child.raw = myself.raw + (++parray[index].last_thread << 14);

	/* Add new pthread */
	parray[current_thread].tid = child;
	parray[current_thread].free_mem = parray[index].free_mem;
	parray[current_thread].last_thread = 0;
	current_thread++;

	/* TODO: Handle attr */
	if(!attr) {
	}

	/* TODO: pthread_sigmask, sigpending, sigaltstack */
	/* TODO: fenv */
	/* TODO: Initilize cpu clock to 0(pthread_getcpuclockid) */
	/* TODO: Update thread structure, fill in new thread id */

	L4_ThreadControl(child, myself, L4_nilthread, myself, (void *)*parray[index].free_mem);
	*parray[index].free_mem += UTCB_SIZE + STACK_SIZE;

	start_thread(child, (L4_Word_t)start_routine, *parray[index].free_mem, STACK_SIZE);

	/* TODO: Error tag return */
	return 0;
}

__USER_TEXT int pthread_detach(pthread_t thread)
{
	return 0;	
}

__USER_TEXT void pthread_exit(void *value_ptr) 
{

}

__USER_TEXT int pthread_join(pthread_t thread, void **value_ptr)
{
	return 0;
}

__USER_TEXT int pthread_mutex_init(pthread_mutex_t *mutex,
		                   const pthread_mutexattr_t *restrict attr)
{
	return 0;
}

__USER_TEXT int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	return 0;
}

__USER_TEXT int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	register int result = 1;

	/* Busy trying */
	while(result) {
		__asm__ __volatile__(
		    "mov r1, #1\n"
		    "mov r2, %[mutex]\n"
		    "ldrex r0, [r2]\n"	/* Load value [r2] */
		    "cmp r0, #0\n"	/* Checking is word set to 1 */

		    "itt eq\n"
		    "strexeq r0, r1, [r2]\n"
		    "moveq %[result], r0\n"
		    : [result] "=r"(result)
		    : [mutex] "r"(mutex)
		    : "r0", "r1", "r2");
	}

	return result == 0;
}

__USER_TEXT int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	register int result = 1;

	__asm__ __volatile__(
	    "mov r1, #1\n"
	    "mov r2, %[mutex]\n"
	    "ldrex r0, [r2]\n"	/* Load value [r2] */
	    "cmp r0, #0\n"	/* Checking is word set to 1 */

	    "itt eq\n"
	    "strexeq r0, r1, [r2]\n"
	    "moveq %[result], r0\n"
	    : [result] "=r"(result)
	    : [mutex] "r"(mutex)
	    : "r0", "r1", "r2");

	return result == 0;
}

__USER_TEXT int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	*mutex = 0;
	return 0;
}

__USER_TEXT int pthread_mutex_timedlock(pthread_mutex_t *restrict mutex,
                                        const struct timespec *restrict abstime)
{
	return 0;
}
