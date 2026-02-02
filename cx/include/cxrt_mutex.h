#if !CXRT_MUTEX_VERSION

	#define CXRT_MUTEX_VERSION 1000000L

	#include <pthread.h>
	#include <cxerror.h>

	typedef pthread_mutex_t cxrt_mutex_t;

	int cxrt_mutex_construct(cxrt_mutex_t *mutex);
	int cxrt_mutex_destruct(cxrt_mutex_t *mutex);

	int cxrt_mutex_lock(cxrt_mutex_t *mutex);
	int cxrt_mutex_trylock(cxrt_mutex_t *mutex);
	int cxrt_mutex_unlock(cxrt_mutex_t *mutex);

#endif
