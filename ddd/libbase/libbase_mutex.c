#include <pthread.h>
#include "libbase_type.h"
#include "libbase_mutex.h"
#include "libbase_memory.h"

struct h_mutex {
	pthread_mutex_t mutex;
};

#if 0
struct __wns_rw_mutex{
	pthread_rwlock_t mutex;
};
#endif

h_mutex_st *h_mutex_create()
{
	h_mutex_st *mutex = (h_mutex_st *)h_malloc(sizeof(h_mutex_st));

	if (!mutex)
		return NULL;

	if(pthread_mutex_init(&mutex->mutex,NULL)!=0)
	{
		h_free(mutex);
		return NULL;
	}
	return mutex;
}
void h_mutex_lock(h_mutex_st *mutex)
{
	pthread_mutex_lock(&mutex->mutex);
}

void h_mutex_unlock(h_mutex_st *mutex)
{
	pthread_mutex_unlock(&mutex->mutex);
}

void h_mutex_destroy(h_mutex_st *mutex)
{
	pthread_mutex_destroy(&mutex->mutex);
	h_free(mutex);
}

#if 0
int32_t wns_rwlock_init(wns_rwlock_st *mutex)
{
	return (int32_t)pthread_rwlock_init(&mutex->mutex,NULL);
}

void wns_read_lock(wns_rwlock_st *mutex)
{
	pthread_rwlock_rdlock(&mutex->mutex);
}

void wns_read_unlock(wns_rwlock_st *mutex)
{
	pthread_rwlock_unlock(&mutex->mutex);
}

void wns_write_lock(wns_rwlock_st *mutex)
{
	pthread_rwlock_wrlock(&mutex->mutex);
}

void wns_write_unlock(wns_rwlock_st *mutex)
{
	pthread_rwlock_unlock(&mutex->mutex);
}

void wns_rwlock_destroy(wns_rwlock_st *mutex)
{
	pthread_rwlock_destroy(&mutex->mutex);
}

#endif
