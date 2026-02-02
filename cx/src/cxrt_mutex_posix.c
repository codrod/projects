#include <cxrt_mutex.h>

int cxrt_mutex_construct(cxrt_mutex_t *mutex)
{
	return pthread_mutex_init(mutex, NULL);
}

int cxrt_mutex_destruct(cxrt_mutex_t *mutex)
{
	return pthread_mutex_destroy(mutex);
}

int cxrt_mutex_lock(cxrt_mutex_t *mutex)
{
	return pthread_mutex_lock(mutex);
}

int cxrt_mutex_trylock(cxrt_mutex_t *mutex)
{
	return pthread_mutex_trylock(mutex);
}

int cxrt_mutex_unlock(cxrt_mutex_t *mutex)
{
	return pthread_mutex_unlock(mutex);
}
