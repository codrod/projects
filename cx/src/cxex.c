#include <cxrt.h>

#include <stdio.h>

cxdef_thread_local cxexception_t cxexcept;
cxexception_t CXException = {"CXException", "CX exception", 0};

jmp_buf* CXEX_TRY()
{
	cxrt_func_t *func;
	cxex_try_block_t block, *blockp;

	if(cxrt_stack_peak(&CXRUNTIME.stack, (cxaddress_t*)&func))
		cxrt_exit("CXRT:ERROR: CXEX_TRY failed to peak try-block stack\n", CXERROR_TRY);

	if(cxrt_stack_push(&func->try_blocks, &block))
		cxrt_exit("CXRT:ERROR: CXEX_TRY failed to push onto try-block stack\n", CXERROR_TRY);

	if(cxrt_stack_peak(&func->try_blocks, (cxaddress_t*)&blockp))
		cxrt_exit("CXRT:ERROR: CXEX_TRY failed to peak try-block stack\n", CXERROR_TRY);

	func->flags |= CXRT_FUNC_FLAG_TRY;

	return &blockp->jump;
}

void CXEX_END_TRY()
{
	cxrt_func_t *funcp;

	if(cxrt_stack_peak(&CXRUNTIME.stack, (cxaddress_t*)&funcp))
		cxrt_exit("CXRT:ERROR: CXEX_END_TRY failed to peak tlfunc\n", CXERROR_TRY);

	if(cxrt_stack_pop(&funcp->try_blocks, NULL))
		cxrt_exit("CXRT:ERROR: CXEX_END_TRY failed to pop jump buffer\n", CXERROR_TRY);

	if(!funcp->try_blocks.size)
		funcp->flags &= ~CXRT_FUNC_FLAG_TRY;

	return;
}

int CXEX_CATCH(int var, ...)
{
	CXEX_END_TRY();

	va_list args;
	va_start(args, var);
	cxexception_t other = {};
	int count = 0;

	for(; (other = va_arg(args, cxexception_t)).type; count++)
	{
		if(cxex_istype(cxexcept, other))
			return 1;
	}

	if(count) cxthrow(cxexcept);

	return 1;
}

jmp_buf* CXEX_THROW(cxexception_t ex, int ret)
{
	cxrt_func_t func, *funcp;
	cxex_try_block_t *block;
	int final_ret = (ret ? ret : ex.ret);
	int i = 0;

	for(; CXRUNTIME.stack.size;)
	{
		if(cxrt_stack_peak(&CXRUNTIME.stack, (cxaddress_t*)&funcp))
			cxrt_exit("CXRT:ERROR: CXEX_THROW failed to pop off runtime stack\n", CXERROR_THROW);

		//printf("%i %p %i\n", i++, funcp, funcp->flags & CXRT_FUNC_FLAG_TRY);

		if(funcp->flags & CXRT_FUNC_FLAG_TRY || CXRUNTIME.stack.size == 1)
			break;

		if(cxrt_stack_pop(&CXRUNTIME.stack, &func))
			cxrt_exit("CXRT:ERROR: CXEX_THROW failed to pop off runtime stack\n", CXERROR_THROW);

		if(cxrt_func_destruct(&func))
			cxrt_exit("CXRT:ERROR: CXEX_THROW failed to destruct func\n", CXERROR_THROW);
	}

	if(!(funcp->flags & CXRT_FUNC_FLAG_TRY))
	{
		//should be done atomicly
		fprintf(stderr, "CX:EXCEPTION: %s\nWHAT: %s\n", ex.type, ex.what);

		CXRT_RETURN();

		exit(final_ret);
	}

	if(cxrt_stack_peak(&funcp->try_blocks, (cxaddress_t*)&block))
		cxrt_exit("CXRT:ERROR: CXEX_THROW failed to peak try-blocks stack\n", CXERROR_THROW);

	cxexcept = ex;
	cxexcept.ret = final_ret;

	return &block->jump;
}

int cxex_istype(cxexception_t except, cxexception_t other)
{
	int i = 0;

	for(; other.type[i] && other.type[i] == except.type[i]; i++);

	if(other.type[i] == except.type[i])
		return 1;

	if((!other.type[i] || other.type[i] == '_') && (!except.type[i] || except.type[i] == '_'))
	{
		if(!except.type[i] && other.type[i] == '_')
			return 0;

		return 1;
	}

	return 0;
}
