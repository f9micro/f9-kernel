#include <libposix/libposix.h>
#include <libposix/unimplemented.h>
#include <platform/link.h>

int __USER_TEXT fork()
{
	UNIMPLEMENTED();
	return 0;
}
