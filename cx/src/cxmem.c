#include <cxrt.h>

cxexception_t CXException_Memory = {"CXException_Memory", "", 0};
cxexception_t CXException_Memory_NoMemory = {"CXException_Memory_NoMemory", "not enough space/cannot allocate memory", CXERROR_NOMEM};
cxexception_t CXException_Memory_InvalidAddress = {"CXException_Memory_InvalidAddress", "tried to realloc or free invalid address", CXERROR_ADDRESS};
cxexception_t CXException_Memory_InvalidPush = {"CXException_Memory_InvalidPush", "tried to push memory off the top of the stack", CXERROR_PUSH};
cxexception_t CXException_Memory_InvalidArg = {"CXException_Memory_InvalidArg", "invalid function argument", CXERROR_ARG};

cxaddress_t cxmalloc(cxmem_storage_t *storage, cxsize_t cap)
{
	cxmem_alloc_t alloc = {};
	int ret = 0;

	if(cap <= 0)
		cxthrow(CXException_Memory_InvalidArg);

	if(!(alloc.addr = malloc(cap)))
		cxthrow(CXException_Memory_NoMemory);

	alloc.cap = cap;

	if(storage && (ret = cxmem_storage_add(storage, alloc)))
		cxrt_exit("CXRT:ERROR: failed to add memory to storage\n", ret);

	return alloc.addr;
}

cxaddress_t cxrealloc(cxaddress_t addr, cxsize_t cap)
{
	cxmem_storage_t *storage = NULL;
	cxmem_alloc_t alloc = {};
	int ret = 0;

	if(cap <= 0)
		cxthrow(CXException_Memory_InvalidArg);

	if(!addr)
		cxthrow(CXException_Memory_InvalidAddress);

	if(ret = cxmem_storage_of(&storage, NULL, addr))
		cxrt_exit("CXRT:ERROR: failed to find storage for address\n", ret);

	//if(!storage)
		//cxthrow(CXException_Memory_InvalidAddress, CXERROR_ADDRESS);

	if(!(alloc.addr = realloc(addr, cap)))
		cxthrow(CXException_Memory_NoMemory);

	alloc.cap = cap;

	if(storage && (ret = cxmem_storage_replace(storage, alloc, addr)))
		cxrt_exit("CXRT:ERROR: failed to add memory to storage\n", ret);

	return alloc.addr;
}

void cxfree(cxaddress_t addr)
{
	cxmem_storage_t *storage = NULL;
	int ret = 0;

	if(!addr)
		cxthrow(CXException_Memory_InvalidAddress);

	if(ret = cxmem_storage_of(&storage, NULL, addr))
		cxrt_exit("CXRT:ERROR: failed to find storage for address\n", ret);

	//if(!storage)
		//cxthrow(CXException_Memory_InvalidAddress, CXERROR_ADDRESS);

	if(storage && (ret = cxmem_storage_remove(storage, NULL, addr)))
		cxrt_exit("CXRT:ERROR: failed to remove memory from storage\n", ret);

	free(addr);

	return;
}

cxaddress_t cxnew(cxmem_storage_t *storage, cxinter_t *inter, ...)
{
	cxmem_alloc_t alloc = {};
	int ret = 0, i = 0;

	va_list argv;
	va_start(argv, inter);

	if(!storage || !inter)
		cxthrow(CXException_Memory_InvalidArg);

	if(!(alloc.addr = calloc(1, inter->size_of)))
		cxthrow(CXException_Memory_NoMemory);

	alloc.inter = inter;
	alloc.cap = inter->size_of;

	if(inter->construct)
	{
		cxtry
		{
			(*inter->construct)(inter->size_of, alloc.addr, &argv);
		}
		cxcatch()
		{
			free(alloc.addr);
			cxthrow(cxexcept);
		}
	}

	if(ret = cxmem_storage_add(storage, alloc))
		cxrt_exit("CXRT:ERROR: failed to add memory to storage\n", ret);

	return alloc.addr;
}

void cxdelete(cxaddress_t addr)
{
	cxmem_storage_t *storage = NULL;
	cxmem_alloc_t alloc = {};
	int ret = 0;

	if(!addr)
		cxthrow(CXException_Memory_InvalidAddress);

	if(ret = cxmem_storage_of(&storage, &alloc, addr))
		cxrt_exit("CXRT:ERROR: failed to find storage for address\n", ret);

	if(!storage)
		cxthrow(CXException_Memory_InvalidAddress);

	if(alloc.inter && alloc.inter->destruct)
	{
		cxtry
		{
			(*alloc.inter->destruct)(alloc.addr);
		}
		cxcatch()
		{
			//continue;
		}
	}

	if(ret = cxmem_storage_remove(storage, NULL, addr))
		cxrt_exit("CXRT:ERROR: failed to remove memory from storage\n", ret);

	free(addr);
}

cxaddress_t cxpush(cxaddress_t addr)
{
	cxmem_storage_t *storage = NULL;
	cxmem_alloc_t alloc = {};
	cxrt_func_t *func = NULL;
	int ret = 0;

	if(ret = cxrt_stack_double_peak(&CXRUNTIME.stack, (cxaddress_t*)&func))
	{
		if(ret == CXERROR_EMPTY)
			cxthrow(CXException_Memory_InvalidPush);

		cxrt_exit("CXRT:ERROR: failed to peak parent functions auto storage\n", ret);
	}

	if(ret = cxmem_storage_of(&storage, &alloc, addr))
		cxrt_exit("CXRT:ERROR: failed to find storage for address\n", ret);

	if(!storage)
		cxthrow(CXException_Memory_InvalidAddress);

	if(ret = cxmem_storage_remove(storage, &alloc, addr))
		cxrt_exit("CXRT:ERROR: failed to remove memory from storage\n", ret);

	if(ret = cxmem_storage_add(&func->storage, alloc))
		cxrt_exit("CXRT:ERROR: failed to remove memory from storage\n", ret);

	return addr;
}

void cxpushall()
{
	cxmem_storage_t *storage = NULL;
	cxmem_alloc_t alloc = {};
	cxrt_func_t *func = NULL;
	int ret = 0;

	if(ret = cxrt_stack_double_peak(&CXRUNTIME.stack, (cxaddress_t*)&func))
	{
		if(ret == CXERROR_EMPTY)
			cxthrow(CXException_Memory_InvalidPush);

		cxrt_exit("CXRT:ERROR: failed to peak parent functions auto storage\n", ret);
	}

	if(ret = cxmem_storage_merge(&func->storage, cxauto))
		cxrt_exit("CXRT:ERROR: failed to merge storage\n", ret);

	return;
}

cxaddress_t cxrestore(cxmem_storage_t *new_storage, cxaddress_t addr)
{
	cxmem_alloc_t alloc = {};
	cxmem_storage_t *storage = NULL;
	int ret = 0;

	if(!new_storage)
		cxthrow(CXException_Memory_InvalidArg);

	if(!addr)
		cxthrow(CXException_Memory_InvalidAddress);

	if(ret = cxmem_storage_of(&storage, &alloc, addr))
		cxrt_exit("CXRT:ERROR: failed to find storage for address\n", ret);

	if(!storage)
		cxthrow(CXException_Memory_InvalidAddress);

	if(ret = cxmem_storage_remove(storage, &alloc, addr))
		cxrt_exit("CXRT:ERROR: failed to remove memory from storage\n", ret);

	if(ret = cxmem_storage_add(new_storage, alloc))
		cxrt_exit("CXRT:ERROR: failed to remove memory from storage\n", ret);

	return addr;
}

void cxrestoreall(cxmem_storage_t *new_storage, cxmem_storage_t *old_storage)
{
	int ret = 0;

	if(!new_storage || !old_storage)
		cxthrow(CXException_Memory_InvalidArg);

	if(ret = cxmem_storage_merge(new_storage, old_storage))
		cxrt_exit("CXRT:ERROR: failed to merge storage\n", ret);

	return;
}
