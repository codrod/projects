/*! \file cxdef.h
    \brief Contains defs for basic types used by the entire library
*/

#if !CXDEF_VERSION

	#define CXDEF_VERSION 1000000L

	#include <stddef.h>
	#include <stdint.h>
	#include <limits.h>

	#if __STDC_VERSION__ >= 201100L && !__STDC_NO_THREADS__

		#include <threads.h>

		#define cxdef_thread_local thread_local

	#elif _MSC_VER

		#define cxdef_thread_local __declspec( thread )

	#else

		#define cxdef_thread_local __thread

	#endif

	#define CX_SIZE_MAX LLONG_MAX
	#define CX_SIZE_MIN LLONG_MIN
	#define CX_USIZE_MAX ULLONG_MAX

	#define CX_UNUSED(var) (void)var

	typedef uint8_t cxbyte_t;
	typedef long long int cxsize_t;
	typedef unsigned long long int cxusize_t;
	typedef void* cxaddress_t;

#endif
