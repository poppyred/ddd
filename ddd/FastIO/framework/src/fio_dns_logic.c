//	$Id: fio_dns_logic.c 2013-05-23 likunxiang$
//
#include "fio_dns_logic.h"
#include "fio_context.h"
#include "fio_handler.h"
#include "fio_sysconfig.h"
#include "fio_pack_misc.h"
#include "dns_pack.h"
#include "log_log.h"

#define _SND_LIMIT_

static int fio_fill_dns_pkt(struct netmap_ring *ring, struct fio_txdata *pkt, u_int count)
{
    u_int sent, cur = ring->cur;
    struct netmap_slot *slot;
    char *p;
    static uint16_t hid = 2; 

    if (ring->avail < count)
        count = ring->avail;

    for (sent = 0; sent < count; sent++)
    {
        slot = &ring->slot[cur];
        p = NETMAP_BUF(ring, slot->buf_idx);

        memcpy(pkt->pdata, &hid, 2);

        struct pktudp *ppkt = (struct pktudp*)pkt->pbuf;
        ppkt->udp.uh_sum = 0;
        fixCheckSumUDP(&ppkt->udp, (struct iphdr*)(&ppkt->ip), ppkt->body);

        memcpy(p, pkt->pbuf, pkt->size);
//hid++;

        slot->len = pkt->size;
        //if (sent == count - 1)
        //    slot->flags |= NS_REPORT;
        cur = NETMAP_RING_NEXT(ring, cur);
    }

    ring->avail -= sent;
    ring->cur = cur;

    return (sent);
}

static void * fio_dns_send(struct fio_context *context)
{
    //uint16_t sport_rang = 5000;
    uint32_t sip_rang[2];
    struct fio_nic *nics[2];
    struct pollfd fds[2+1];
    uint32_t dirs[2];
    int all_move, limit;
    int m, num_ready, i, j, num_nic = context->num_nic;
    struct netmap_if *nifp;
    struct netmap_ring *txring;
    struct fio_nic *tn;
    struct fio_poll_data pd;
    //uint32_t dst_ips[] = {(ntohl((uint32_t)(sysconfig.dst_ip[0].start.s_addr)) & 0xFFFFFF00), 
    //    (ntohl((uint32_t)(sysconfig.dst_ip[1].start.s_addr)) & 0xFFFFFF00)};
    //uint32_t dst_ips[] = {(ntohl((uint32_t)(sysconfig.src_ip.start.s_addr)) & 0xFFFFFF00), 
    //    (ntohl((uint32_t)(sysconfig.src_ip.start.s_addr)) & 0xFFFFFF00)};
    char donames[][255] = {
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com", 
        "www.likunxiang.com" 
    };
    int donames_len[] = {
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com"), 
        strlen("www.likunxiang.com")
    };
    struct fio_txdata txds[2][24];
    char buf_if_necessary[2*24][MAX_PKT_SIZE];
    struct pktudp *pkt4test[] = {&g_pkt4test, &g_pkt4test2};

    for (j = 0; j < num_nic; j++)
    {
        sip_rang[j] = 126;
        for (i = 0; i < 24; i++)
        {
            struct fio_txdata *txd = &txds[j][i];
            memset(txd, 0, sizeof(struct fio_txdata));
            txd->type = TXDATA_TYPE_NORMAL;
            //txd->pbuf = txd->buf;
            txd->pbuf = buf_if_necessary[j*24+i];
            txd->pdata = txd->pbuf+g_payload_offset;
            memcpy(txd->pbuf, (char*)pkt4test[j], sysconfig.pkt_size);
            int paylen = dns_pack_head(txd->pdata, donames[i], donames_len[i]);
            txd->size = g_payload_offset+paylen;
            *(uint16_t*)(txd->pbuf+38) = htons(paylen+8);
            *(uint16_t*)(txd->pbuf+16) = htons(paylen+8+20);

            struct pktudp *ppkt = (struct pktudp*)txd->pbuf;
            //*(uint32_t*)(txd->pbuf+g_sip_offset) = htonl(dst_ips[j]|sip_rang[j]++);
            *(uint32_t*)(txd->pbuf+g_sip_offset) = sysconfig.src_ip.start.s_addr;
            bcopy(&sysconfig.src_mac.start, txd->pbuf+g_eth_smac, 6);

            struct ip *ip = &ppkt->ip;                                                 
            ip->ip_sum = 0;
            ip->ip_sum = ip_fast_csum((char*)ip, ip->ip_hl);
        }
    }

    for (i = 0; i < num_nic; i++)
    {
        nics[i] = &context->nics[i];
        dirs[i] = FIO_DIR_TX;
        sip_rang[i] = 0;
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
            fio_nic_send_arpbuf(tn);
            fio_nic_send_txbuf(tn);
            if (tn->nictxavl < 1)
                continue;

            all_move = 0;
            m = 0;
            nifp = tn->nifp;
            limit = tn->nictxavl;

#ifdef _SND_LIMIT_
            //D("limit %d", limit);
            limit = sysconfig.snd_pkts_per_sec;
#endif

            while(limit > 0)
            {
                txring = NETMAP_TXRING(nifp, tn->cur_tx_head);
                if (txring->avail == 0)
                {
                    tn->cur_tx_head = (tn->cur_tx_head+1 == tn->qtxlast ? 
                            tn->qtxfirst : tn->cur_tx_head+1);
                    if (++sip_rang[i] >= 24)
                        sip_rang[i] = 0;
                }
                else
                {
                    m = fio_fill_dns_pkt(txring, &txds[i][sip_rang[i]], limit);
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
        sleep(2);
#endif
    }

    /* reset the ``used`` flag. */
    context->used = 0;
    OD( "I'll shutdown");

    return NULL;
}

void * fio_dns_recv(struct fio_context *context)
{
    struct fio_context *s_ct = &g_contexts[0];
    struct fio_nic *nics[2];
    struct pollfd fds[2+1];
    uint32_t dirs[2];
    struct fio_asted_rxs rxbufs[T_FIO_PKT_DISCARD];
    int limit = MAX_PKT_COPY_ONCE;
    int num_ready, i, num_nic = context->num_nic;
    struct fio_nic *rn, *tn;
    struct fio_poll_data pd;

    memset(rxbufs, 0, T_FIO_PKT_DISCARD*sizeof(struct fio_asted_rxs));
    for (i = T_FIO_PKT_INTD; i < T_FIO_PKT_DISCARD; i++)
        rxbufs[i].cb_type = i;

    for (i = 0; i < num_nic; i++)
    {
        nics[i] = &context->nics[i];
        dirs[i] = (FIO_DIR_RX);
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

    while (sysconfig.working) 
    {
//sleep(2);
//continue;
        if ( (num_ready=context->vtbl.polls(&pd, 2000)) < 0) 
        {
            gettimeofday(&context->toc, NULL);
            context->toc.tv_sec -= 1; /* Subtract timeout time. */
            OD( "<error> poll error!!!!!");
            continue;
        }

        if (num_ready == 0)
        {
            continue;
        }

        for (i = 0; i < num_nic; i++)
        {
            rn = nics[i];
            tn = &s_ct->nics[i];

            if (rn->rxavl > 0)
            {
                if (context->vtbl.recv(rn, NULL, rxbufs, limit, FIO_LIMIT_FROM) > 0)
                    process_data(rn, tn, nics, num_nic, rxbufs, TXDATA_TYPE_NORMAL, 0);
            }
        }
    }

    /* reset the ``used`` flag. */
    context->used = 0;
    OD( "I'll shutdown");

    return (NULL);
}

void * fio_dns_test(void *t)
{
    struct fio_context *context = (struct fio_context *)t;
    if (!strcmp(context->nics[0].alise, "send"))
        return fio_dns_send(context);
    else
        return fio_dns_recv(context);
    return NULL;
}

