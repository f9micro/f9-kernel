#include <libposix/pthread.h>
#include <l4/ipc.h>
#include <l4/utcb.h>
#include <l4io.h>
#include <platform/link.h>

#define STACK_SIZE 256
#define MAX_THREAD 128

#define ROOT_THREAD 	1
#define USER_ROOT	2
#define USER_THREAD	3

/* pthread tree architecture, e.g
 *   +------+
 *   | Root |
 *   +------+
 *     |  ^
 *     v  |
 *   +-------+    +-------+    +-------+
 *   | User1 |<-->| User2 |<-->| User3 |
 *   +-------+    +-------+    +-------+
 *     |  ^                      | ^
 *     v  |                      v |
 *   +--------+    +--------+  +--------+
 *   | Child1 |<-->| Child2 |  | Child1 +
 *   +--------+    +--------+  +--------+
 *
 * Every user will have a free_mem address.
 * When nested pthread_create() occurs, they will use
 * the free_mem address in their user's free_mem.
 */    
typedef struct pthread_i {
	L4_ThreadId_t tid;
	L4_Word_t free_mem;
	struct pthread_i *child;
	struct pthread_i *parent;
	struct pthread_i *next;
	struct pthread_i *prev;
	int index;
	int type;
	int count_thread;
} pthread_i;

static __USER_DATA pthread_i *root;
static __USER_DATA pthread_i parray[MAX_THREAD];

// TODO: use bitmap to trace allocate state
__USER_DATA uint8_t flag[MAX_THREAD];

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

__USER_TEXT static void _pthread_dump(void)
{
	int i;

	for(i = 0; i < MAX_THREAD; i++) {
		if(flag[i]) {
			printf("%d: %#x, parent:%p, child:%p, next:%p, prev:%p\n"
			       , i, parray[i].tid.raw, parray[i].parent,
			       parray[i].child, parray[i].next, parray[i].prev);
		}
	}
}

void __USER_TEXT _pthread_init(void)
{
	int i;

	for(i = 1; i < MAX_THREAD; i++) {
		flag[i] = 0;
		parray[i].index = i;
	}

	flag[0] = 1;
	root = &parray[0];
	root->tid.raw = 0x8000; // ref to include/thread.h
	root->child = NULL;
	root->parent = NULL;
	root->next = NULL;
	root->prev = NULL;
	root->index = 0;
	root->type = ROOT_THREAD;
	root->count_thread = 0;
}

__USER_TEXT static int _get_free_index(void)
{
	int i;

	for(i = 1; i < MAX_THREAD; i++) {
		if(!flag[i]) {
			flag[i] = 1;
			return i;
		}
	}

	return -1;
}

__USER_TEXT static pthread_i *_find_user_root(void)
{
	int i;
	L4_ThreadId_t myself = L4_MyGlobalId();

	for(i = 0; i < MAX_THREAD; i++) {
		if(flag[i] && parray[i].tid.raw == myself.raw) {
			break;
		}
	}

	if(parray[i].type == USER_ROOT) {
		return &parray[i];
	}
	else if(parray[i].type == USER_THREAD) {
		pthread_i *it;
		for(it = &parray[i]; it->type != USER_ROOT; it = it->parent);
		return it;
	}

	/* bug reach here */
	printf("_find_user_root bug\n");

	return NULL;
}

/*
__USER_TEXT static void _pthread_remove(pthread_i *node)
{

}
*/

__USER_TEXT static void _pthread_insert(pthread_i *child)
{
	int i;
	L4_ThreadId_t myself = L4_MyGlobalId();

	for(i = 0; i < MAX_THREAD; i++) {
		if(flag[i] && parray[i].tid.raw == myself.raw) {
			break;
		}

		if(i == MAX_THREAD - 1) {
			/* bug reach here */
			printf("pthread: pthread_insert bug\n");
			return;
		}
	}

	if(parray[i].child == NULL) {
		parray[i].child = child;
		child->parent = &parray[i];
	}
	else {
		pthread_i *it;

		for(it = parray[i].child; it->next != NULL; it = it->next);
		it->next = child;
		child->prev = it;
	}

	_pthread_dump();
}

void __USER_TEXT _pthread_add_user(L4_ThreadId_t tid, L4_Word_t free_mem)
{
	int index = _get_free_index();

	parray[index].tid.raw = tid.raw;
	parray[index].free_mem = free_mem;
	parray[index].child = NULL;
	parray[index].parent = NULL;
	parray[index].next = NULL;
	parray[index].prev = NULL;
	parray[index].index = index;
	parray[index].type = USER_ROOT;
	parray[index].count_thread = 0;

	_pthread_insert(&parray[index]);
}

int __USER_TEXT pthread_create(pthread_t *restrict thread,
                               const pthread_attr_t *restrict attr,
                               void *(*start_routine)(void*), void *restrict arg)
{
	int index = _get_free_index();
	L4_ThreadId_t myself = L4_MyGlobalId();
	L4_ThreadId_t child;
	pthread_i *user_root = _find_user_root();

	child.raw = myself.raw + (++parray[index].count_thread << 14);

	/* Init new pthread_i node */
	parray[index].tid = child;
	parray[index].index = index;
	parray[index].type = USER_THREAD;
	parray[index].count_thread = 0;

	_pthread_insert(&parray[index]);

	/* TODO: Handle attr */
	if(!attr) {
	}

	/* TODO: pthread_sigmask, sigpending, sigaltstack */
	/* TODO: fenv */
	/* TODO: Initilize cpu clock to 0(pthread_getcpuclockid) */

	// XXX: L4_ThreadControl may fail but can not check it here.
	L4_ThreadControl(child, myself, L4_nilthread, myself, (void *)user_root->free_mem);
	user_root->free_mem += UTCB_SIZE + STACK_SIZE;

	start_thread(child, (L4_Word_t)start_routine, user_root->free_mem, STACK_SIZE);

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
	/* Busy trying */
	while(pthread_mutex_trylock(mutex));

	return 0;
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

	return result;
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
