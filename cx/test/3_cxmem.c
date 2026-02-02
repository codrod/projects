#include <cxrt.h>

void cxui_objA_construct(cxsize_t size_of, cxaddress_t obj, va_list *argv);
void cxui_objB_destruct(cxaddress_t obj);

int func();
int func2(cxaddress_t addr, cxaddress_t addr2);
int func3(cxaddress_t addr, cxaddress_t addr2);
int func4(cxaddress_t addr3, cxaddress_t addr4, cxaddress_t *addr7, cxaddress_t *addr8);
int func5(cxaddress_t *addr9, cxaddress_t *addr10);

cxui_t cxui_objA = {"objA", 10, cxui_objA_construct, NULL, NULL, NULL, NULL, NULL};
cxui_t cxui_objB = {"objB", 10, NULL, cxui_objB_destruct, NULL, NULL, NULL, NULL};
cxui_t cxui_objC = {};

int main()
{
	__cxmain__

	printf("\nCXMEM: Test started\n");

	cxmem_storage_t storage;
	cxmem_alloc_t alloc = {}, tmp = {};
	int ret = 0, i = 0;

	alloc.addr = malloc(10);

	if(ret = cxmem_storage_construct(&storage))
		printf("cxmem_storage_construct: %i\n", ret);

	for(; i < 9; i++, alloc.addr++)
	{
		if(ret = cxmem_storage_add(&storage, alloc))
			printf("cxmem_storage_add: %i\n", ret);

		if(ret = cxmem_storage_get(&storage, &tmp, alloc.addr) || tmp.addr != alloc.addr)
			printf("cxmem_storage_get: %i\n", ret);
	}

	while(i--)
	{
		if(cxmem_storage_remove(&storage, NULL, --alloc.addr))
			printf("cxmem_storage_remove: %i\n", ret);

		if(ret = cxmem_storage_get(&storage, &tmp, alloc.addr) || tmp.addr)
			printf("cxmem_storage_get: %i\n", ret);
	}

	for(i = 0; i < 3; i++, alloc.addr++)
	{
		if(ret = cxmem_storage_add(&storage, alloc))
			printf("cxmem_storage_add: %i\n", ret);
	}

	for(i = 0; i < 3; i++)
	{
 		if(ret = cxmem_storage_replace(&storage, tmp, --alloc.addr))
			printf("cxmem_storage_replace: %i\n", ret);
	}

	cxmem_storage_t new_storage;

	if(ret = cxmem_storage_construct(&new_storage))
		printf("cxmem_storage_construct: %i\n", ret);

	for(i = 0; i < 3; i++, alloc.addr++)
	{
		if(ret = cxmem_storage_add(&new_storage, alloc))
			printf("cxmem_storage_add: %i\n", ret);
	}

	if(ret = cxmem_storage_merge(&storage, &new_storage))
		printf("cxmem_storage_merge: %i\n", ret);

	for(i = 0; i < 3; i++)
	{
		alloc.addr--;

		if(ret = cxmem_storage_remove(&storage, NULL, alloc.addr))
			printf("cxmem_storage_remove: %i\n", ret);
	}

	tmp.addr = malloc(10);
	tmp.ui = &cxui_objA;
	tmp.cap = 10;

	cxmem_storage_add(&storage, tmp);

	if(ret = cxmem_storage_destruct(&storage))
		printf("cxmem_storage_destruct: %i\n", ret);

	cxaddress_t addr = cxmalloc(cxstatic, 10);
	cxaddress_t addr2 = cxmalloc(cxauto, 10);
	float *addr3 = cxmalloc(cxthread_local, sizeof(float)*10);

	cxmem_storage_t *storagep = NULL;

	if(ret = cxmem_storage_of(&storagep, &alloc, addr) || storagep != cxstatic || alloc.addr != addr)
		printf("cxmem_storage_of: %i\n", ret);

	if(ret = cxmem_storage_of(&storagep, &alloc, addr2) || storagep != cxauto || alloc.addr != addr2)
		printf("cxmem_storage_of: %i\n", ret);

	if(ret = cxmem_storage_of(&storagep, &alloc, addr3) || storagep != cxthread_local || alloc.addr != addr3)
		printf("cxmem_storage_of: %i\n", ret);

	if((ret = cxmem_storage_of(&storagep, &alloc, addr + 2)) || storagep)
		printf("cxmem_storage_of: %i\n", ret);

	addr = cxrealloc(addr, 20);
	addr2 = cxrealloc(addr2, 30);
	addr3 = cxrealloc(addr3, sizeof(float)*20);

	cxfree(addr3);
	cxfree(addr);
	cxfree(addr2);

	cxtry
	{
		cxaddress_t objA = cxnew(cxauto, &cxui_objA);
	}
	cxcatch()
	{
		if(!cxex_istype(cxexcept, CXException_Signal_SegFault))
			printf("cxnew: caught wrong signal\n");
	}

	cxaddress_t objB = cxnew(cxauto, &cxui_objB);
	cxdelete(objB);

	addr = cxnew(cxstatic, &cxui_objC);
	addr2 = cxnew(cxauto, &cxui_objC);
	addr3 = cxnew(cxthread_local, &cxui_objC);

	storagep = NULL;

	if(ret = cxmem_storage_of(&storagep, &alloc, addr) || storagep != cxstatic || alloc.addr != addr)
		printf("cxmem_storage_of: %i\n", ret);

	if(ret = cxmem_storage_of(&storagep, &alloc, addr2) || storagep != cxauto || alloc.addr != addr2)
		printf("cxmem_storage_of: %i\n", ret);

	if(ret = cxmem_storage_of(&storagep, &alloc, addr3) || storagep != cxthread_local || alloc.addr != addr3)
		printf("cxmem_storage_of: %i\n", ret);

	if((ret = cxmem_storage_of(&storagep, &alloc, addr + 2)) || storagep)
		printf("cxmem_storage_of: %i\n", ret);

	cxdelete(addr2);
	cxdelete(addr);
	cxdelete(addr3);

	addr = cxmalloc(cxauto, 10);

	if(ret = cxmem_storage_of(&storagep, &alloc, addr) || storagep != cxauto || alloc.addr != addr)
		printf("cxmem_storage_of: %i\n", ret);

	cxrestore(cxstatic, addr);

	if(ret = cxmem_storage_of(&storagep, &alloc, addr) || storagep != cxstatic || alloc.addr != addr)
		printf("cxmem_storage_of: %i\n", ret);

	cxfree(addr);

	addr2 = cxmalloc(cxstatic, 20);

	if(ret = cxmem_storage_of(&storagep, &alloc, addr2) || storagep != cxstatic || alloc.addr != addr2)
		printf("cxmem_storage_of: %i\n", ret);

	cxrestore(cxauto, addr2);

	if(ret = cxmem_storage_of(&storagep, &alloc, addr2) || storagep != cxauto || alloc.addr != addr2)
		printf("cxmem_storage_of: %i\n", ret);

	cxfree(addr2);

	addr = cxmalloc(cxauto, 10);
	addr2 = cxmalloc(cxauto, 20);

	cxrestoreall(cxstatic, cxauto);

	if(ret = cxmem_storage_of(&storagep, &alloc, addr) || storagep != cxstatic || alloc.addr != addr)
		printf("cxmem_storage_of: %i\n", ret);

	if(ret = cxmem_storage_of(&storagep, &alloc, addr2) || storagep != cxstatic || alloc.addr != addr2)
		printf("cxmem_storage_of: %i\n", ret);

	cxtry
	{
		cxpush(addr);
	}
	cxcatch()
	{
		if(!cxex_istype(cxexcept, CXException_Memory_InvalidPush))
			printf("cxpush: caught wrong exception\n");
	}

	cxfree(addr);
	cxfree(addr2);

	func();

	printf("CXMEM: Test finished\n");

	cxreturn 0;
}

int func()
{
	__cxfunc__

	cxaddress_t addr = cxmalloc(cxauto, 10);
	cxaddress_t addr2 = cxnew(cxauto, &cxui_objC);

	func2(addr, addr2);

	cxreturn 0;
}

int func2(cxaddress_t addr, cxaddress_t addr2)
{
	__cxfunc__

	cxaddress_t addr3 = cxmalloc(cxauto, 10);
	cxaddress_t addr4 = cxnew(cxauto, &cxui_objC);

	cxfree(addr);
	cxdelete(addr2);

	func3(addr3, addr4);

	cxreturn 0;
}

int func3(cxaddress_t addr3, cxaddress_t addr4)
{
	__cxfunc__

	cxaddress_t addr5 = cxmalloc(cxauto, 10);
	cxaddress_t addr6 = cxnew(cxauto, &cxui_objC);
	cxaddress_t addr7 = NULL;
	cxaddress_t addr8 = NULL;

	func4(addr3, addr4, &addr7, &addr8);

	cxfree(addr5);
	cxdelete(addr6);

	cxdelete(addr8);
	cxfree(addr7);

	cxreturn 0;
}

int func4(cxaddress_t addr3, cxaddress_t addr4, cxaddress_t *addr7, cxaddress_t *addr8)
{
	__cxfunc__

	*addr7 = cxmalloc(cxauto, 10);
	*addr8 = cxnew(cxauto, &cxui_objC);
	cxaddress_t addr9 = NULL;
	cxaddress_t addr10 = NULL;


	cxpush(*addr7);
	cxpush(*addr8);

	func5(&addr9, &addr10);

	cxfree(addr3);
	cxdelete(addr4);

	cxfree(addr9);
	cxdelete(addr10);

	cxreturn 0;
}

int func5(cxaddress_t *addr9, cxaddress_t *addr10)
{
	__cxfunc__

	*addr9 = cxmalloc(cxauto, 10);
	*addr10 = cxnew(cxauto, &cxui_objC);

	cxpushall();

	cxreturn 0;
}

void cxui_objA_construct(cxsize_t size_of, cxaddress_t obj, va_list *argv)
{
	__cxfunc__

	cxthrow_ret(CXException_Signal_SegFault, 0);

	cxreturn_void;
}

void cxui_objB_destruct(cxaddress_t obj)
{
	__cxfunc__

	//printf("cxui_objB_destruct:\n");

	cxreturn_void;
}
