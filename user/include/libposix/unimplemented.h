#ifndef __UNIMPLEMENTED_H__
#define __UNIMPLEMENTED_H__

#define ESC "\e["
#define LIGHT_RED    ESC "31;1m"
#define BLACK        ESC "0m"

#include <l4io.h>

#define UNIMPLEMENTED()						\
	do {							\
		printf("%s", LIGHT_RED);			\
		printf("\nUnimplemented: %s, at %s:%d",		\
		       __func__, __FILE__, __LINE__);		\
		printf("%s\n", BLACK);				\
	} while (0)						\

#endif
