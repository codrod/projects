#include <cxrt.h>

int cxrt_stack_construct(cxrt_stack_t *stack, cxsize_t size_of, int (*construct)(cxaddress_t), int (*destruct)(cxaddress_t))
{
	if(!(stack->addr = malloc(CXRT_STACK_DEFAULT_CAP * size_of)))
		return CXERROR_NOMEM;

	stack->cap = CXRT_STACK_DEFAULT_CAP;
	stack->size = 0;

	stack->size_of = size_of;
	stack->construct = construct;
	stack->destruct = destruct;

	return 0;
}

int cxrt_stack_destruct(cxrt_stack_t *stack)
{
	cxsize_t i = 0;

	if(stack->addr)
	{
		if(stack->destruct)
			for(; i < stack->size; i++)
				(*stack->destruct)(stack->addr + i * stack->size_of);

		free(stack->addr);
		stack->addr = NULL;
	}

	return 0;
}

int cxrt_stack_push(cxrt_stack_t *stack, cxaddress_t func)
{
	void *tmp = NULL;

	if(stack->size >= stack->cap)
	{
		if(!(tmp = realloc(stack->addr, stack->cap * 2 * stack->size_of)))
		{
			errno = CXERROR_NOMEM;
			return CXERROR_NOMEM;
		}

		stack->addr = tmp;
		stack->cap *= 2;
	}

	memcpy(stack->addr + stack->size++ * stack->size_of, func, stack->size_of);

	return 0;
}

int cxrt_stack_peak(cxrt_stack_t *stack, cxaddress_t *addr)
{
	if(stack->size < 1)
	{
		errno = CXERROR_EMPTY;
		return CXERROR_EMPTY;
	}

	*addr = stack->addr + (stack->size - 1) * stack->size_of;

	return 0;
}

int cxrt_stack_double_peak(cxrt_stack_t *stack, cxaddress_t *entry)
{
	if(stack->size < 2)
	{
		errno = CXERROR_EMPTY;
		return CXERROR_EMPTY;
	}

	*entry = stack->addr + (stack->size - 2) * stack->size_of;

	return 0;
}


int cxrt_stack_pop(cxrt_stack_t *stack, cxaddress_t func)
{
	int ret = 0;
	cxaddress_t addr = NULL;

	if((ret = cxrt_stack_peak(stack, &addr)))
		return ret;

	if(func)
		memcpy(func, addr, stack->size_of);

	stack->size--;

	return 0;
}

int cxrt_stack_reverse_traversal(cxrt_stack_t *stack, int (*func)(cxaddress_t entry, cxaddress_t arg, cxaddress_t ret), cxaddress_t arg, cxaddress_t ret)
{
	cxsize_t i = 0;
	int tmp = 0;

	if(!stack->size)
		return 0;

	for(i = stack->size - 1; i >= 0; i--)
	{
		if(tmp = (*func)(stack->addr + i * stack->size_of, arg, ret))
			return tmp;
	}

	return 0;
}
