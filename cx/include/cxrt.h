#if !CXRT_VERSION

	#define CXRT_VERSION 1000001L
	#define _XOPEN_SOURCE_EXTENDED

	#include <stdio.h>
	#include <stdint.h>
	#include <stdlib.h>
	#include <string.h>
	#include <errno.h>
	#include <setjmp.h>
	#include <signal.h>

	#include <cxerror.h>
	#include <cxex.h>
	#include <cxmem.h>
	#include <cxrt_mutex.h>

	#define CXRT_STACK_DEFAULT_CAP 4
	#define CXRT_FUNC_FLAG_TRY 1
	#define CXRT_FLAG_MAIN 1
	#define CXRT_FLAG_THREAD 2

	#define __cxmain__ CXRT_MAIN();
	#define __cxthread__ CXRT_THREAD();
	#define __cxfunc__ CXRT_FUNC();

	#define cxreturn return CXRT_RETURN(),
	#define cxreturn_void return CXRT_RETURN()

	#define cxauto CXRT_AUTO()

	/// @private
	typedef struct cxrt_stack
	{
		cxaddress_t addr;
		cxsize_t size;
		cxsize_t cap;

		cxsize_t size_of;
		int (*construct)(cxaddress_t entry);
		int (*destruct)(cxaddress_t entry);
	}
	cxrt_stack_t;

	/// @private
	typedef struct cxrt_func
	{
		uint64_t flags;

		cxrt_stack_t try_blocks;
		cxmem_storage_t storage;
	}
	cxrt_func_t;

	/// @private
	typedef struct cxrt
	{
		uint64_t flags;

		cxrt_stack_t stack;
		cxmem_storage_t storage;
	}
	cxrt_t;

	void CXRT_MAIN();
	void CXRT_FUNC();
	void CXRT_THREAD();
	void CXRT_RETURN();
	cxmem_storage_t* CXRT_AUTO();

	void cxrt_signal_handler(int signum, siginfo_t *info, void *context);
	void cxrt_exit(char *str, int ret);

	int cxrt_construct(cxrt_t *runtime);
	int cxrt_destruct(cxrt_t *runtime);

	int cxrt_func_construct(cxaddress_t func);
	int cxrt_func_destruct(cxaddress_t func);

	int cxrt_stack_construct(cxrt_stack_t *stack, cxsize_t size_of, int (*construct)(cxaddress_t), int (*destruct)(cxaddress_t));
	int cxrt_stack_destruct(cxrt_stack_t *stack);
	int cxrt_stack_push(cxrt_stack_t *stack, cxaddress_t entry);
	int cxrt_stack_peak(cxrt_stack_t *stack, cxaddress_t *entry);
	int cxrt_stack_double_peak(cxrt_stack_t *stack, cxaddress_t *entry);
	int cxrt_stack_pop(cxrt_stack_t *stack, cxaddress_t entry);
	int cxrt_stack_reverse_traversal(cxrt_stack_t *stack, int (*func)(cxaddress_t entry, cxaddress_t arg, cxaddress_t ret), cxaddress_t arg, cxaddress_t ret);

	extern cxdef_thread_local cxrt_t CXRUNTIME;

	extern cxexception_t CXException_Signal;
	extern cxexception_t CXException_Signal_SegFault;
	extern cxexception_t CXException_Signal_FloatingPointError;
	extern cxexception_t CXException_Signal_IllegalInstruction;
	extern cxexception_t CXException_Signal_BusError;

#endif
