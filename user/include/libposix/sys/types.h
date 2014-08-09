#ifndef __TYPES_H_
#define __TYPES_H_

#include <stdint.h>

/* FIXME: This should be moved to time.h */
struct timespec {
	uint64_t nsec;
};

/* FIXME: Define proper type for pthread type */
typedef uint32_t pthread_mutex_t;
typedef uint32_t pthread_mutexattr_t;
typedef uint32_t pthread_t;
typedef uint32_t pthread_attr_t;

#endif
