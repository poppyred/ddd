#include "fio_sysstack.h"
#include "fio_sysconfig.h"
#include "fio_context.h"
//#include "log_log.h"

#include <assert.h>

static const uint8_t fio_stack_protocol_sum[] = {
    IPPROTO_TCP,
    IPPROTO_UDP,
    0
};

inline int fio_stack_send_up(struct fio_nic *nic, char * const pbuf, int pkt_size)
{
    struct fio_txdata *txd;
    int sent = 0;
    if (fio_stack_reserve_tx(&nic->sys_txbuf, &txd) < 1)
        goto ret_here;
    txd->size = pkt_size;
    memcpy(txd->pbuf, pbuf, txd->size);
    sent = fio_rbf_commit(&nic->sys_txbuf, 1);
ret_here:
    return sent;
}
inline void fio_stack_notify_systhread(struct fio_nic *nic, struct fio_notify_msg *msg)
{
    int write_systhread = 0;
    if ((write_systhread=write(*NIC_EXTRA_CONTEXT(nic)->notifyfds, msg, sizeof(struct fio_notify_msg))) < 0) 
    {
        NOD( "tid %d error %d writing notify sockpair", NIC_EXTRA_CONTEXT(nic)->me, errno);
        //assert(write_systhread>=0);
    }
    else
    {
        if (sysconfig.prsys_stack)
            OD( "tid %d notify systhread msg_id = %u, nic_id = %d, abstract = %p", 
                NIC_EXTRA_CONTEXT(nic)->me, msg->msg_id, msg->nic_id, (void*)msg->abstract);
    }
}
int fio_stack_deliver(struct fio_nic *nic, struct fio_rxdata *rxd)
{
    if (sysconfig.enable_sysstack)
    {
        struct fio_txdata *txd;
        if (fio_stack_reserve_tx(&nic->sys_txbuf, &txd) > 0)
        {
            txd->size = rxd->size;
            memcpy(txd->pbuf, rxd->pbuf, txd->size);
            if (fio_rbf_commit(&nic->sys_txbuf, 1) > 0)
                return 0;
        }
        return -1;
    }
    return 0;
}
int fio_stack_reserve_tx(struct fio_ringbuf *rbf, struct fio_txdata **m)
{
    int ret = fio_rbf_reserve_one(rbf, m);

    if (ret != 1)
        return 0;

    if ((*m)->vtbl.reset)
    {
        (*m)->vtbl.reset(*m);
    }
    else
    {
        assert((*m)->vtbl.reset);
    }

    return ret;
}

int fio_stack_send_nic(struct fio_nic *sys_nic, struct fio_nic *_nic, int limit, void *param)
{
    struct netmap_if *nifp = sys_nic->nifp;
    struct netmap_ring *txring;
    int m, sent = 0;

    if (nm_unlikely(sys_nic->nictxavl == 0))
        return sent;
    if (limit > sys_nic->nictxavl)
        limit = sys_nic->nictxavl;

    while (limit > 0)
    {
        txring = NETMAP_TXRING(nifp, sys_nic->cur_tx_head);
        if (txring->avail == 0)
            sys_nic->cur_tx_head = (sys_nic->cur_tx_head+1 == sys_nic->qtxlast ? 
                    sys_nic->qtxfirst : sys_nic->cur_tx_head+1);
        else
        {
            m = fio_nic_send_pkts(txring, sys_nic, &_nic->sys_txbuf, TXDATA_TYPE_SYSSTACK_TX, limit);
            limit -= m;
            sent += m;
        }
    }
    sys_nic->nictxavl -= sent;
    sys_nic->txcount += sent;
    return sent;
}

int fio_stack_send(struct fio_nic *sys_nic, struct fio_nic *nic)
{
    int num = fio_rbf_occupy(&nic->sys_txbuf);
    if (num > 0)
        num = sys_nic->vtbl.send_sys(sys_nic, nic, num, NULL);
    return num;
}

void fio_stack_init_rxbuf(struct fio_ringbuf *rbf, struct fio_txdata *txb, int tid, int _id)
{
    txb->parent = rbf;
    txb->id_ = ((TXDATA_TYPE_SYSSTACK_RX<<TXDATA_TYPE_MASK_OFFSET) | (tid<<TXDATA_TYPE_CACHEID_OFFSET) | _id);
    txb->type = TXDATA_TYPE_SYSSTACK_RX;
    txb->pbuf = &rbf->buf_if_necessary[_id * MAX_PKT_SIZE];
    txb->pdata = txb->pbuf + g_payload_offset;
}

void fio_stack_init_txbuf(struct fio_ringbuf *rbf, struct fio_txdata *txb, int tid, int _id)
{
    txb->parent = rbf;
    txb->id_ = ((TXDATA_TYPE_SYSSTACK_TX<<TXDATA_TYPE_MASK_OFFSET) | (tid<<TXDATA_TYPE_CACHEID_OFFSET) | _id);
    txb->type = TXDATA_TYPE_SYSSTACK_TX;
    txb->pbuf = &rbf->buf_if_necessary[_id * MAX_PKT_SIZE];
    txb->pdata = txb->pbuf + g_payload_offset;
}

void fio_stack_judge_sum(struct pktudp *ppkt)
{
    struct ether_header *eh = &ppkt->eh;
    struct ip *ip = &ppkt->ip;
    int i;

    if (eh->ether_type != g_ip_type)
        return;

    i = 0;
    while (fio_stack_protocol_sum[i] != 0)
    {
        if (ip->ip_p == fio_stack_protocol_sum[i])
            break;
        i++;
    }
    
    if (fio_stack_protocol_sum[i] == 0)
        return;

    switch (fio_stack_protocol_sum[i])
    {
        case IPPROTO_TCP:
            {
                struct pkttcp *ptcp = (struct pkttcp*)ppkt;
                if (ptcp->tcp.uh_sum != 0)
                {
                    ptcp->tcp.uh_sum = 0;
                    ptcp->tcp.uh_sum = fixCheckSumTCP(ptcp);
				    printf("************ptcp->tcp.uh_sum = %x\n", ptcp->tcp.uh_sum);
                }
            }
            break;
        case IPPROTO_UDP:
            if (ppkt->udp.uh_sum != 0)
            {
                fixCheckSumUDP(&ppkt->udp, (struct iphdr*)ip, ppkt->body);
            }
            break;
    }
}

inline int fio_stack_get_threadnum(uint32_t ip, uint16_t port)
{
    return (int)((ip+port)%sysconfig.nthreads);
}

int fio_stack_recv_pkts(struct netmap_ring *ring, struct fio_nic *nic, uint32_t limit, int *arr_count, int *recv_again)
{
    uint32_t cur, pkts;
    struct netmap_slot *slot;
    char *p;
    int rx = 0, useless_pkt = 0, sysstack_pkt = 0;
    int threadnum;
    struct fio_txdata *txd;

    cur = ring->cur;
    if (ring->avail < limit)
        limit = ring->avail;

    for (pkts = 0; pkts < limit; pkts++) 
    {
        slot = &ring->slot[cur];
        p = NETMAP_BUF(ring, slot->buf_idx);
        nm_prefetch(p);

        //fio_stack_judge_sum((struct pktudp *)p);

        //{
        //    char str_sip[16];
        //    char str_dip[16];
        //    inet_ntop(AF_INET, (uint32_t*)(p+g_sip_offset), str_sip, 16);
        //    inet_ntop(AF_INET, (uint32_t*)(p+g_dip_offset), str_dip, 16);
        //    D("tid %d pkt len %d sip %s, sport %d, dip %s, dport %d", 
        //            NIC_EXTRA_CONTEXT(nic)->me, slot->len, 
        //            str_sip, ntohs(*(uint16_t*)(p+g_sport_offset)),
        //            str_dip, ntohs(*(uint16_t*)(p+g_recvport_offset)));

        //    char str_x[1500] = {0};
        //    char str_byte[10];
        //    int i;
        //    for (i = 0; i < slot->len; i++)
        //    {
        //        snprintf(str_byte, 10, "%02X ", (unsigned char)p[i]);
        //        if (i%8 == 0)
        //            strncat(str_x, "\n", 1500);
        //        strncat(str_x, str_byte, 1500);
        //    }
        //    D("%s", str_x);
        //}

        threadnum = fio_stack_get_threadnum(*(uint32_t*)(p+g_dip_offset), *(uint16_t*)(p+g_recvport_offset));
        if (fio_stack_reserve_tx(&g_contexts[threadnum].nics[nic->type_id].sys_rxbuf, &txd) < 1)
            goto _rxbuf_full;

        txd->size = slot->len;
        memcpy(txd->pbuf, p, txd->size);
        fio_rbf_commit(&g_contexts[threadnum].nics[nic->type_id].sys_rxbuf, 1);

        arr_count[threadnum]++;
        rx++;
        cur = NETMAP_RING_NEXT(ring, cur);
    }
    *recv_again = 0;

_rxbuf_finish:
    if (nm_likely(pkts > 0))
    {
        nic->unuse_count += useless_pkt;
        nic->sysstack_count += sysstack_pkt;
        ring->avail -= pkts;
        ring->cur = cur;
    }

    return pkts;

_rxbuf_full:
    *recv_again = 1;
    goto _rxbuf_finish;
}

int fio_stack_recv_nic(struct fio_nic *from, uint32_t limit, int *arr_count, int *recv_again)
{
    struct netmap_ring *rxring;
    struct netmap_if *nifp = from->nifp;
    int m, recv = 0;
    while (limit > 0)
    {
        rxring = NETMAP_RXRING(nifp, from->cur_rx_head);
        if (rxring->avail == 0)
            from->cur_rx_head = ((from->cur_rx_head+1 == from->qrxlast) ?
                    from->qrxfirst : from->cur_rx_head+1);
        else
        {
            m = fio_stack_recv_pkts(rxring, from, limit, arr_count, recv_again);
            limit -= m;
            recv += m;
            if (*recv_again)
                break;
        }
    }
    if (recv > 0)
    {
        from->rxavl -= recv;
        from->rxcount += recv;
    }
    return recv;
}

inline void fio_stack_notify_workthread(struct fio_context *syscontext, int workthread_id, struct fio_notify_msg *msg)
{
    int write_wthread = 0;
    if ((write_wthread=write(syscontext->notifyfds[workthread_id], msg, sizeof(struct fio_notify_msg))) < 0)
    {
        OD( "tid %d error %d writing thread %d's sockpair", syscontext->me, errno, workthread_id);
        assert(write_wthread>=0);
    }
    else
    {
        if (sysconfig.prsys_stack)
            OD( "tid %d wrote workthread %d's  msg_id = %u, nic_id = %d, abstract = %p", 
                syscontext->me, workthread_id, msg->msg_id, msg->nic_id, (void*)msg->abstract);
    }
}
inline void fio_stack_notify_all_workthread(struct fio_context *syscontext, struct fio_notify_msg *msg)
{
    int i;
    for (i = 0; i < sysconfig.nthreads; i++)
            fio_stack_notify_workthread(syscontext, i, msg);
}
inline void fio_stack_notify_all_workthread_exceptme(struct fio_context *syscontext, int me, struct fio_notify_msg *msg)
{
    int i;
    for (i = 0; i < sysconfig.nthreads; i++)
        if (i != me)
            fio_stack_notify_workthread(syscontext, i, msg);
}
static void fio_stack_handle_msg(struct fio_context *syscontext, struct fio_notify_msg *msg, 
        struct fio_nic **sys_nics, struct fio_context *wcontexts)
{
    if (sysconfig.prsys_stack)
        OD( "tid %d msg_id = %u, nic_id = %d, abstract = %p", syscontext->me, msg->msg_id,
            msg->nic_id, (void*)msg->abstract);
    switch (msg->msg_id)
    {
        case FIO_MSG_SYSSTACK:
            fio_stack_send(sys_nics[msg->nic_id], &wcontexts->nics[msg->nic_id]);
            if (fio_rbf_occupy(&wcontexts->nics[msg->nic_id].sys_txbuf))
                sys_nics[msg->nic_id]->nifp->ni_flag |= NIF_RXNOSLEEP;
            break;
        case FIO_MSG_UPMAC:
            {
                struct fio_mac_info *old_mac_info = (struct fio_mac_info*)msg->abstract;
                int i;
                for (i = 0; i < sysconfig.nthreads; i++)
                {
                    if (i != wcontexts->me)
                    {
                        struct fio_mac_info *pmac_info = malloc(sizeof(struct fio_mac_info));
                        memcpy(pmac_info, old_mac_info, sizeof(struct fio_mac_info));
                        msg->abstract = (unsigned long)pmac_info;
                        fio_stack_notify_workthread(syscontext, i, msg);
                    }
                }
                free(old_mac_info);
            }
            break;
        case FIO_MSG_DEL_BUDDYIP:
        case FIO_MSG_CLEAR_MACPOOL:
            fio_stack_notify_all_workthread_exceptme(syscontext, wcontexts->me, msg);
            break;
        default:
            break;
    }
}
static void * fio_stack_thread(void *t)
{
    struct fio_context *context = (struct fio_context*)t;
    int num_nic = context->num_nic, enable_sysstack = sysconfig.enable_sysstack;
    int i, j, num_ready, recv_again = 0, recv_once, nthreads = sysconfig.nthreads;
    const int fast_time = 50;
    uint32_t dirs[4];
    struct fio_nic *nics[2], *cur_nic;
    struct pollfd fds[4+nthreads];
    const uint32_t recv_limit = sysconfig.burst;
    struct fio_poll_data pd;
    struct fio_notify_msg msg;
    int arr_count[nthreads];
    memset(arr_count, 0, sizeof(arr_count));

    for (i = 0; i < num_nic; i++)
    {
        nics[i] = &context->nics[i];
        dirs[i] = (FIO_DIR_RX);
    }

    NOD("tid %d Wait %d secs for phy reset", context->me, 5);
    sleep(5);
    OD( "tid %d Ready... num_nic %d", context->me, num_nic);
    gettimeofday(&context->tic, NULL);

    pd.nics = nics;
    pd.fds = fds;
    pd.dirs = dirs;
    pd.num_nic = num_nic;
    pd.notifyfds = context->notifyfds;
    pd.num_notify = 0;
    if (enable_sysstack)
        pd.num_notify = nthreads;

    while (sysconfig.working) 
    {
        num_ready = context->vtbl.polls(&pd, fast_time);
        if ( num_ready < 0) 
        {
            gettimeofday(&context->toc, NULL);
            context->toc.tv_sec -= 1; /* Subtract timeout time. */
            OD( "<error> poll error!!!!!");
            continue;
        }

        if (num_ready == 0)
        {
            for (j = 0; j < num_nic; j++)
            {
                cur_nic = nics[j];
                if (fio_nic_txlimit(cur_nic))
                {
                    for (i = 0; i < nthreads; i++)
                    {
                        fio_stack_send(cur_nic, &g_contexts[i].nics[j]);
                        if (fio_rbf_occupy(&g_contexts[i].nics[j].sys_txbuf))
                        {
                            cur_nic->nifp->ni_flag |= NIF_RXNOSLEEP;
                        }
                    }
                }

            }
            continue;
        }
         
		NOD("tid %d num_ready %d", context->me, num_ready);

		recv_again = 0;
		for (j = 0; j < num_nic; j++)
		{
			cur_nic = nics[j];
			recv_once = fio_stack_recv_nic(cur_nic, 
					(recv_limit > cur_nic->rxavl) ? cur_nic->rxavl : recv_limit,
					arr_count, &recv_again);

            for (i = 0; i < nthreads; i++)
            {
                if (recv_once > 0 && arr_count[i] > 0)
                {
                    msg.msg_id = FIO_MSG_SYSSTACK;
                    msg.nic_id = j;
                    msg.abstract = arr_count[i];
                    arr_count[i] = 0;
                    fio_stack_notify_workthread(context, i, &msg);
                    }
                    }
                }

        if (enable_sysstack)
        {
            for (i = 0; i < nthreads; i++)
            {
                if (fds[num_nic+i].revents & POLLIN)
                {
                    if (read(context->notifyfds[i], &msg, sizeof(msg)) < 0) 
                    {
                        OD( "tid %d error %d reading sockpair", context->me, errno);
                        continue;
                    }
                    fio_stack_handle_msg(context, &msg, nics, &g_contexts[i]);
                    }
                }
            }

    }

    OD( "I'll shutdown");
    return (NULL);
}

int fio_stack_start()
{
    if (sysconfig.single_thread != FIO_MODE_TEST_SND && sysconfig.enable_sysstack)
    {
        if (pthread_create(&g_syscontext->thread, NULL, fio_stack_thread, g_syscontext) == -1)
        {
            OD( "Unable to create sysstack thread");
            return -1;
        }
    }

    return 0;
}

