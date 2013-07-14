#ifndef LIB_STDARG_H_
#define LIB_STDARG_H_

#if defined(__GNUC__)

typedef __builtin_va_list va_list;

#define va_start(v,l)	__builtin_va_start((v),l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v,l)	__builtin_va_arg((v),l)

#else

#error "stdarg.h is not implemented."

#endif

#endif	/* LIB_STDARG_H_ */
