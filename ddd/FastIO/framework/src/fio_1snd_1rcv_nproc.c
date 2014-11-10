#include "fio_1snd_1rcv_nproc.h"
#include "fio_context.h"

struct fio_context *g_workthreads;

int fio_cb_arp(struct fio_nic *from, struct fio_nic *to, struct fio_rxdata *rxdata,
        int pre_realtime, void *param)
{
}

void fio_proc_eth(struct fio_nic *from, struct fio_nic *to, 
        struct fio_asted_rxs *bufs, int immediately, int pre_realtime, void *param)
{
    eth_handle fun;
    int i, end = bufs->avail;
    struct fio_rxdata *rxd = bufs->rxds;
    for (i = 0; i < end; i++, rxd++)
    {
        if (MyMap32_find(g_eth_handles, rxd->eh_type, (unsigned long*)&fun) && fun)
            fun(from, to, rxd, pre_realtime, param);
    }
    bufs->avail = 0;
}  

int fio_dispath(struct fio_nic *from, struct fio_nic *to, struct fio_nic **nics, int num_nic, struct fio_asted_rxs *rxds,
        int pre_realtime)
{
    int ret = 0;
    fun_proc_many fun;
    fun_proc_def fun_def;

    fio_proc_eth();

    struct fio_asted_rxs *defrxds = &rxds[T_FIO_PKT_DEF];
    struct fio_rxdata *rxdatas = defrxds->rxds;
    for (pos = 0; pos < defrxds->avail; pos++, rxdatas++)
    {
        if (MyMap32_find(g_defcbf_tbl, rxdatas->ip_type, (unsigned long*)&fun_def) && fun_def)
        {
            fun_def(from, rxdatas, NULL);
        }
    }
	defrxds->avail = 0;

    for (pos = T_FIO_PKT_INTD; pos < T_FIO_PKT_ETH; pos++, rxds++)
    {
        ND("tid %d rxds %p, intd avail %d ", NIC_EXTRA_CONTEXT(from)->me, rxds, rxds->avail);
        if (rxds->avail > 0 && MyMap32_find(g_cbf_tbl, rxds->cb_type, (unsigned long*)&fun) && fun)
        {
            fun(from, nics[0], nics[1], rxds->rxds, rxds->avail, NULL);
        }
        rxds->avail = 0;
		ND("tid %d txavail %d %d", NIC_EXTRA_CONTEXT(from)->me, from->txavl, to->txavl);
    }

    return ret;
}

static void * fio_recvthread(void *t)
{
    struct fio_context *s_ct = t;
    struct fio_nic *nics[2];
    struct pollfd fds[2];
    uint32_t dirs[2];
    struct fio_asted_rxs rxbufs[T_FIO_PKT_DISCARD];
    int limit = MAX_PKT_COPY_ONCE;
    int num_ready, i, num_nic = context->num_nic;
    struct fio_nic *rn, *tn;

    memset(rxbufs, 0, T_FIO_PKT_DISCARD*sizeof(struct fio_asted_rxs));
    for (i = T_FIO_PKT_INTD; i < T_FIO_PKT_DISCARD; i++)
        rxbufs[i].cb_type = i;

    for (i = 0; i < num_nic; i++)
    {
        nics[i] = &context->nics[i];
        dirs[i] = (FIO_DIR_RX);
    }

    D("tid %d Wait %d secs for phy reset", context->me, 5);
    sleep(5);
    D("tid %d Ready... num_nic %d", context->me, num_nic);
    gettimeofday(&context->tic, NULL);

    while (1) 
    {
        if ( (num_ready=context->vtbl.polls(nics, fds, dirs, num_nic, 2000)) < 0) 
        {
            gettimeofday(&context->toc, NULL);
            context->toc.tv_sec -= 1; 
            D("<error> poll error!!!!!");
            continue;
        }

        if (num_ready == 0)
            continue;

        for (i = 0; i < num_nic; i++)
        {
            rn = nics[i];
            tn = &s_ct->nics[i];

            if (rn->rxavl > 0)
            {
                if (context->vtbl.recv(rn, NULL, rxbufs, limit, FIO_LIMIT_FROM) > 0)
                    fio_dispath(rn, tn, nics, num_nic, rxbufs, TXDATA_TYPE_NORMAL);
            }
        }
    }

    context->used = 0;
    D("I'll shutdown");
    return NULL;
}

static void * fio_dns_send(struct fio_context *context)
{
    struct fio_nic *nics[2];
    struct pollfd fds[2];
    uint32_t dirs[2];
    int can_sleep, all_move, limit;
    int m, num_ready, i, j, num_nic = context->num_nic;
    struct netmap_if *nifp;
    struct netmap_ring *txring;
    struct fio_nic *tn;

    for (i = 0; i < num_nic; i++)
    {
        nics[i] = &context->nics[i];
        dirs[i] = FIO_DIR_TX;
    }

    D("tid %d Wait %d secs for phy reset", context->me, 5);
    sleep(5);
    D("tid %d Ready... num_nic %d", context->me, num_nic);
    gettimeofday(&context->tic, NULL);

    while (1) 
    {
        if ( (num_ready=context->vtbl.polls(nics, fds, dirs, num_nic, 500)) < 0) 
        {
            gettimeofday(&context->toc, NULL);
            context->toc.tv_sec -= 1; /* Subtract timeout time. */
            D("<error> poll error!!!!!");
            continue;
        }

        if (num_ready == 0)
        {
            continue;
        }

        for (i = 0; i < num_nic; i++)
        {
            tn = nics[i];
            fio_nic_send_arpbuf(tn);
            fio_nic_send_txbuf(tn);
            if (tn->txavl < 1)
                continue;

            all_move = 0;
            m = 0;
            nifp = tn->nifp;
            limit = tn->txavl;

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
                tn->txavl -= all_move;
            }
        }
        usleep(50000)
    }

    /* reset the ``used`` flag. */
    context->used = 0;
    D("I'll shutdown");

    return NULL;
}

int32_t fio_cb_icmpreq(struct fio_nic *nic, struct fio_rxdata *rxd, void *param)
{
    D("");
    //fprintf(stderr, "fio_cb_icmpreq\n");
    struct fio_txdata *m;

    if (fio_nic_reserve_tx(nic, &m, rxd) < 1)
        goto err;

    if (fio_icmp_gen_resp(nic, (struct icmp_packet*)rxd->pbuf, (struct icmp_packet*)(m->pbuf), rxd, TXDATA_TYPE_REALTIME))
    {
        D("tid %d generate icmp reply packet error", NIC_EXTRA_CONTEXT(nic)->me);
        goto err;
    }
    m->size = rxd->size;
    fio_nic_commit(nic, m, 1);
    return 0;
err:
    return -1;
}

int fio_enter_1snd_1rcv_nproc(int *txqueues)
{
    int i, j;
    uint32_t dirs[2] = {FIO_DIR_TX, FIO_DIR_RX};
    char *alises[2] = {"send", "recv"};
    void *mmap_addr = NULL;

    //replace handler
    MyMap32_set(g_eth_handles, g_arp_type, (unsigned long)fio_cb_arp);
    MyMap32_set(g_defcbf_tbl, IPPROTO_ICMP, (unsigned long)fio_cb_icmpreq);

    g_contexts = calloc(sysconfig.nthreads, sizeof(*g_contexts));
    __initialize_dns_packet("12345678901234567890", 20);
    for (i = 0; i < sysconfig.nthreads; i++) 
    {
        g_contexts[i].me = i;
        g_contexts[i].vtbl.polls = fio_context_polls;
        g_contexts[i].vtbl.recv = fio_context_recv_only;

        for (j = 0; j < FIO_MAX_NICS; j++) 
            g_contexts[i].nics[j].type_id = j;

        if (mmap_addr)
            g_contexts[i].nics[0].mmap_addr = mmap_addr;
        if (fio_nic_open(&g_contexts[i].nics[0], alises[i],  sysconfig.if_in, 
                    FIO_LSTN_ALL_RING, diris[i], g_contexts+i) )
        {
            D("tid %d open %s %s error!!", i, alises[i], sysconfig.if_in);
            goto err;
        }
        if (!mmap_addr)
            mmap_addr = g_contexts[i].nics[0].mmap_addr;
        g_contexts[i].num_nic++;

        if (strcmp(sysconfig.if_out, sysconfig.if_in))
        {
            g_contexts[i].nics[1].mmap_addr = g_contexts[i].nics[0].mmap_addr;
            if ( fio_nic_open(&g_contexts[i].nics[1], alises[i], sysconfig.if_out, 
                        FIO_LSTN_ALL_RING, dirs[i], g_contexts+i) )
            {
                D("tid %d open %s %s error!!", i, alises[i], sysconfig.if_out);
                goto err;
            }
            g_contexts[i].num_nic++;
        }

        struct pktudp *pkts[] = {&g_pkt4test, &g_pkt4test};
        for (j = 0; j < g_contexts[i].num_nic; j++) 
        {
#ifdef __old_mac_mgr__
            fio_mac_init(&g_contexts[i].nics[j].mac_mgr, sysconfig.vlans[j]);
#endif
            fio_nic_init(&g_contexts[i].nics[j], txqueues[j]*TXBUF_SIZE, txqueues[j]*sysconfig.burst);
            g_contexts[i].nics[j].pkt_test = pkts[j];
        }

        g_contexts[i].mac_lifetime = sysconfig.mac_lifetime;
        g_contexts[i].used = 1;
        g_contexts[i].n_test = 99999;
    }
    fio_register_timer_handler(FIO_T_CHECK_MAC, fio_brcast_mac);

    return 0;
err:
    return -1;
}

