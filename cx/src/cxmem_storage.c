#include <cxrt.h>

cxmem_storage_t CXMEM_STATIC;
cxdef_thread_local cxmem_storage_t CXMEM_THREAD_LOCAL;

int cxmem_storage_construct(cxmem_storage_t *storage)
{
	if(!(storage->allocs = malloc(sizeof(cxmem_alloc_t) * CXMEM_DEFAULT_STORAGE_CAP)))
		return CXERROR_NOMEM;

	storage->cap = CXMEM_DEFAULT_STORAGE_CAP;
	storage->size = 0;

	storage->mutex = NULL;

	return 0;
}

int cxmem_storage_destruct(cxmem_storage_t *storage)
{
	int i = 0, ret = 0, tmp = 0;

	if(storage->mutex && (tmp = cxrt_mutex_lock(storage->mutex)))
		ret = tmp;

		for(; i < storage->size; i++)
		{
			if(storage->allocs[i].inter && storage->allocs[i].inter->destruct)
			{
				cxtry
				{
					(*storage->allocs[i].inter->destruct)(&storage->allocs[i]);
				}
				cxcatch()
				{
					//continue
				}

				free(storage->allocs[i].addr);
			}
		}

		free(storage->allocs);

	if(storage->mutex && (tmp = cxrt_mutex_unlock(storage->mutex)))
		ret = tmp;

	if(storage->mutex)
	{
		if((tmp = cxrt_mutex_destruct(storage->mutex)))
			ret = tmp;

		free(storage->mutex);
	}

	return ret;
}

int cxmem_storage_add(cxmem_storage_t *storage, cxmem_alloc_t alloc)
{
	cxaddress_t tmp = NULL;
	int ret = 0;

	if(storage->mutex && (ret = cxrt_mutex_lock(storage->mutex)))
		return ret;

		if(storage->size >= storage->cap)
		{
			if(!(tmp = realloc(storage->allocs, sizeof(cxmem_alloc_t) * storage->cap * 2)))
				return CXERROR_NOMEM;

			storage->allocs = tmp;
			storage->cap *= 2;
		}

		storage->allocs[storage->size] = alloc;
		storage->size++;

	if(storage->mutex && (ret = cxrt_mutex_unlock(storage->mutex)))
		return ret;

	return 0;
}

int cxmem_storage_remove(cxmem_storage_t *storage, cxmem_alloc_t *alloc, cxaddress_t addr)
{
	int ret = 0, found = 0;
	cxsize_t i = 0;

	if(storage->mutex && (ret = cxrt_mutex_lock(storage->mutex)))
		return ret;

		for(; i < storage->size; i++)
		{
			if(storage->allocs[i].addr == addr)
			{
				if(alloc) *alloc = storage->allocs[i];

				if(i != storage->size - 1)
					memmove(&storage->allocs[i], &storage->allocs[i + 1],
							sizeof(cxmem_storage_t)*(storage->size - (i + 1)));

				storage->size--;
				found = 1;

				break;
			}
		}

	if(storage->mutex && (ret = cxrt_mutex_unlock(storage->mutex)))
		return ret;

	if(!found)
		return CXERROR_NOT_FOUND;

	return 0;
}

int cxmem_storage_replace(cxmem_storage_t *storage, cxmem_alloc_t alloc, cxaddress_t addr)
{
	int ret = 0, found = 0;
	cxsize_t i = 0;

	//printf("replace start:\n");

	if(storage->mutex && (ret = cxrt_mutex_lock(storage->mutex)))
		return ret;

		for(; i < storage->size; i++)
		{
			//printf("replace: %lli %p %p\n", i, storage->allocs[i].addr, addr);

			if(storage->allocs[i].addr == addr)
			{
				//printf("break\n");
				storage->allocs[i] = alloc;
				found = 1;

				break;
			}
		}

	if(storage->mutex && (ret = cxrt_mutex_unlock(storage->mutex)))
		return ret;

	if(!found)
		return CXERROR_NOT_FOUND;

	return 0;
}

int cxmem_storage_get(cxmem_storage_t *storage, cxmem_alloc_t *alloc, cxaddress_t addr)
{
	int tmp = 0, found = 0;
	cxsize_t i = 0;
	cxmem_alloc_t ret = {};

	//printf("get start:\n");

	if(storage->mutex && (tmp = cxrt_mutex_lock(storage->mutex)))
		return tmp;

		for(; i < storage->size; i++)
		{
			//printf("get: %lli %p %p\n", i, storage->allocs[i].addr, addr);

			if(storage->allocs[i].addr == addr)
			{
				//printf("break\n");

				ret = storage->allocs[i];
				found = 1;

				break;
			}
		}

		*alloc = ret;

	if(storage->mutex && (tmp = cxrt_mutex_unlock(storage->mutex)))
		return tmp;

	return 0;
}

int cxmem_storage_get_traversal(cxaddress_t entry, cxaddress_t arg, cxaddress_t ret)
{
	cxmem_storage_t* *storagep = ret;
	cxmem_alloc_t alloc = {};
	cxaddress_t addr = arg;
	cxrt_func_t *func = entry;
	int tmp = 0;

	if(tmp = cxmem_storage_get(&func->storage, &alloc, addr))
		return tmp;

	if(alloc.addr)
		*storagep = &func->storage;

	return 0;
}

int cxmem_storage_of(cxmem_storage_t* *storagep, cxmem_alloc_t *allocp, cxaddress_t addr)
{
	cxmem_alloc_t alloc = {};
	int ret = 0, found = 1;
	cxmem_storage_t *storage = NULL;

	if(ret = cxrt_stack_reverse_traversal(&CXRUNTIME.stack, cxmem_storage_get_traversal, addr, &storage))
		return ret;

	if(storage)
		alloc.addr = addr;

	if(!alloc.addr)
	{
		if(ret = cxmem_storage_get(cxthread_local, &alloc, addr))
			return ret;

		if(!alloc.addr)
		{
			if(ret = cxmem_storage_get(cxstatic, &alloc, addr))
				return ret;

			if(alloc.addr)
				storage = cxstatic;
		}
		else storage = cxthread_local;
	}

	if(allocp) *allocp = alloc;

	*storagep = storage;

	return 0;
}

int cxmem_storage_merge(cxmem_storage_t *new_storage, cxmem_storage_t *old_storage)
{
	cxsize_t i = 0, j = 0;
	cxaddress_t tmp = NULL;
	int ret = 0;

	if(new_storage->mutex && (ret = cxrt_mutex_lock(new_storage->mutex)))
		return ret;

		if(old_storage->mutex && (ret = cxrt_mutex_lock(old_storage->mutex)))
			return ret;

			for(j = new_storage->size; i < old_storage->size; i++, j++, new_storage->size++)
			{
				//printf("get: %lli %p %p\n", i, storage->allocs[i].addr, addr);

				if(j >= new_storage->cap)
				{
					if(!(tmp = realloc(new_storage->allocs, sizeof(cxmem_alloc_t) * new_storage->cap * 2)))
						return CXERROR_NOMEM;

					new_storage->allocs = tmp;
					new_storage->cap *= 2;
				}

				new_storage->allocs[j] = old_storage->allocs[i];
			}

			old_storage->size = 0;

		if(old_storage->mutex && (ret = cxrt_mutex_unlock(old_storage->mutex)))
			return ret;

	if(new_storage->mutex && (ret = cxrt_mutex_unlock(new_storage->mutex)))
		return ret;

	return 0;
}
