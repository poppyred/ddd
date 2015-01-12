//	$Id: fio_timer.c 2013-05-23 likunxiang$
//
#include "fio_timer.h"
#include "fio_nm_util.h"
#include "fio_map32.h"
//#include "log_log.h"
#include "fio_sysconfig.h"

#define constMaxTimerHandler 1024
struct timer_handle_map
{
    int32_t id;
    FIO_FUN_TIMER_HANDLE handle;
};

struct timer_handle_map g_t_handle_map[constMaxTimerHandler];
struct fio_timer g_fio_timer;

int32_t fio_register_timer_handler(int32_t id, FIO_FUN_TIMER_HANDLE fun)
{
    int i, found = 0;
    for (i = 0; i < constMaxTimerHandler && g_t_handle_map[i].id; i++)
        if (g_t_handle_map[i].id == id)
        {
            found = 1;
            break;
        }

    if (i == constMaxTimerHandler)
        return -1;

    if (found)
        return -2;

    g_t_handle_map[i].id = id;
    g_t_handle_map[i].handle = fun;
    return 0;
}

void * fio_timer_thread(void *data)
{
    struct timeval tic, toc;
    int report_interval = g_fio_timer.interval*1000;	/* report interval */
    uint64_t t_elapse;

    int pos;
    gettimeofday(&toc, NULL);
    while (sysconfig.working)
    {
        struct timeval now, delta;

        delta.tv_sec = report_interval/1000;
        delta.tv_usec = (report_interval%1000)*1000;

        //计时== 
        select(0, NULL, NULL, NULL, &delta);

        gettimeofday(&now, NULL);
        timersub(&now, &toc, &toc);

        t_elapse = toc.tv_sec* 1000000 + toc.tv_usec;
        if (t_elapse < 10000)
            continue;

        //notify_mac_thread(MM_ALL, 0);    
        //brcast_ifin_mac();
        for (pos = 0; pos < constMaxTimerHandler && g_t_handle_map[pos].id; pos++)
        {
            g_t_handle_map[pos].handle(t_elapse);
        }

        toc = now;
    }

    timerclear(&tic);
    timerclear(&toc);

    OD( "I'll shutdown");

    return NULL;
}

void fio_open_timer(int32_t intv)
{
    g_fio_timer.interval = intv;
    g_fio_timer.by_session = fio_map_create(sizeof(unsigned long));
    fio_map_init(g_fio_timer.by_session, HASH_TIMER_SESSION, HASH_TIMER_SESSION*5, "g_fio_timer");
    pthread_create(&g_fio_timer.thread_id, NULL, fio_timer_thread, NULL);
}

void fio_close_timer()
{
    if (g_fio_timer.thread_id)
        pthread_cancel(g_fio_timer.thread_id);
    if (g_fio_timer.by_session)
        fio_map_destroy(g_fio_timer.by_session);
}
