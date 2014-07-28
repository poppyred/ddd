#ifndef __LIBWNS__MUTEX__H__
#define __LIBWNS__MUTEX__H__

struct h_mutex;

typedef struct h_mutex h_mutex_st;

h_mutex_st *h_mutex_create();
void h_mutex_destroy(h_mutex_st *mutex);
void h_mutex_lock(h_mutex_st *mutex);
void h_mutex_unlock(h_mutex_st *mutex);

#if 0
struct wns_spinlock;
typedef struct wns_spinlock wns_spinlock_st;
enum {
	WNS_SPIN_CPU_SHARED,
	WNS_SPIN_CPU_PRIVATE
};
int pthread_spin_create(wns_spinlock_st *lock, int pshared);
int pthread_spin_destroy(wns_spinlock_st *lock);
int wns_spin_lock(wns_spinlock_st *lock);
int wns_spin_trylock(wns_spinlock_st *lock);
int pthread_spin_unlock(wns_spinlock_st *lock);
#endif

#if 0
struct wns_rw_mutex;
typedef struct wns_rw_mutex wns_rwlock_st;
int32_t wns_rwlock_init(wns_rwlock_st *mutex);
void wns_rwlock_destroy(wns_rwlock_st *mutex);
void wns_read_lock(wns_rwlock_st *mutex);
void wns_read_unlock(wns_rwlock_st *mutex);
void wns_write_lock(wns_rwlock_st *mutex);
void wns_write_unlock(wns_rwlock_st *mutex);
#endif

#endif

