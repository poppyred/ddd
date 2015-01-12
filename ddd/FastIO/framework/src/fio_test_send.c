//	$Id: fio_test_send.c 2014-04-12 likunxiang $
//
#include "fio_test_send.h"
#include "fio_context.h"
#include "fio_handler.h"
#include "fio_sysconfig.h"
#include "fio_pack_misc.h"
//#include "log_log.h"

#define _SND_LIMIT_
fun_get_pkt g_get_pkt;
void fio_test_snd_set_fun(fun_get_pkt _fun)
{
    g_get_pkt = _fun;
}

static int fio_fill_pkt(struct netmap_ring *ring, struct fio_txdata *pkt, u_int count)
{
    u_int sent, cur = ring->cur;
    struct netmap_slot *slot;
    char *p;

    if (ring->avail < count)
        count = ring->avail;

    for (sent = 0; sent < count; sent++)
    {
        slot = &ring->slot[cur];
        p = NETMAP_BUF(ring, slot->buf_idx);

        memcpy(p, pkt->pbuf, pkt->size);

        slot->len = pkt->size;
        cur = NETMAP_RING_NEXT(ring, cur);
    }

    ring->avail -= sent;
    ring->cur = cur;

    return (sent);
}

static void * fio_send_loop(struct fio_context *context)
{
    //uint16_t sport_rang = 5000;
    struct fio_nic *nics[2];
    struct pollfd fds[2+1];
    uint32_t dirs[2];
    int all_move, limit;
    int m, num_ready, i, num_nic = context->num_nic;
    struct netmap_if *nifp;
    struct netmap_ring *txring;
    struct fio_nic *tn;
    struct fio_poll_data pd;
    //uint32_t dst_ips[] = {(ntohl((uint32_t)(sysconfig.dst_ip[0].start.s_addr)) & 0xFFFFFF00), 
    //    (ntohl((uint32_t)(sysconfig.dst_ip[1].start.s_addr)) & 0xFFFFFF00)};
    //uint32_t dst_ips[] = {(ntohl((uint32_t)(sysconfig.src_ip.start.s_addr)) & 0xFFFFFF00), 
    //    (ntohl((uint32_t)(sysconfig.src_ip.start.s_addr)) & 0xFFFFFF00)};
    for (i = 0; i < num_nic; i++)
    {
        nics[i] = &context->nics[i];
        dirs[i] = FIO_DIR_TX;
    }

    OD( "tid %d Wait %d secs for phy reset", context->me, 5);
    sleep(5);
    OD( "tid %d Ready... num_nic %d", context->me, num_nic);
    gettimeofday(&context->tic, NULL);

    pd.nics = nics;
    pd.fds = fds;
    pd.dirs = dirs;
    pd.num_nic = num_nic;
    pd.notifyfds = context->notifyfds;
    pd.num_notify = 0;

#ifdef _BREAKh_
    int breakh = 0;
#endif
    while (sysconfig.working) 
    {
        //if ( (num_ready=context->vtbl.polls(nics, fds, dirs, num_nic, 500)) < 0) 
        if ( (num_ready=context->vtbl.polls(&pd, 500)) < 0) 
        {
            gettimeofday(&context->toc, NULL);
            context->toc.tv_sec -= 1; /* Subtract timeout time. */
            OD( "<error> poll error!!!!!");
            continue;
        }
#ifdef _BREAKh_
        if (breakh)
            break;
#endif

        if (num_ready == 0)
        {
            continue;
        }

        for (i = 0; i < num_nic; i++)
        {
            tn = nics[i];
            //fio_nic_send_arpbuf(tn);
            fio_nic_send_txbuf(tn);
            if (tn->nictxavl < 1)
                continue;

            all_move = 0;
            m = 0;
            nifp = tn->nifp;
            limit = tn->nictxavl;

#ifdef _SND_LIMIT_
            //D("limit %d", limit);
            limit = sysconfig.snd_pkts_per_sec < tn->nictxavl ? sysconfig.snd_pkts_per_sec : tn->nictxavl;
#endif

            while(limit > 0)
            {
                txring = NETMAP_TXRING(nifp, tn->cur_tx_head);
                if (txring->avail == 0)
                {
                    tn->cur_tx_head = (tn->cur_tx_head+1 == tn->qtxlast ? 
                            tn->qtxfirst : tn->cur_tx_head+1);
                }
                else
                {
                    m = fio_fill_pkt(txring, g_get_pkt(i), limit);
                    all_move += m;
                    limit -= m;
                }
            }
            if (nm_likely(all_move > 0))
            {
                tn->txcount += all_move;
                tn->nictxavl -= all_move;
            }
        }
#ifdef _BREAKh_
        //breakh = 1;
#endif

#ifdef _SND_LIMIT_
        //sleep(1);
#endif
    }

    /* reset the ``used`` flag. */
    context->used = 0;
    OD( "I'll shutdown");

    return NULL;
}

void * fio_test_send(void *t)
{
    struct fio_context *context = (struct fio_context *)t;
    return fio_send_loop(context);
}

