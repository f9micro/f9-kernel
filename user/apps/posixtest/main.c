#include <user_runtime.h>
#include <platform/link.h>
#include <l4/utcb.h>
#include <l4io.h>

/* posix layer */
#include <libposix/libposix.h>

#define STACK_SIZE 256

static __USER_TEXT void main(user_struct *user)
{
	printf("\nPosix Layer test starts\n");
	fork();
	return;
}

DECLARE_USER(
	255,
	posixtest,
	main,
	DECLARE_FPAGE(0x0, 4 * (UTCB_SIZE + STACK_SIZE))
);
