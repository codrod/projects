#include <cxrt.h>

cxdef_thread_local cxrt_t CXRUNTIME;

cxexception_t CXException_Signal = {"CXException_Signal", "", 0};
cxexception_t CXException_Signal_SegFault = {"CXException_Signal_SegFault", "segmentation fault", CXERROR_SIGNAL};
cxexception_t CXException_Signal_FloatingPointError = {"CXException_Signal_FloatingPointError", "floating point error", CXERROR_SIGNAL};
cxexception_t CXException_Signal_IllegalInstruction = {"CXException_Signal_IllegalInstruction", "illegal instruction", CXERROR_SIGNAL};
cxexception_t CXException_Signal_BusError = {"CXException_Signal_BusError", "bus error", CXERROR_SIGNAL};

void CXRT_SIGNALS()
{
	struct sigaction action;

	action.sa_flags = SA_SIGINFO | SA_NODEFER;
    action.sa_sigaction = cxrt_signal_handler;
    sigemptyset(&action.sa_mask);

    if(sigaction(SIGSEGV, &action, NULL) == -1)
        cxrt_exit("CXRT:ERROR: CXRT_MAIN failed replace default signal handler\n", CXERROR_SIGNAL);

	if(sigaction(SIGFPE, &action, NULL) == -1)
        cxrt_exit("CXRT:ERROR: CXRT_MAIN failed replace default signal handler\n", CXERROR_SIGNAL);

	if(sigaction(SIGILL, &action, NULL) == -1)
        cxrt_exit("CXRT:ERROR: CXRT_MAIN failed replace default signal handler\n", CXERROR_SIGNAL);

	if(sigaction(SIGBUS, &action, NULL) == -1)
        cxrt_exit("CXRT:ERROR: CXRT_MAIN failed replace default signal handler\n", CXERROR_SIGNAL);

	return;
}

void CXRT_MAIN()
{
	if(cxrt_construct(&CXRUNTIME))
		cxrt_exit("CXRT:ERROR: CXRT_MAIN failed to construct runtime\n", CXERROR_MAIN);

	CXRT_FUNC();
	CXRT_SIGNALS();

	if(cxmem_storage_construct(&CXMEM_STATIC))
		cxrt_exit("CXRT:ERROR: CXRT_MAIN failed to construct cxstatic\n", CXERROR_MAIN);

	if(!(CXMEM_STATIC.mutex = malloc(sizeof(cxrt_mutex_t))))
		cxrt_exit("CXRT:ERROR: CXRT_MAIN failed to construct cxstatic's mutex\n", CXERROR_MAIN);

	if(cxrt_mutex_construct(CXMEM_STATIC.mutex))
		cxrt_exit("CXRT:ERROR: CXRT_MAIN failed to construct cxstatic's mutex\n", CXERROR_MAIN);

	CXRUNTIME.flags |= CXRT_FLAG_MAIN;

	return;
}

void CXRT_THREAD()
{
	if(cxrt_construct(&CXRUNTIME))
		cxrt_exit("CXRT:ERROR: CXRT_THREAD failed to construct runtime\n", CXERROR_THREAD);

	CXRT_FUNC();
	CXRT_SIGNALS();

	CXRUNTIME.flags |= CXRT_FLAG_THREAD;

	return;
}

void CXRT_FUNC()
{
	cxrt_func_t func;

	if(cxrt_func_construct(&func))
		cxrt_exit("CXRT:ERROR: CXRT_FUNC failed to construct function\n", CXERROR_FUNC);

	if(cxrt_stack_push(&CXRUNTIME.stack, &func))
		cxrt_exit("CXRT:ERROR: CXRT_FUNC failed to push onto stack\n", CXERROR_FUNC);

	return;
}

void CXRT_RETURN()
{
	cxrt_func_t *func;

	if(CXRUNTIME.stack.size == 1)
	{
		if(cxmem_storage_destruct(&CXRUNTIME.storage))
			cxrt_exit("CXRT:ERROR: CXRT_RETURN failed to destruct thread local storage\n", CXERROR_RETURN);

		if(CXRUNTIME.flags & CXRT_FLAG_MAIN)
			if(cxmem_storage_destruct(&CXMEM_STATIC))
				cxrt_exit("CXRT:ERROR: CXRT_RETURN failed to destruct static storage\n", CXERROR_RETURN);
	}

	if(cxrt_stack_peak(&CXRUNTIME.stack, (cxaddress_t*)&func))
		cxrt_exit("CXRT:ERROR: CXRT_RETURN failed to pop off stack\n", CXERROR_RETURN);

	//the function should be destructed on the stack
	if(cxrt_func_destruct(func))
		cxrt_exit("CXRT:ERROR: CXRT_RETURN failed to destruct function\n", CXERROR_RETURN);

	if(cxrt_stack_pop(&CXRUNTIME.stack, NULL))
		cxrt_exit("CXRT:ERROR: CXRT_RETURN failed to pop off stack\n", CXERROR_RETURN);

	if(!CXRUNTIME.stack.size)
	{
		if(cxrt_destruct(&CXRUNTIME))
			cxrt_exit("CXRT:ERROR: CXRT_RETURN failed to destruct runtime\n", CXERROR_RETURN);
	}

	return;
}

cxmem_storage_t* CXRT_AUTO()
{
	cxrt_func_t *func = NULL;
	int ret = 0;

	if((ret = cxrt_stack_peak(&CXRUNTIME.stack, (cxaddress_t*)&func)))
		cxrt_exit("CXRT:ERROR: CXRT_AUTO failed to peak auto storage\n", ret);

	return &func->storage;
}

void cxrt_signal_handler(int signal, siginfo_t *info, void *context)
{
	if(signal == SIGSEGV)
		cxthrow(CXException_Signal_SegFault);
	else if(signal == SIGFPE)
		cxthrow(CXException_Signal_FloatingPointError);
	else if(signal == SIGILL)
		cxthrow(CXException_Signal_IllegalInstruction);
	else if(signal == SIGBUS)
		cxthrow(CXException_Signal_BusError);

	return;
}

void cxrt_exit(char *str, int ret)
{
	fprintf(stderr, "%s", str);
	exit(ret);
}

int cxrt_construct(cxrt_t *runtime)
{
	int ret = 0;

	runtime->flags = 0;

	if(ret = cxrt_stack_construct(&runtime->stack, sizeof(cxrt_func_t), &cxrt_func_construct, &cxrt_func_destruct))
		return ret;

	if(ret = cxmem_storage_construct(&runtime->storage))
		return ret;

	return 0;
}

int cxrt_destruct(cxrt_t *runtime)
{
	int ret = 0;

	if(ret = cxrt_stack_destruct(&runtime->stack))
		return ret;

	return 0;
}

int cxrt_func_construct(cxaddress_t addr)
{
	cxrt_func_t *func = addr;
	int ret = 0;

	if((ret = cxrt_stack_construct(&func->try_blocks, sizeof(cxex_try_block_t), NULL, NULL)))
		return ret;

	if((ret = cxmem_storage_construct(&func->storage)))
		return ret;

	func->flags = 0;

	return 0;
}

int cxrt_func_destruct(cxaddress_t addr)
{
	cxrt_func_t *func = addr;
	int ret = 0;

	//destruct storage first
	if(ret = cxmem_storage_destruct(&func->storage))
		return ret;

	if(ret = cxrt_stack_destruct(&func->try_blocks))
		return ret;

	return ret;
}
