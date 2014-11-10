//	$Id: fio_timer.h 2013-05-23 likunxiang$
//
#ifndef __FIO_TIMER_H__
#define __FIO_TIMER_H__

#include <inttypes.h>
#include <pthread.h>

#define HASH_TIMER_SESSION 127
#define constMAXTIMERID 0x10000000
enum
{
    FIO_T_CHECK_MAC = 1,
    FIO_T_CHECK_ROUTE,
    FIO_T_CHECK_MULTIP,
    FIO_T_CHECK_BUDDYMAC,
};

struct fio_map_t;
struct fio_timer
{
    int32_t interval;
    pthread_t thread_id;
    struct fio_map_t *by_session;
};
typedef void(*FIO_FUN_TIMER_HANDLE)(uint64_t elapse);

void fio_open_timer(int32_t intv);
void fio_close_timer();
int32_t fio_register_timer_handler(int32_t id, FIO_FUN_TIMER_HANDLE fun);


#endif
