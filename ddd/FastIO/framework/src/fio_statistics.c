//	$Id: fio_statistics.c 2013-05-23 likunxiang$
//
#include "fio_statistics.h"
#include "fio_sysconfig.h"
#include "fio_context.h"
#include "log_log.h"

struct timeval fio_now;
uint32_t fio_gnow = 0;
time_t g_cur_ts;

inline void tv_update_date()
{
    gettimeofday(&fio_now, NULL);
}

void * fio_count(void *data)
{
    int i;
    uint64_t my_rxcount = 0, my_txcount = 0, unuse_count = 0, prev_rx = 0, prev_tx = 0;
    struct timeval tic, toc;
    int report_interval = 1*1000;	/* report interval */
    char dump_buf_rx[2000] = {0};
    char dump_buf_tx[2000] = {0};
    char dump_buf[1000] = {0};

    gettimeofday(&toc, NULL);
    while (sysconfig.working)
    {
        struct timeval delta;
        uint64_t pps_rx, pps_tx;
        int done = 0;
        dump_buf_rx[0] = '\0';
        dump_buf_tx[0] = '\0';

        delta.tv_sec = report_interval/1000;
        delta.tv_usec = (report_interval%1000)*1000;

        //计时== 
        select(0, NULL, NULL, NULL, &delta);

        tv_update_date();
        timersub(&fio_now, &toc, &toc);
        my_rxcount = 0;
        my_txcount = 0;
        unuse_count = 0;
        for (i = 0; i < sysconfig.nthreads; i++) {
            my_rxcount += g_contexts[i].nics[0].rxcount;
            my_rxcount += g_contexts[i].nics[1].rxcount;
            my_txcount += g_contexts[i].nics[0].txcount;
            my_txcount += g_contexts[i].nics[1].txcount;
            unuse_count += g_contexts[i].nics[0].unuse_count;
            unuse_count += g_contexts[i].nics[1].unuse_count;
            sprintf(dump_buf, "%"LU64" ", g_contexts[i].nics[0].rxcount 
                    + g_contexts[i].nics[1].rxcount);
            strcat(dump_buf_rx, dump_buf);
            sprintf(dump_buf, "%"LU64" ", g_contexts[i].nics[0].txcount
                    + g_contexts[i].nics[1].txcount);
            strcat(dump_buf_tx, dump_buf);
            if (g_contexts[i].used == 0)
                done++;
        }
        pps_tx = pps_rx = toc.tv_sec* 1000000 + toc.tv_usec;
        if (pps_rx < 10000)
            continue;
        pps_rx = (my_rxcount - prev_rx)*1000000 / pps_rx;
        OD( "rx %"LU64" ppsrx %s", pps_rx, dump_buf_rx);
        pps_tx = (my_txcount - prev_tx)*1000000 / pps_tx;
        OD( "tx %"LU64" ppstx %s", pps_tx, dump_buf_tx);
        OD( "rx useless pkt %"LU64"\n", unuse_count);
        prev_rx = my_rxcount;
        prev_tx = my_txcount;
        toc = fio_now;
        fio_gnow++;
        if (done == sysconfig.nthreads)
            break;
    }

    timerclear(&tic);
    timerclear(&toc);
    OD( "I'll shutdown");

    return NULL;
}

