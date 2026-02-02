#include <cxrt.h>
#include <pthread.h>
#include <stdio.h>

int func();
int func2();
void* func_thread(void*);

cxexception_t typeA = {"typeA", "what", 0};
cxexception_t typeB = {"typeB", "what", 0};
cxexception_t typeC = {"typeC", "what", 0};

int main()
{
	printf("\nCXEX: Test started\n");

	__cxmain__

	int var = 0;
	pthread_t thr, thr2, thr3;

	cxexception_t ex = {"CX_Signal_SegFault", "what", -1};

	if(!cxex_istype(ex, (cxexception_t){"CX", "what",  -1}))
		printf("istype: should be type\n");

	if(!cxex_istype(ex, (cxexception_t){"CX_Signal", "what",  -1}))
		printf("istype: should be type\n");

	if(!cxex_istype(ex, (cxexception_t){"CX_Signal_SegFault", "what",  -1}))
		printf("istype: should be type\n");

	if(cxex_istype(ex, (cxexception_t){"", "what",  -1}))
		printf("istype: should not be type\n");

	if(cxex_istype((cxexception_t){"CX_Signal", "", -1}, ex))
		printf("istype: should not be type\n");

	if(cxex_istype(ex, (cxexception_t){"asdofuhsd", "what",  -1}))
		printf("istype: should not be type\n");

	if(cxex_istype(ex, (cxexception_t){"CX_Sig", "what",  -1}))
		printf("istype: should not be type\n");

	cxtry
	{
		cxtry
		{
			var = 1;
		}
		cxcatch()
		{
			//nothing to do
		}

		cxtry
		{
			cxthrow_ret(typeA, -1);
		}
		cxcatch()
		{
			if(!cxex_istype(cxexcept, typeA))
				printf("cxcatch: caught wrong exception %s\n", cxexcept.type);
		}

		cxthrow_ret(typeB, -1);
	}
	cxcatch()
	{
		if(!cxex_istype(cxexcept, typeB))
			printf("cxcatch: caught wrong exception %s\n", cxexcept.type);
		//continue
	}

	cxtry
	{
		cxtry
		{
			cxthrow_ret(CXException_Signal_SegFault, 0);
		}
		cxcatch(CXException_Signal_IllegalInstruction, CXException_Signal_FloatingPointError)
		{
			if(cxex_istype(cxexcept, CXException_Signal_SegFault))
				printf("cxcatch: caught wrong exception type %s\n", cxexcept.type);
		}
	}
	cxcatch(CXException_Signal_IllegalInstruction, CXException_Signal_SegFault)
	{
		if(!cxex_istype(cxexcept, CXException_Signal_SegFault))
			printf("cxcatch: caught wrong exception type %s\n", cxexcept.type);
	}

	cxtry
	{
		cxthrow_ret(CXException_Signal_SegFault, 0);
	}
	cxcatch()
	{
		if(!cxex_istype(cxexcept, CXException_Signal_SegFault) || cxexcept.ret != CXERROR_SIGNAL)
			printf("cxcatch: caught wrong exception type %s\n", cxexcept.type);
	}

	cxtry
	{
		cxbyte_t *p = NULL;

		cxbyte_t b = *p;
	}
	cxcatch()
	{
		if(!cxex_istype(cxexcept, CXException_Signal_SegFault))
			printf("cxcatch: did not catch SegFault %s\n", cxexcept.type);
	}

	cxtry
	{
		cxbyte_t *p = NULL;

		cxbyte_t b = *p;
	}
	cxcatch()
	{
		if(!cxex_istype(cxexcept, CXException_Signal_SegFault))
			printf("cxcatch: did not catch SegFault %s\n", cxexcept.type);
	}

	cxtry
	{
		int i = 0;

		int j = 10 / i;
	}
	cxcatch()
	{
		if(!cxex_istype(cxexcept, CXException_Signal_FloatingPointError))
			printf("cxcatch: did not catch FloatingPointException %s\n", cxexcept.type);
	}

	cxtry
	{
		func();
	}
	cxcatch()
	{
		if(!cxex_istype(cxexcept, typeC))
			printf("cxcatch: caught wrong exception %s\n", cxexcept.type);
	}

	pthread_create(&thr, NULL, &func_thread, NULL);
	pthread_create(&thr2, NULL, &func_thread, NULL);
	pthread_create(&thr3, NULL, &func_thread, NULL);

	pthread_join(thr, NULL);
	pthread_join(thr2, NULL);
	pthread_join(thr3, NULL);

	printf("CXEX: Test finished\n");

	cxreturn 0;
}


int func()
{
	__cxfunc__

	int var = 0;

	cxtry
	{
		cxtry
		{
			var = 1;
		}
		cxcatch()
		{
			//nothing to do
		}

		cxtry
		{
			cxthrow_ret(typeA, -1);
		}
		cxcatch()
		{
			if(!cxex_istype(cxexcept, typeA))
				printf("cxcatch: caught wrong exception %s\n", cxexcept.type);

			//continue
		}

		cxthrow_ret(typeB, -1);
	}
	cxcatch()
	{
		if(!cxex_istype(cxexcept, typeB))
			printf("cxcatch: caught wrong exception %s\n", cxexcept.type);
	}

	func2();

	cxreturn 0;
}

int func2()
{
	__cxfunc__

	cxthrow_ret(typeC, -1);

	cxreturn 0;
}

void* func_thread(void*foo)
{
	__cxthread__

	cxtry
	{
		func();
	}
	cxcatch()
	{
		if(!cxex_istype(cxexcept, typeC))
			printf("cxcatch: caught wrong exception %s\n", cxexcept.type);
	}

	cxtry
	{
		cxbyte_t *p = NULL;

		cxbyte_t b = *p;
	}
	cxcatch()
	{
		if(!cxex_istype(cxexcept, CXException_Signal_SegFault))
			printf("cxcatch: did not catch SegFault %s\n", cxexcept.type);
	}

	cxreturn NULL;
}
