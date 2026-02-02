#include <cxrt.h>

int traversal_func(cxaddress_t entry, cxaddress_t arg, cxaddress_t ret);
int func();
int func2();
int func3();
void* func_thread(void *param);

int main()
{
	cxrt_stack_t stack = {};
	cxrt_func_t entry = {}, *entry2 = NULL, entry3 = {};
	cxaddress_t addr = NULL;
	int ret = 0;

	printf("\nCXRT: Test started\n");

	entry.flags = 123;

	if(ret = cxrt_func_construct(&entry))
		printf("cxrt_func_con: %i", ret);

	if(ret = cxrt_stack_construct(&stack, sizeof(cxrt_func_t), &cxrt_func_construct, &cxrt_func_destruct))
		printf("cxrt_stack_con: %i", ret);

	if(ret = cxrt_stack_push(&stack, &entry))
		printf("cxrt_stack_push: %i", ret);

	if(ret = cxrt_stack_peak(&stack, (cxaddress_t*)&entry2))
		printf("cxrt_stack_peak: %i", ret);

	if(ret = cxrt_stack_pop(&stack, &entry3))
		printf("cxrt_stack_pop: %i", ret);

	if(entry.flags != entry2->flags || entry.flags != entry3.flags)
		printf("cxrt_stack flags\n");

	for(int i = 0; i < 9; i++)
	{
		if(ret = cxrt_func_construct(&entry))
			printf("cxrt_func_con: %i", ret);

		if(ret = cxrt_stack_push(&stack, &entry))
			printf("cxrt_stack_push: %i", ret);
	}

	int arg = 1, ret2 = 0;

	if(ret = cxrt_stack_reverse_traversal(&stack, &traversal_func, &arg, &ret2))
		printf("cxrt_stack_reverse_traversal: %i", ret);

	if(ret2 != -1)
		printf("cxrt_stack_reverse_traversal: %i", ret);

	if(ret = cxrt_stack_double_peak(&stack, (cxaddress_t*)&entry2) || entry2->flags != arg)
		printf("cxrt_stack_double_peak: %i", ret);

	for(int i = 0; i < 9; i++)
	{
		if(ret = cxrt_stack_pop(&stack, &entry))
			printf("cxrt_stack_pop: %i", ret);

		if(entry.flags != arg)
			printf("cxrt_stack_reverse_traversal: flag not set\n");
	}

	if(ret = cxrt_stack_destruct(&stack))
		printf("cxrt_stack_des: %i", ret);

	__cxmain__

	func();

	pthread_t thr, thr2, thr3;

	pthread_create(&thr, NULL, &func_thread, NULL);
	pthread_create(&thr2, NULL, &func_thread, NULL);
	pthread_create(&thr3, NULL, &func_thread, NULL);

	pthread_join(thr, NULL);
	pthread_join(thr2, NULL);
	pthread_join(thr3, NULL);

	//printf("after join\n");

	CXRT_RETURN();

	printf("CXRT: Test finished\n");

	return 0;
}

int traversal_func(cxaddress_t entry, cxaddress_t arg, cxaddress_t ret)
{
	cxrt_func_t *func = entry;
	int *num = arg;
	int *num2 = ret;

	func->flags = *num;
	*num2 = -1;

	return 0;
}

int func()
{
	__cxfunc__

	//printf("func_\n");

	func2();

	cxreturn 0;
}

int func2()
{
	__cxfunc__

	//printf("func_2\n");

	func3();

	cxreturn 0;
}

int func3()
{
	__cxfunc__

	//printf("func_3\n");

	cxreturn 0;
}

void* func_thread(void *param)
{
	__cxthread__

	//printf("func_thread\n");

	func();

	cxreturn NULL;
}
