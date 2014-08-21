#include <user_runtime.h>
#include <platform/link.h>
#include <l4/utcb.h>
#include <l4/ipc.h>
#include <l4io.h>

/* posix layer */
#include <libposix/pthread.h>

#define STACK_SIZE 256

__USER_DATA pthread_mutex_t mutex;

__USER_DATA int shared = 0;

__USER_TEXT void *child_thread2(void *args)
{
	printf("child task 2 start\n");

	while(1) {
		shared = shared + 1;
		printf("task 2: %d\n", shared);
		L4_Sleep(L4_TimePeriod(1000));
	}

	return 0;
}

__USER_TEXT void *child_thread1(void *args)
{
	printf("child task 1 start\n");
	pthread_create(NULL, NULL, child_thread2, NULL);

	for(int i = 0; i <= 10; i++) {
		printf("%d\n", 10 - i);
		L4_Sleep(L4_TimePeriod(500));
	}

	printf("task 1: suicide\n");
	pthread_exit(0);

	for(int i = 0; i <= 10; i++) {
		printf("%d\n", 10 - i);
		L4_Sleep(L4_TimePeriod(500));
	}

	while(1) {
		L4_Sleep(L4_Never);
	}

	return 0;
}

static __USER_TEXT void main(user_struct *user)
{
	printf("\nPosix Layer test starts\n");
	mutex = 0;
	pthread_create(NULL, NULL, child_thread1, NULL);

	while(1)
		L4_Sleep(L4_Never);

	return;
}

DECLARE_USER(
	255,
	posixtest,
	main,
	DECLARE_FPAGE(0x0, 4 * (UTCB_SIZE + STACK_SIZE))
);
