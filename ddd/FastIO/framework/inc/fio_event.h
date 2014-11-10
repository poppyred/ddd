#ifndef _HIK_EVENT_H_ 
#define _HIK_EVENT_H_

#ifdef _MSC_VER
#include <Windows.h>
#define hik_fio_event_handle HANDLE
#else
#include <pthread.h>
#include <stdbool.h>

typedef struct 
{
    bool state;
    bool manual_reset;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}fio_event_t;
#define fio_event_handle fio_event_t*
#endif

//返回值：NULL 出错
fio_event_handle fio_event_create(bool manual_reset, bool init_state);

//返回值：0 等到事件，-1出错
int fio_event_wait(fio_event_handle hevent);

//返回值：0 等到事件，1 超时，-1出错
int fio_event_timedwait(fio_event_handle hevent, long milliseconds);

//返回值：0 成功，-1出错
int fio_event_set(fio_event_handle hevent);

//返回值：0 成功，-1出错
int fio_event_reset(fio_event_handle hevent);

//返回值：无
void fio_event_destroy(fio_event_handle hevent);

#endif

