//	$Id: fio_nic.c 2013-05-23 likunxiang$
//
#include "fio_context.h"
#include "fio_pack_misc.h"
#include "fio_route_manager.h"
#include "fio_sysconfig.h"
#include "fio_syslog.h"
#include "fio_sysstack.h"
#include "fio_map32.h"
#include "fio_mapstr.h"
#include "fio_handler.h"
#include "log_log.h"
#include <assert.h>

#define STR_SHUTDOWN "c2h1dGRvd24="
#define STR_STARTUP "c2h1dGRvd25="

//#define debug_dns

int fio_nic_do_ioctl(struct fio_nic *nic, int what, int subcmd)
{
    struct ifreq ifr;
    int error;

    struct ethtool_value eval;
    int fd;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        OD( "Error: cannot get device control socket.\n");
        return -1;
    }

    (void)subcmd;	// unused
    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, nic->nmr.nr_name, sizeof(ifr.ifr_name));
    switch (what) {
        case SIOCSIFFLAGS:
            ifr.ifr_flagshigh = nic->if_flags >> 16;
            ifr.ifr_flags = nic->if_flags & 0xffff;
            break;

        case SIOCETHTOOL:
            if (subcmd != ~ETH_FLAG_LRO && subcmd != ETH_FLAG_LRO)
            {
                eval.cmd = subcmd;
                eval.data = 0;
                ifr.ifr_data = (caddr_t)&eval;
            }
            else
            {
                eval.cmd = ETHTOOL_GFLAGS;
                eval.data = 0;
                ifr.ifr_data = (caddr_t)&eval;
            }
            break;
    }
    error = ioctl(fd, what, &ifr);
    if (error)
        goto done;
    switch (what) {
        case SIOCGIFFLAGS:
            nic->if_flags = (ifr.ifr_flagshigh << 16) |
                (0xffff & ifr.ifr_flags);
            OD( "nic %s flags are 0x%x", ifr.ifr_name, nic->if_flags);
            break;

        case SIOCETHTOOL:
            if (subcmd == ~ETH_FLAG_LRO)
            {
                eval.cmd = ETHTOOL_SFLAGS;
                eval.data &= subcmd;
                error = ioctl(fd, what, &ifr);
            }
            else if (subcmd == ETH_FLAG_LRO)
            {
                eval.cmd = ETHTOOL_SFLAGS;
                eval.data |= subcmd;
                error = ioctl(fd, what, &ifr);
            }
            break;
    }
done:
    close(fd);
    if (error)
        OD( "ioctl error %d %d", error, what);
    return error;
}

static inline int fio_nic_include_txd(struct fio_nic **nic, struct fio_txdata *txd)
{
    if (nm_likely(((struct fio_ringbuf*)txd->parent)->owner != *nic))
    {
        OD( "<ERROR> tid %d txd's owner %p != nic %p, set nic to txd's owner",
                NIC_EXTRA_CONTEXT(*nic)->me, ((struct fio_ringbuf*)txd->parent)->owner, *nic);
        assert(((struct fio_ringbuf*)txd->parent)->owner == *nic);
        *nic = ((struct fio_ringbuf*)txd->parent)->owner;
        return 0;
    }
    return 1;
}

int fio_nic_recv(struct fio_nic *from, struct fio_asted_rxs *bufs, uint32_t limit);

inline int fio_nic_ioctltx(struct fio_nic *nic)
{
    ioctl(nic->fd, NIOCTXSYNC, NULL);
    return 0;
}

static int fio_nic_filter_specip(struct fio_nic *nic, char *pkt, int pkt_size)
{
    if (pkt_size > 42 && *(uint32_t*)(pkt+g_dip_offset) == (uint32_t)nic->spec_ip.s_addr)
    {
        char str_dip_debug[16];
        inet_ntop(AF_INET, (uint32_t*)(pkt+g_dip_offset), str_dip_debug, 16);
        OD( "tid %d nic %s catch a spec ip %s's pkt first2byte %d", NIC_EXTRA_CONTEXT(nic)->me,
			nic->alise, str_dip_debug, ntohs(*(uint16_t*)(pkt+42)));
        struct fio_rxdata *pb = nic->spec_rxpkts.rxds + nic->spec_rxpkts.avail;
        pb->eh_type = *(uint16_t*)(pkt+g_eth_type_offset);
        pb->ip_type = *(uint8_t*)(pkt+g_prototype_offset);
        pb->smac = *(struct ether_addr*)(pkt+g_eth_smac);
        pb->sip = *(uint32_t*)(pkt+g_sip_offset);
        pb->dip = *(uint32_t*)(pkt+g_dip_offset);
        pb->sport = *(uint16_t*)(pkt+g_sport_offset);
        pb->dport = *(uint16_t*)(pkt+g_recvport_offset);
        pb->size = pkt_size;
        memcpy(pb->pbuf, pkt, pkt_size);
        nic->spec_rxpkts.avail++;
        return 1;
    }
    return 0;
} 
int fio_nic_send_pkts(struct netmap_ring *ring, struct fio_nic *nic, struct fio_ringbuf *rbf,
        int txd_type, uint32_t count)
{
    char *p;
    struct netmap_slot *slot;
    uint32_t pkt_bufidx;
    uint32_t sent, skip = 0, cur = ring->cur;

    struct fio_txdata *_head = rbf->bf_head;
    struct fio_txdata *_tail = rbf->bf_tail;
    struct fio_txdata *_end = rbf->bf_end;
    struct fio_txdata *_txbufs = rbf->bf_bufs;

    if (ring->avail < count)
        count = ring->avail;

    for (sent = 0; sent < count && !_fio_rbf_empty(_head, _tail); 
            sent++, fio_rbf_seek(&_head, _end, _txbufs, 1))
    {
        slot = &ring->slot[cur];

        switch (txd_type)
        {
            case TXDATA_TYPE_REALTIME:
                pkt_bufidx = slot->buf_idx;
                slot->buf_idx = _head->slot_rx->buf_idx;
                _head->slot_rx->buf_idx = pkt_bufidx;
                break;
            case TXDATA_TYPE_SYSSTACK_RX:
                if (nic->spec_ip.s_addr != 0 && fio_nic_filter_specip(nic, _head->pbuf, _head->size))
                {
                    skip++;
                    continue;
                }
                p = NETMAP_BUF(ring, slot->buf_idx);
                memcpy(p, _head->pbuf, _head->size);
                break;

            case TXDATA_TYPE_PKT_CACHE:
                OD( "copy pmac");
                bcopy(rbf->pmac, ((struct pktudp*)_head->pbuf)->eh.ether_dhost, ETH_ALEN);
            default:
                p = NETMAP_BUF(ring, slot->buf_idx);
                memcpy(p, _head->pbuf, _head->size);
                break;
        }

        slot->len = _head->size;

#ifdef debug_dns
        p = NETMAP_BUF(ring, slot->buf_idx);
        if (*(uint16_t*)(p+g_recvport_offset) == htons(53) ||
				*(uint16_t*)(p+g_recvport_offset) == htons(29876) ||
                *(uint16_t*)(p+g_sport_offset) == htons(53))
        {
            char str_sip[16];
            char str_dip[16];
            inet_ntop(AF_INET, (uint32_t*)(p+g_sip_offset), str_sip, 16);
            inet_ntop(AF_INET, (uint32_t*)(p+g_dip_offset), str_dip, 16);
            OD( "tid %d snd pkt len %d sip %s, sport %d, dip %s, dport %d", 
                    NIC_EXTRA_CONTEXT(nic)->me, slot->len, 
                    str_sip, ntohs(*(uint16_t*)(p+g_sport_offset)),
                    str_dip, ntohs(*(uint16_t*)(p+g_recvport_offset)));

            char str_x[1500] = {0};
            char str_byte[10];
            int i;
            for (i = 0; i < slot->len; i++)
            {
                snprintf(str_byte, 10, "%02X ", (unsigned char)p[i]);
                if (i%8 == 0)
                    strncat(str_x, "\n", 1500);
                strncat(str_x, str_byte, 1500);
            }
            OD( "%s", str_x);
        }
#endif

        //if (sent == count - 1)
        //    slot->flags |= NS_REPORT;
        cur = NETMAP_RING_NEXT(ring, cur);
    }

    fio_rbf_consume(rbf, sent);

    ring->avail -= (sent-skip);
    ring->cur = cur;

    return sent;
}

inline int fio_nic_send_pktcache(struct fio_nic *nic)
{
    char *p;
    struct netmap_slot *slot;
    struct netmap_if *nifp = nic->nifp;
    struct netmap_ring *txring;
    int limit = nic->nictxavl, sent = 0;
    uint32_t cur;
    struct fio_mac_pkt_data *ptr_data = NULL;
    struct fio_list_head *list_cache = &nic->pktcache_list_nic[nic->type_id];
    if (nm_unlikely(limit == 0))
    {
        NOD("tid %d %s nictxavl %d! full!!", NIC_EXTRA_CONTEXT(nic)->me, nic->alise, 0);
        return sent;
    }
    txring = NETMAP_TXRING(nifp, nic->cur_tx_head);
    while (limit > 0)
    {
        if (txring->avail == 0)
        {
            nic->cur_tx_head = (nic->cur_tx_head+1 == nic->qtxlast ? 
                    nic->qtxfirst : nic->cur_tx_head+1);
            txring = NETMAP_TXRING(nifp, nic->cur_tx_head);
        }
        else
        {
            if (fio_list_empty(list_cache)) 
                break;
            ptr_data = fio_list_entry(list_cache->next, struct fio_mac_pkt_data, nic_link);
            fio_list_del(&ptr_data->nic_link);
            cur = txring->cur;
            slot = &txring->slot[cur];
            bcopy(&ptr_data->mac, ((struct pktudp*)ptr_data->data)->eh.ether_dhost, ETH_ALEN);
            p = NETMAP_BUF(txring, slot->buf_idx);
            memcpy(p, ptr_data->data, ptr_data->size);
            slot->len = ptr_data->size;
            fio_mac_pkt_data_del(nic->buddy_mac_pkt_factory, ptr_data);
            txring->cur = NETMAP_RING_NEXT(txring, cur);
            limit--;
            txring->avail--;
            sent++;
        }
    }
    nic->nictxavl -= sent;
    nic->txcount += sent;
    return sent;
}
inline int fio_nic_send(struct fio_nic *nic, int txd_type, int limit, void *param)
{
    struct netmap_if *nifp = nic->nifp;
    struct netmap_ring *txring;
    int m, sent = 0;
    struct fio_ringbuf *rbf;

    if (nm_unlikely(nic->nictxavl == 0))
    {
        NOD("tid %d %s nictxavl %d", NIC_EXTRA_CONTEXT(nic)->me, nic->alise, 0);
        return sent;
    }
    if (limit > nic->nictxavl)
    {
        limit = nic->nictxavl;
        if (nm_unlikely(TXDATA_TYPE_REALTIME == txd_type))
            OD( "tid %d %s limit %d, nictxavl %u realtime %d loss packet", NIC_EXTRA_CONTEXT(nic)->me,
                    nic->alise, limit, nic->nictxavl, txd_type);
    }      

    switch(txd_type)
    {
        case TXDATA_TYPE_REALTIME:
            rbf = &nic->bg_buf;
            break;
        case TXDATA_TYPE_PKT_CACHE:
            rbf = (struct fio_ringbuf*)param;
            break;
        case TXDATA_TYPE_PKT_ARP:
            rbf = &nic->arp_buf;
            break;
        case TXDATA_TYPE_SYSSTACK_TX:
            rbf = &nic->sys_txbuf;
            break;
        case TXDATA_TYPE_SYSSTACK_RX:
            rbf = &nic->sys_rxbuf;
            break;
        case TXDATA_TYPE_UPSND:
            rbf = &nic->upsnd_buf;
            break;
        default:
            rbf = &nic->tx_buf;
            break;
    }
    while (limit > 0)
    {
        txring = NETMAP_TXRING(nifp, nic->cur_tx_head);
        if (txring->avail == 0)
            nic->cur_tx_head = (nic->cur_tx_head+1 == nic->qtxlast ? 
                    nic->qtxfirst : nic->cur_tx_head+1);
        else
        {
            m = fio_nic_send_pkts(txring, nic, rbf, txd_type, limit);
            limit -= m;
            sent += m;
        }
    }
    //if (nm_likely(sent > 0))
    //{
    nic->nictxavl -= sent;
    nic->txcount += sent;
    //}
    return sent;
}

inline int fio_nic_txlimit(struct fio_nic *nic)
{
    int di, tx_limit = 0;
    struct netmap_if *txnifp = nic->nifp;
    struct netmap_ring *txring;
    for (di = nic->qtxfirst; di < nic->qtxlast; di++) 
    {
        txring = NETMAP_TXRING(txnifp, di);
        tx_limit += txring->avail;
    }
    nic->txslow = tx_limit/5;
    nic->txfast = tx_limit-nic->txslow;
    nic->nictxavl = tx_limit;
    NOD("%s snd limit %d, slow %d, fast %d", nic->alise, tx_limit, nic->txslow, nic->txfast);
    return nic->nictxavl;
}

inline int fio_nic_rxlimit(struct fio_nic *nic)
{
    int di, rx_limit = 0;
    struct netmap_if *rxnifp = nic->nifp;
    struct netmap_ring *rxring;
    for (di = nic->qrxfirst; di < nic->qrxlast; di++) 
    {
        rxring = NETMAP_RXRING(rxnifp, di);
        rx_limit += rxring->avail;
    }
    nic->rxavl = rx_limit;
    NOD("%s recv limit %d", nic->alise, rx_limit);
    return rx_limit;
}

int fio_nic_backoff(struct fio_nic *nic, int buf_type, void *param)
{
    struct fio_context *context = NIC_EXTRA_CONTEXT(nic);
    struct fio_ringbuf *_buf = NULL;
    int _half = 0, sent; 

    switch (buf_type)
    {
        case TXDATA_TYPE_PKT_ARP:
            _buf = &nic->arp_buf;
            break;
        case TXDATA_TYPE_NORMAL:
            _buf = &nic->tx_buf;
            break;
        case TXDATA_TYPE_PKT_CACHE:
            _buf = (struct fio_ringbuf*)param;
            break;
        default:
            OD( "tid %d: tx type error", context->me);
            assert(0);
            break;
    }
    _half = fio_rbf_occupy(_buf)/2;

    NOD("tid %d: tx cache full!! flush action, avail %d",
            context->me, fio_rbf_avail(_buf)); 
    context->txblock = 1;
    while (_half > 0)
    {
        sent = nic->vtbl.send(nic, buf_type, _half, param);
        _half -= sent;
        fio_nic_ioctltx(nic);
        fio_nic_txlimit(nic);
        OD( "tid %d: flush %d", context->me, sent); 
        //usleep(10);
    }
    //ND("tid %d: tx cache empty %d!!", context->me, fio_rbf_empty(_buf)); 
    //usleep(5);
    context->txblock = 0;

    return 0 ;
}

inline int fio_nic_commit(struct fio_nic *nic, struct fio_txdata *txd, int count)
{
    int ret = 0;
    struct fio_ringbuf *rbf = NULL;
    switch (txd->type)
    {
        case TXDATA_TYPE_REALTIME:
            rbf = &nic->bg_buf;
            break;
        case TXDATA_TYPE_PKT_ARP:
            rbf = &nic->arp_buf;
            break;
        case TXDATA_TYPE_SYSSTACK_TX:
            rbf = &nic->sys_txbuf;
            break;
        case TXDATA_TYPE_SYSSTACK_RX:
            rbf = &nic->sys_rxbuf;
            break;
        case TXDATA_TYPE_UPSND:
            rbf = &nic->upsnd_buf;
            break;
        default:
            rbf = &nic->tx_buf;
            break;
    }
    assert(rbf == txd->parent);
    if (rbf != txd->parent)
    {
        goto err;
    }

    ret = fio_rbf_commit(rbf, count);

go_out:
    return ret;
err:
    ret = -1;
    goto go_out;
}

inline int fio_upsend_reserve_tx(int nic_id, struct fio_txdata **m)
{
    return fio_stack_reserve_tx(&g_contexts->nics[nic_id].upsnd_buf, m);
}

inline int fio_nic_reserve_tx(struct fio_nic *nic, struct fio_txdata **m, void *param)
{
    int realtime; // = (param ? TXDATA_TYPE_REALTIME : TXDATA_TYPE_NORMAL);
    int ret = 0;
    if (param && nic->nictxavl - fio_rbf_occupy(&nic->bg_buf) > 0)
    {
        realtime = TXDATA_TYPE_REALTIME;
        ret = fio_rbf_reserve_one(&nic->bg_buf, m);
    }
    else
    {
        realtime = TXDATA_TYPE_NORMAL;
        ret = fio_rbf_reserve_one(&nic->tx_buf, m);
    }

    if (ret != 1)
    {
        if (TXDATA_TYPE_NORMAL != realtime) 
            return ret;

        if (fio_rbf_size(&nic->tx_buf) < 2*TXBUF_SIZE)
        {
            if (0 != fio_rbf_expand(&nic->tx_buf, NIC_EXTRA_CONTEXT(nic)->me))
            {
                OD( "pkt normal ring buf expend error!!!!");
                fio_nic_backoff(nic, TXDATA_TYPE_NORMAL, NULL);
#if 1
                assert(0);
#endif
            }
        }
        else
            fio_nic_backoff(nic, TXDATA_TYPE_NORMAL, NULL);
        ret = fio_rbf_reserve_one(&nic->tx_buf, m);
    }

    (*m)->vtbl.set_buf_ptr((*m), (struct fio_rxdata*)param);
    (*m)->vtbl.reset(*m);
    return ret;
}

inline int fio_nic_reserve_arp(struct fio_nic *nic, struct fio_txdata **m)
{
    int ret = fio_rbf_reserve_one(&nic->arp_buf, m);

    if (ret != 1)
    {
        if (fio_rbf_size(&nic->arp_buf) < 2*TXBUF_SIZE)
        {
            if (0 != fio_rbf_expand(&nic->arp_buf, NIC_EXTRA_CONTEXT(nic)->me))
            {
                OD( "arp ring buf expend error!!!!");
                fio_nic_backoff(nic, TXDATA_TYPE_PKT_ARP, NULL);
#if 1
                assert(0);
#endif
            }
        }
        else
            fio_nic_backoff(nic, TXDATA_TYPE_PKT_ARP, NULL);
        ret = fio_rbf_reserve_one(&nic->arp_buf, m);
    }

    (*m)->vtbl.reset(*m);
    return ret;
}

int fio_recv_pkts(struct netmap_ring *ring, struct fio_nic *nic, 
        struct fio_asted_rxs *bufs, uint32_t limit)
{
    uint32_t cur, pkts;
    struct netmap_slot *slot;
    char *p;
    struct fio_rxdata *pbs[T_FIO_PKT_DISCARD];
    struct fio_rxdata *pb;
    int rx = 0, pkt_type, useless_pkt = 0, sysstack_pkt = 0;
    uint8_t ip_type = 0;
    uint16_t eh_type = 0;

    for (cur = 0; cur < T_FIO_PKT_DISCARD; cur++)
        pbs[cur] = bufs[cur].rxds + bufs[cur].avail;

    cur = ring->cur;
    if (ring->avail < limit)
        limit = ring->avail;

    for (pkts = 0; pkts < limit; pkts++) 
    {
        slot = &ring->slot[cur];
        if (slot->len < 42 )
        {
            useless_pkt++;
            OD( "tid %d nic %s discard! too short", 
                    NIC_EXTRA_CONTEXT(nic)->me, nic->alise);
            goto _pkt_discard;
        }
        p = NETMAP_BUF(ring, slot->buf_idx);
        nm_prefetch(p);
        pkt_type = fio_assort(nic, p, &eh_type, &ip_type);
        if (T_FIO_PKT_DISCARD == pkt_type)
        {
            useless_pkt++;
            goto _pkt_discard;
        }
        else if (T_FIO_PKT_MAX == pkt_type)
        {
            sysstack_pkt++;
            NOD("tid %d discard! pkt useless len %d sip %s, sport %d, dip %s, dport %d", 
                    NIC_EXTRA_CONTEXT(nic)->me, slot->len, 
                    str_sip, ntohs(*(uint16_t*)(p+g_sport_offset)),
                    str_dip, ntohs(*(uint16_t*)(p+g_recvport_offset)));
            goto _pkt_sysstack;
        }

        NOD("nooocopy**********");
        pb = pbs[pkt_type];
        pb->eh_type = eh_type;
        pb->ip_type = ip_type;
        pb->smac = *(struct ether_addr*)(p+g_eth_smac);
        pb->sip = *(uint32_t*)(p+g_sip_offset);
        pb->dip = *(uint32_t*)(p+g_dip_offset);
        pb->sport = *(uint16_t*)(p+g_sport_offset);
        pb->dport = *(uint16_t*)(p+g_recvport_offset);

#ifdef debug_dns
        if (pb->dport == htons(53))
        {
            char str_sip[16];
            char str_dip[16];
            inet_ntop(AF_INET, (uint32_t*)(p+g_sip_offset), str_sip, 16);
            inet_ntop(AF_INET, (uint32_t*)(p+g_dip_offset), str_dip, 16);
            OD( "tid %d recv pkt len %d sip %s, sport %d, dip %s, dport %d", 
                    NIC_EXTRA_CONTEXT(nic)->me, slot->len, 
                    str_sip, ntohs(*(uint16_t*)(p+g_sport_offset)),
                    str_dip, ntohs(*(uint16_t*)(p+g_recvport_offset)));

            char str_x[1500] = {0};
            char str_byte[10];
            int i;
            for (i = 0; i < slot->len; i++)
            {
                snprintf(str_byte, 10, "%02X ", (unsigned char)p[i]);
                if (i%8 == 0)
                    strncat(str_x, "\n", 1500);
                strncat(str_x, str_byte, 1500);
            }
            OD( "%s", str_x);
        }
#endif

        pb->pbuf = p;
        pb->slot_rx = slot;
        pb->size = slot->len;
        pbs[pkt_type]++;
        rx++;

        sysconfig.maclog.vtbl.print(&sysconfig.maclog, "tid %d nic %s dport %d\n", 
                NIC_EXTRA_CONTEXT(nic)->me, nic->alise, ntohs(pb->dport));

        if (!strncmp((const char*)pb->pbuf+g_payload_offset, STR_SHUTDOWN, strlen(STR_SHUTDOWN)+1))
        {   
            OD( "tid %d recv shutdown!!!!!!\n\n\n\n\n", NIC_EXTRA_CONTEXT(nic)->me);
            sysconfig.working = 2;
        }  
        else if (!strncmp((const char*)pb->pbuf+g_payload_offset, STR_STARTUP, strlen(STR_STARTUP)+1))
        {   
            OD( "tid %d recv startup!!!!!!\n\n\n\n\n", NIC_EXTRA_CONTEXT(nic)->me);
            sysconfig.working = 1;
        }  

_pkt_discard:
        cur = NETMAP_RING_NEXT(ring, cur);
        continue;
_pkt_sysstack:
        if (sysconfig.enable_sysstack)
            if (fio_stack_send_up(nic, p, slot->len) > 0)
        {
            pbs[pkt_type]++;
	    rx++;
        }
            goto _pkt_discard;
    }
    if (nm_likely(pkts > 0))
    {
        if (sysstack_pkt > 0)
        {
            struct fio_notify_msg _msg;
            _msg.msg_id = FIO_MSG_SYSSTACK;
            _msg.nic_id = nic->type_id;
            _msg.abstract = sysstack_pkt;
            fio_stack_notify_systhread(nic, &_msg);
        }
        nic->unuse_count += useless_pkt;
        nic->sysstack_count += sysstack_pkt;
        ring->avail -= pkts;
        ring->cur = cur;
        NOD("tid %s avail %u, cur %u, pkts %u", NIC_EXTRA_CONTEXT(nic)->me, nic->alise, ring->avail,
			ring->cur, pkts);
        if (rx > 0)
        {
            for (cur = 0; cur < T_FIO_PKT_DISCARD; cur++)
                bufs[cur].avail = pbs[cur] - bufs[cur].rxds;
            NOD("tid %d bufs[T_FIO_PKT_MAX] avail = %d", NIC_EXTRA_CONTEXT(nic)->me, bufs[T_FIO_PKT_MAX].avail);
        }
    }

    NOD("%s pkts %u, limit %u", nic->alise, pkts, limit);
    return pkts;
}

inline struct fio_map_t * fio_nic_get_localips(struct fio_nic *nic)
{
   return nic->mp_ipinfo.if_localaddrs;
}
inline int fio_nic_is_myip(struct fio_nic *nic, uint32_t ip_network)
{
    struct fio_map_t *map_lips = fio_nic_get_localips(nic);
    if (!map_lips || !fio_map_exist(map_lips, ntohl(ip_network)))
        return 0;
    return 1;
}

inline struct fio_mapstr_t * fio_nic_get_arpa(struct fio_nic *nic)
{
    return nic->mp_ipinfo.map_arpa;
}
inline int fio_nic_is_myarpa(struct fio_nic *nic, const char *strkey)
{
    struct fio_mapstr_t *map_arpa = fio_nic_get_arpa(nic);
    if (!map_arpa || !fio_mapstr_exist(map_arpa, strkey))
        return 0;
    return 1;
}
int fio_nic_snd_alldomain_mac(struct fio_nic *nic)
{
    struct fio_mp_ipinfo *mp_ipinfo = &nic->mp_ipinfo;
    struct fio_mp_mask **arr_mask_idx = mp_ipinfo->arr_mask_idx;
    struct fio_map_t *buddy_ips = NULL;
    struct fio_node_t *nd;
    int i, stat = 0, dev_idx, find_route;
    uint32_t dip, sip, nip;
    struct fio_mac_pkt_cache *ptr_cache;
    struct fio_context *context = NIC_EXTRA_CONTEXT(nic);
    struct in_addr sin_sip, sin_dip;
    char str_dip_debug[16], str_sip_debug[16];
    struct fio_notify_msg msg;
    for (i = 0; i < mp_ipinfo->mask_cnt; i++)
    {
        if (arr_mask_idx[i]->flag == fio_mp_mask_off || !(buddy_ips=arr_mask_idx[i]->buddy_ips))
            continue;
        for (nd = fio_map_first(buddy_ips); nd;)
        {
	    find_route = -1;
            dip = (uint32_t)nd->key;
            sin_dip.s_addr = htonl(dip);
            inet_ntop(AF_INET, &sin_dip, str_dip_debug, 16);
            if (!fio_map_find_ptr(nic->buddy_macs, (unsigned long)dip, (unsigned long*)&ptr_cache)
                    || ptr_cache->mac_info.stat == MAC_NOTEXIST)
            {
                msg.msg_id = FIO_MSG_DEL_BUDDYIP;
		msg.nic_id = nic->type_id;
                msg.abstract = (unsigned long)dip;
                fio_stack_notify_systhread(nic, &msg);
                OD( "tid %d nic %s del ip %s from buddy_ips for can't find it from buddy_macs",
                        context->me, nic->alise, str_dip_debug);
                nd = fio_map_next(buddy_ips, nd);
                fio_map_del(buddy_ips, (unsigned long)dip);
                fio_mac_del_buddy_mac(nic, dip);
                continue;
            } 
            stat = ptr_cache->mac_info.stat;
            if (((stat != MAC_EXIST) && (ptr_cache->mac_info.fail_time+1 >= sysconfig.mac_try))
			    || 0 != (find_route=fio_route_find(nic, dip, &sip, &dev_idx, &nip)))
            {
                ptr_cache->mac_info.fail_time++;
                msg.msg_id = FIO_MSG_DEL_BUDDYIP;
		msg.nic_id = nic->type_id;
                msg.abstract = (unsigned long)dip;
                fio_stack_notify_systhread(nic, &msg);
                OD( "tid %d nic %s ip %s dead! stat %d->%d, fail_time %d del it! %s",
                        context->me, nic->alise, str_dip_debug, stat, MAC_NOTEXIST, ptr_cache->mac_info.fail_time,
			find_route?"":"unfound route");
                nd = fio_map_next(buddy_ips, nd);
                fio_map_del(buddy_ips, (unsigned long)dip);
                fio_mac_del_buddy_mac(nic, dip);
                continue;
            }
            if (dev_idx != nic->type_id)
            {
                OD( "tid %d nic %s the route require nic %d skip!", context->me, nic->alise, dev_idx);
                nd = fio_map_next(buddy_ips, nd);
                continue;
            }
            ptr_cache->mac_info.fail_time++;
            if (stat == MAC_EXIST)
            {
                ptr_cache->mac_info.fail_time = 0;
                ptr_cache->mac_info.stat = MAC_UPDATING;
            }
            {
		    struct fio_mac_info *pmac_info = malloc(sizeof(struct fio_mac_info));
		    msg.msg_id = FIO_MSG_UPMAC;
		    msg.nic_id = nic->type_id;
                memcpy(pmac_info, &ptr_cache->mac_info, sizeof(struct fio_mac_info));
		    msg.abstract = (unsigned long)pmac_info;
		    fio_stack_notify_systhread(nic, &msg);
            }
            sin_sip.s_addr = htonl(sip);
            inet_ntop(AF_INET, &sin_sip, str_sip_debug, 16);
            OD( "tid %d nic %s check exist ip %s use sip %s dev %d stat %d->%d, fail_time %d",
                    context->me, nic->alise, str_dip_debug, str_sip_debug, dev_idx, stat,
                    ptr_cache->mac_info.stat, ptr_cache->mac_info.fail_time);
            if (sysconfig.prmac_info)
            {
                sysconfig.maclog.vtbl.print(&sysconfig.maclog, "tid %d nic %s, sip %s, dip %s --> %02x:%02x:%02x:%02x:%02x:%02x\n",
                        context->me, nic->alise, str_sip_debug, str_dip_debug,
                        ptr_cache->mac_info.mac.ether_addr_octet[0], ptr_cache->mac_info.mac.ether_addr_octet[1], 
                        ptr_cache->mac_info.mac.ether_addr_octet[2], ptr_cache->mac_info.mac.ether_addr_octet[3], 
                        ptr_cache->mac_info.mac.ether_addr_octet[4], ptr_cache->mac_info.mac.ether_addr_octet[5]);
            }
            fio_nic_send_one_mac(nic, ARPOP_REQUEST, NULL, sin_dip.s_addr, sin_sip.s_addr,
                    TXDATA_TYPE_PKT_ARP, NULL);
            nd = fio_map_next(buddy_ips, nd);
        }
    }
    return 0;
}

int fio_nic_send_one_mac(struct fio_nic *nic, uint16_t op, struct ether_addr *recv_mac,
        uint32_t recv_ip, uint32_t snd_ip, int realtime, void *param)
{
    struct fio_txdata *m; 
    struct in_addr sin_recv, sin_snd;

    NOD("tid %d recvip 0x%X, sndip 0x%X", NIC_EXTRA_CONTEXT(nic)->me, recv_ip, (uint32_t)nic->if_ip.s_addr);
    switch (realtime)
    {
        case TXDATA_TYPE_PKT_ARP:
            if (fio_nic_reserve_arp(nic, &m) < 1)
            {
                OD( "tid %d alloc arp error", NIC_EXTRA_CONTEXT(nic)->me);
                goto err;
            }
            break;
        case TXDATA_TYPE_REALTIME:
            if (fio_nic_reserve_tx(nic, &m, param) < 1)
            {       
                OD( "tid %d alloc tx error", NIC_EXTRA_CONTEXT(nic)->me);
                goto err;
            }
            break;
        default:
            OD( "tid %d alloc tx type error", NIC_EXTRA_CONTEXT(nic)->me);
            goto err;
    }

    sin_recv.s_addr = recv_ip;
    sin_snd.s_addr = snd_ip;

    NOD("fio_mac_gen_pack");
    if (fio_mac_gen_pack((struct arp_packet*)(m->pbuf), &sin_snd, &nic->if_mac, &sin_recv, 
                recv_mac, op))
    {
        OD( "tid %d generate arp packet error", NIC_EXTRA_CONTEXT(nic)->me);
        goto err;
    }

    m->size = g_arppkt_size;
    fio_nic_commit(nic, m, 1);

    return 0;
err:
    return -1;
}

int fio_nic_sendmsg_direct(struct fio_nic *nic, uint32_t dstip, struct fio_txdata *txd)
{ 
    //uint32_t nip, get_ret = 0;
    //struct ether_addr *dstmac = NULL;
    //if (0 != next_hop(dstip, &nip) || 0 != (get_ret=get_mac_info(nic, nip, &dstmac)))
    //    return get_ret;

    memcpy(txd->pbuf, nic->pkt_test, sysconfig.pkt_size);
    //struct pkt *ppkt = (struct pkt*)txd->pbuf;
    //struct ether_header *eh = &ppkt->eh;
    //bcopy(&sysconfig.dst_mac.start, eh->ether_dhost, ETH_ALEN);
    txd->size = sysconfig.pkt_size;
    //if (nm_unlikely(eh->ether_type == g_arp_type)
    //{
    //    ND("tid %d: reset pkt from arp", t->me);
    //    memcpy(txd->pbuf, &g_pkt4test, sysconfig.pkt_size);
    //    txd->size = sysconfig.pkt_size;
    //}

    //struct ip *ip = &ppkt->ip;
    //ip->ip_dst.s_addr = dstip;
    ////checksum??
    //ip->ip_sum = 0;
    ////ip->ip_sum = checksum((uint16_t*)ip, sizeof(struct iphdr)/sizeof(uint16_t));
    //ip->ip_sum = ip_fast_csum((char*)ip, ip->ip_hl);
    fio_nic_commit(nic, txd, 1);
    return 0;
}

inline void fio_notify_upthread(struct fio_context *_context, struct fio_notify_msg *msg)
{
    int write_wthread = 0;
    int fd_id = 2;
    if (!sysconfig.enable_sysstack)
        fd_id--;
    if ((write_wthread=write(_context->notifyfds[fd_id], msg, sizeof(struct fio_notify_msg))) < 0)
    {
        OD( "tid %d error %d writing thread %d's sockpair", _context->me, errno, fd_id);
        assert(write_wthread>=0);
    }
    else
    {
        if (sysconfig.prsys_stack)
            OD( "tid %d wrote workthread %d's  msg_id = %u, nic_id = %d, abstract = %p", 
                _context->me, fd_id, msg->msg_id, msg->nic_id, (void*)msg->abstract);
    }
}
void fio_flushup(int nic_id)
{
    struct fio_notify_msg msg;
    msg.msg_id = FIO_MSG_UPSNDBUF;
    msg.nic_id = nic_id;
    msg.abstract = 1;
    fio_notify_upthread(g_contexts, &msg);
}
int fio_upsend(int nic_id, uint16_t data_len, struct fio_txdata *txd, int need_csum)
{
    uint32_t nip, get_ret = 0, sip = 0;
    int snd_ret = 0, dev_idx;
    struct pktudp *ppkt = (struct pktudp*)txd->pbuf;
    struct ether_header *eh = &ppkt->eh;
    struct ip *ip = &ppkt->ip;
    struct udphdr *udp = &ppkt->udp;
    int pkt_size = g_payload_offset+ntohs(data_len);
    struct fio_nic *dnic = &g_contexts->nics[nic_id];
    if (txd->type != TXDATA_TYPE_UPSND)
        goto err;
    if (!txd->dmac)
    {
        if (0 != fio_route_find(dnic, ntohl(txd->dstip), &sip, &dev_idx, &nip))
            goto err;
        if (dev_idx != dnic->type_id)
            goto err;
        get_ret = fio_mac_get_bynip(dnic, nip, &txd->dmac, sip);
        ip->ip_src.s_addr = htonl(sip);
    }
    if (pkt_size > MAX_PKT_SIZE)
    {
        pkt_size = MAX_PKT_SIZE;
        data_len = htons(MAX_PAYLOAD_SIZE);
    }
    ip->ip_len = ntohs(pkt_size - sizeof(*eh));
    ip->ip_dst.s_addr = txd->dstip;
    if (0 == sip)
        ip->ip_src.s_addr = txd->srcip;
    ip->ip_sum = 0;
    ip->ip_sum = ip_fast_csum((char*)ip, ip->ip_hl);
    udp->uh_sport = txd->srcport;
    udp->uh_dport = txd->dstport;
    udp->uh_ulen = htons(pkt_size-g_sport_offset);
    udp->uh_sum = 0;
    if (need_csum)
        fixCheckSumUDP(udp, (struct iphdr*)ip, ppkt->body);
    txd->size = pkt_size;
    bcopy(&dnic->if_mac, eh->ether_shost, ETH_ALEN);
    if (0 == get_ret)
    {
        bcopy(txd->dmac, eh->ether_dhost, ETH_ALEN);
        snd_ret = fio_nic_commit(dnic, txd, 1);
        //struct fio_notify_msg msg;
        //msg.msg_id = FIO_MSG_UPSNDBUF;
        //msg.nic_id = dnic->type_id;
        //msg.abstract = 1;
        //fio_notify_upthread(NIC_EXTRA_CONTEXT(dnic), &msg);
    }
go_out:
   return snd_ret;
err:
   snd_ret = -1; 
   goto go_out;
}
int fio_send(struct fio_nic *dnic, uint16_t data_len, struct fio_txdata *txd, int need_csum)
{ 
#ifdef debug_dns
    OD( "tid %d %s send pkt", NIC_EXTRA_CONTEXT(dnic)->me, dnic->alise); 
#endif
    uint32_t nip, get_ret = 0, sip = 0;
    int snd_ret = 0, dev_idx, chnic = 0;
    struct pktudp *ppkt = (struct pktudp*)txd->pbuf;
    struct ether_header *eh = &ppkt->eh;
	
	//稳定后要置0

    struct ip *ip = &ppkt->ip;
    struct udphdr *udp = &ppkt->udp;
    int pkt_size = g_payload_offset+ntohs(data_len);

    if (!txd->dmac)
    {
        if (0 != fio_route_find(dnic, ntohl(txd->dstip), &sip, &dev_idx, &nip))
            goto err;
        if (dev_idx != dnic->type_id)
    {
            dnic = &NIC_EXTRA_CONTEXT(dnic)->nics[dev_idx];
            chnic = 1;
}
        get_ret = fio_mac_get_bynip(dnic, nip, &txd->dmac, sip);
        ip->ip_src.s_addr = htonl(sip);
    }
    if (pkt_size > MAX_PKT_SIZE)
    {
        pkt_size = MAX_PKT_SIZE;
        data_len = htons(MAX_PAYLOAD_SIZE);
    }
    ip->ip_len = ntohs(pkt_size - sizeof(*eh));
    ip->ip_dst.s_addr = txd->dstip;

    if (0 == sip)
        ip->ip_src.s_addr = txd->srcip;

    ip->ip_sum = 0;
    ip->ip_sum = ip_fast_csum((char*)ip, ip->ip_hl);

    udp->uh_sport = txd->srcport;
    udp->uh_dport = txd->dstport;
    udp->uh_ulen = htons(pkt_size-g_sport_offset);
    udp->uh_sum = 0;
    if (need_csum)
    {
        fixCheckSumUDP(udp, (struct iphdr*)ip, ppkt->body);
        //udp->uh_sum = wrapsum(checksum2((uint16_t*)udp, sizeof(*udp), 
        //            checksum2((uint16_t*)ppkt->body, pkt_size - sizeof(*udp), 
        //                checksum2((uint16_t*)&ip->ip_src, 2 * sizeof(ip->ip_src),
        //                    IPPROTO_UDP + (uint32_t)ntohs(udp->uh_ulen)
        //                    )
        //                )
        //            ));
    }
    txd->size = pkt_size;
    bcopy(&dnic->if_mac, eh->ether_shost, ETH_ALEN);
    if (0 == get_ret)
    {
        if (chnic)
        {
            struct fio_mac_pkt_data *ptr_data = fio_mac_pkt_data_create(dnic->buddy_mac_pkt_factory);
            assert(ptr_data);
            memcpy(ptr_data->data, txd->pbuf, txd->size);
            memcpy(&ptr_data->mac, txd->dmac, ETH_ALEN);
            ptr_data->size = txd->size;
            fio_list_add_tail(&ptr_data->nic_link, &dnic->pktcache_list_nic[dnic->type_id]);
            snd_ret = 1;
        }
        else
        {
        bcopy(txd->dmac, eh->ether_dhost, ETH_ALEN);
        snd_ret = fio_nic_commit(dnic, txd, 1);
        }
    }
    else
    {
        memset(eh->ether_dhost, 0, ETH_ALEN);
        snd_ret = fio_mac_cache_pkt(dnic, nip, txd);
    }
#ifdef debug_dns
        char str_sip[16];
        char str_dip[16];
        inet_ntop(AF_INET, &ip->ip_src.s_addr, str_sip, 16);
        inet_ntop(AF_INET, &ip->ip_dst.s_addr, str_dip, 16);
        OD( "tid %d %s send pkt type %d srcip %s dstip %s", 
                NIC_EXTRA_CONTEXT(dnic)->me, dnic->alise, txd->type, str_sip, str_dip);
#endif
go_out:
   return snd_ret;
err:
   snd_ret = -1; 
   goto go_out;
}
int fio_nic_sendmsg_(struct fio_nic *nic, uint16_t data_len, struct fio_txdata *txd, int need_csum)
{ 
#ifdef debug_dns
    OD( "tid %d %s send pkt", NIC_EXTRA_CONTEXT(nic)->me, nic->alise); 
#endif
    uint32_t nip, get_ret = 0, sip = 0;
    int snd_ret = 0, dev_idx, chnic = 0;
    struct pktudp *ppkt = (struct pktudp*)txd->pbuf;
    struct ether_header *eh = &ppkt->eh;
    struct ip *ip = &ppkt->ip;
    struct udphdr *udp = &ppkt->udp;
    int pkt_size = g_payload_offset+ntohs(data_len);
    
    fio_nic_include_txd(&nic, txd);

    if (!txd->dmac)
    {
        if (0 != fio_route_find(nic, ntohl(txd->dstip), &sip, &dev_idx, &nip))
            goto err;
        if (dev_idx != nic->type_id)
        {
            nic = &NIC_EXTRA_CONTEXT(nic)->nics[dev_idx];
            chnic = 1;
        }
        get_ret = fio_mac_get_bynip(nic, nip, &txd->dmac, sip);
        ip->ip_src.s_addr = htonl(sip);
    }
    if (pkt_size > MAX_PKT_SIZE)
    {
        pkt_size = MAX_PKT_SIZE;
        data_len = htons(MAX_PAYLOAD_SIZE);
    }
    ip->ip_len = ntohs(pkt_size - sizeof(*eh));
    ip->ip_dst.s_addr = txd->dstip;

    if (0 == sip)
        ip->ip_src.s_addr = txd->srcip;

    ip->ip_sum = 0;
    ip->ip_sum = ip_fast_csum((char*)ip, ip->ip_hl);

    udp->uh_sport = txd->srcport;
    udp->uh_dport = txd->dstport;
    udp->uh_ulen = htons(pkt_size-g_sport_offset);
    udp->uh_sum = 0;
    if (need_csum)
    {
        fixCheckSumUDP(udp, (struct iphdr*)ip, ppkt->body);
        //udp->uh_sum = wrapsum(checksum2((uint16_t*)udp, sizeof(*udp), 
        //            checksum2((uint16_t*)ppkt->body, pkt_size - sizeof(*udp), 
        //                checksum2((uint16_t*)&ip->ip_src, 2 * sizeof(ip->ip_src),
        //                    IPPROTO_UDP + (uint32_t)ntohs(udp->uh_ulen)
        //                    )
        //                )
        //            ));
    }

    txd->size = pkt_size;
    bcopy(&nic->if_mac, eh->ether_shost, ETH_ALEN);
    if (0 == get_ret)
    {
        if (chnic)
        {
            struct fio_mac_pkt_data *ptr_data = fio_mac_pkt_data_create(nic->buddy_mac_pkt_factory);
            assert(ptr_data);
            memcpy(ptr_data->data, txd->pbuf, txd->size);
            memcpy(&ptr_data->mac, txd->dmac, ETH_ALEN);
            ptr_data->size = txd->size;
            fio_list_add_tail(&ptr_data->nic_link, &nic->pktcache_list_nic[nic->type_id]);
            snd_ret = 1;
        }
        else
        {
        bcopy(txd->dmac, eh->ether_dhost, ETH_ALEN);
            snd_ret = fio_nic_commit(nic, txd, 1);
        }
    }
    else
    {
        memset(eh->ether_dhost, 0, ETH_ALEN);
        snd_ret = fio_mac_cache_pkt(nic, nip, txd);
    }

#ifdef debug_dns
    char str_sip[16];
    char str_dip[16];
    inet_ntop(AF_INET, &ip->ip_src.s_addr, str_sip, 16);
    inet_ntop(AF_INET, &ip->ip_dst.s_addr, str_dip, 16);
    OD( "tid %d %s send pkt type %d srcip %s dstip %s", 
            NIC_EXTRA_CONTEXT(nic)->me, nic->alise, txd->type, str_sip, str_dip);
#endif

    //if (sysconfig.enable_slog)
    //   fio_slog_snd(nic, txd->pdata, ntohs(data_len));

go_out:
   return snd_ret;
err:
   snd_ret = -1; 
   goto go_out;
}

int fio_nic_recv(struct fio_nic *from, struct fio_asted_rxs *bufs, uint32_t limit)
{
    struct netmap_ring *rxring;
    struct netmap_if *nifp = from->nifp;
    int m, recv = 0;
    NOD("tid %d limit %u", NIC_EXTRA_CONTEXT(from)->me, limit);
    while (limit > 0)
    {
        NOD("tid %d, limit %u, from->cur_rx_head %d", 
                NIC_EXTRA_CONTEXT(from)->me, limit, from->cur_rx_head);
        rxring = NETMAP_RXRING(nifp, from->cur_rx_head);
        if (rxring->avail == 0)
            from->cur_rx_head = ((from->cur_rx_head+1 == from->qrxlast) ?
                    from->qrxfirst : from->cur_rx_head+1);
        else
        {
            m = fio_recv_pkts(rxring, from, bufs, limit);
            NOD("fio_recv_pkts return %d, limit %u, recv %d", m, limit, recv);
            limit -= m;
            recv += m;
            NOD("m %d, limit %u, recv %d", m, limit, recv);
        }
    }
    NOD("tid %d %s final recv %d, limit %u", NIC_EXTRA_CONTEXT(from)->me, from->alise, recv, limit);
    if (recv > 0)
    {
        from->rxavl -= recv;
        from->rxcount += recv;
    }
    return recv;
}

void fio_nic_init_arpdata(struct fio_ringbuf *rbf, struct fio_txdata *txb, int tid, int _id)
{
    txb->parent = rbf;
    txb->id_ = ((TXDATA_TYPE_PKT_ARP<<TXDATA_TYPE_MASK_OFFSET) | (tid<<TXDATA_TYPE_CACHEID_OFFSET) | _id);
    txb->type = TXDATA_TYPE_PKT_ARP;
    txb->pbuf = &rbf->buf_if_necessary[_id * MAX_PKT_SIZE];
    txb->pdata = txb->pbuf;
    txb->size = 0;
}
void fio_nic_reset_txdata(struct fio_txdata *txb)
{
    memset(&txb->dmac, 0, (((unsigned long)&txb->size)-((unsigned long)&txb->dmac))/sizeof(char));
}
void fio_nic_init_upsndbuf(struct fio_ringbuf *rbf, struct fio_txdata *txb, int tid, int _id)
{
    txb->parent = rbf;
    txb->id_ = ((TXDATA_TYPE_UPSND<<TXDATA_TYPE_MASK_OFFSET) | (tid<<TXDATA_TYPE_CACHEID_OFFSET) | _id);
    txb->type = TXDATA_TYPE_UPSND;
    txb->pbuf = &rbf->buf_if_necessary[_id * MAX_PKT_SIZE];
    txb->pdata = txb->pbuf + g_payload_offset;
    txb->size = sysconfig.pkt_size;
    memcpy(txb->pbuf, &g_pkt4test, sysconfig.pkt_size);
}
void fio_nic_init_txdata(struct fio_ringbuf *rbf, struct fio_txdata *txb, int tid, int _id)
{
    txb->parent = rbf;
    txb->id_ = ((TXDATA_TYPE_NORMAL<<TXDATA_TYPE_MASK_OFFSET) | (tid<<TXDATA_TYPE_CACHEID_OFFSET) | _id);
    txb->type = TXDATA_TYPE_NORMAL;
    txb->pbuf = &rbf->buf_if_necessary[_id * MAX_PKT_SIZE];
    txb->pdata = txb->pbuf + g_payload_offset;
    txb->size = sysconfig.pkt_size;
    memcpy(txb->pbuf, &g_pkt4test, sysconfig.pkt_size);
}
void fio_nic_init_bgdata(struct fio_ringbuf *rbf, struct fio_txdata *txb, int tid, int _id)
{
    txb->parent = rbf;
    txb->id_ = ((TXDATA_TYPE_REALTIME<<TXDATA_TYPE_MASK_OFFSET) | (tid<<TXDATA_TYPE_CACHEID_OFFSET) | _id);
    txb->type = TXDATA_TYPE_REALTIME;
    txb->size = 0;
    txb->pbuf = NULL;
    txb->pdata = NULL;
}
static inline void fio_nic_set_bgbuf_ptr(struct fio_txdata *txb, struct fio_rxdata *rxb)
{
    txb->slot_rx = rxb->slot_rx;
    txb->pbuf = rxb->pbuf;
    txb->pdata = txb->pbuf + g_payload_offset;
}
static inline void fio_nic_set_txbuf_ptr(struct fio_txdata *txb, struct fio_rxdata *rxb)
{
}
#define fio_nic_set_arpbuf_ptr fio_nic_set_txbuf_ptr

static inline void fio_nic_alloc_tx_ifbuf(struct fio_ringbuf *rbf, int total)
{
    rbf->buf_if_necessary = calloc(total, MAX_PKT_SIZE);
}

static inline void fio_nic_realloc_tx_ifbuf(struct fio_ringbuf *rbf, int presize)
{
    int i;
    char *newbuf = calloc(rbf->bf_size+1, MAX_PKT_SIZE);

    for (i = 0; i < presize; i++)
    {
        if (rbf->bf_bufs[i].size > 0)
            memcpy(newbuf+i*MAX_PKT_SIZE, rbf->bf_bufs[i].pbuf, rbf->bf_bufs[i].size);
        else
            memcpy(newbuf+i*MAX_PKT_SIZE, rbf->bf_bufs[i].pbuf, g_payload_offset);
        rbf->bf_bufs[i].pbuf = newbuf+i*MAX_PKT_SIZE;
        rbf->bf_bufs[i].pdata = rbf->bf_bufs[i].pbuf + g_payload_offset;
    }

    free(rbf->buf_if_necessary);
    rbf->buf_if_necessary = newbuf;
}

#define fio_nic_alloc_arp_ifbuf fio_nic_alloc_tx_ifbuf
#define fio_nic_realloc_arp_ifbuf fio_nic_realloc_tx_ifbuf

inline void fio_nic_init_txbuf(struct fio_nic *nic, int txb_size, int bg_size)                     
{                                                                               
    struct _ringbuf_vtbl rbfvtbl = {
        .alloc_if_buf = fio_nic_alloc_tx_ifbuf,
        .realloc_if_buf = fio_nic_realloc_tx_ifbuf
    };

    struct _txdata_vtbl txdvtbl = {
        .init = fio_nic_init_txdata,
        .set_buf_ptr = fio_nic_set_txbuf_ptr,
        .reset = fio_nic_reset_txdata
    };

    //nic->tx_buf.buf_if_necessary = calloc(txb_size + 1, MAX_PKT_SIZE);
    nic->tx_buf.owner = nic;
    fio_rbf_init(&nic->tx_buf, NIC_EXTRA_CONTEXT(nic)->me, txb_size, &rbfvtbl, &txdvtbl);

    if (FIO_MODE_BRG_SINGLE == sysconfig.single_thread)
    {
    txdvtbl.init = fio_nic_init_upsndbuf;
    nic->upsnd_buf.owner = nic;
        fio_rbf_init(&nic->upsnd_buf, NIC_EXTRA_CONTEXT(nic)->me, 100*bg_size, &rbfvtbl, &txdvtbl);
    }

    if (sysconfig.enable_sysstack)
    {
        txdvtbl.init = fio_stack_init_txbuf;
        nic->sys_txbuf.owner = nic;
        fio_rbf_init(&nic->sys_txbuf, NIC_EXTRA_CONTEXT(nic)->me, 2*bg_size, &rbfvtbl, &txdvtbl);
        txdvtbl.init = fio_stack_init_rxbuf;
        nic->sys_rxbuf.owner = nic;
        fio_rbf_init(&nic->sys_rxbuf, NIC_EXTRA_CONTEXT(nic)->me, 2*bg_size, &rbfvtbl, &txdvtbl);
    }
    rbfvtbl.alloc_if_buf = fio_nic_alloc_arp_ifbuf;
    rbfvtbl.realloc_if_buf = fio_nic_realloc_arp_ifbuf;
    txdvtbl.init = fio_nic_init_arpdata;
    txdvtbl.set_buf_ptr = fio_nic_set_arpbuf_ptr;
    //nic->arp_buf.buf_if_necessary = calloc(txb_size + 1, MAX_PKT_SIZE);
    nic->arp_buf.owner = nic;
    fio_rbf_init(&nic->arp_buf, NIC_EXTRA_CONTEXT(nic)->me, txb_size, &rbfvtbl, &txdvtbl);

#ifdef __old_mac_mgr__
    rbfvtbl.alloc_if_buf = fio_nic_alloc_tx_ifbuf;
    rbfvtbl.realloc_if_buf = fio_nic_realloc_tx_ifbuf;
    txdvtbl.init = fio_mac_init_pkt_cache;
    txdvtbl.set_buf_ptr = fio_nic_set_txbuf_ptr;
    int i;
    for (i = 0; i < nic->mac_mgr.vlan_size; i++)
    {
        //nic->mac_mgr.free_pkt_cache_by_dip[i].buf_if_necessary = calloc(MAX_ARP_CACHE+1, MAX_PKT_SIZE);
        nic->mac_mgr.free_pkt_cache_by_dip[i].owner = nic;
        fio_rbf_init(&nic->mac_mgr.free_pkt_cache_by_dip[i], NIC_EXTRA_CONTEXT(nic)->me, MAX_ARP_CACHE,
                &rbfvtbl, &txdvtbl);
    }
#endif

    rbfvtbl.alloc_if_buf = NULL;
    rbfvtbl.realloc_if_buf = NULL;
    txdvtbl.init = fio_nic_init_bgdata;
    txdvtbl.set_buf_ptr = fio_nic_set_bgbuf_ptr;
    nic->bg_buf.owner = nic;
    fio_rbf_init(&nic->bg_buf, NIC_EXTRA_CONTEXT(nic)->me, bg_size, &rbfvtbl, &txdvtbl);
}

inline void fio_nic_init(struct fio_nic *nic, int txb_size, int bg_size)                     
{
    nic->vtbl.send_sys = NULL;
    nic->vtbl.send = fio_nic_send;
    nic->vtbl.send_pktcache = fio_nic_send_pktcache;
    nic->vtbl.recv = fio_nic_recv;
    fio_nic_init_txbuf(nic, txb_size, bg_size);
    if (sysconfig.needcp)
        nic->spec_tasks |= FIO_SPECTSK_COPYRX;
    if (strlen(sysconfig.str_spec_ip) > 0)
    {
        inet_aton(sysconfig.str_spec_ip, &nic->spec_ip);
        int i;
        for (i = 0; i < MAX_PKT_COPY_ONCE; i++)
            nic->spec_rxpkts.rxds[i].pbuf = malloc(MAX_PKT_SIZE);
    }
}

inline int fio_nic_send_arpbuf(struct fio_nic *nic)
{
    int num = fio_rbf_occupy(&nic->arp_buf);
    if (num > 0)
        num = nic->vtbl.send(nic, TXDATA_TYPE_PKT_ARP, num, NULL);
    return num;
}
inline int fio_nic_send_upsndbuf(struct fio_nic *nic)
{
    int num = fio_rbf_occupy(&nic->upsnd_buf);
    if (num > 0)
        num = nic->vtbl.send(nic, TXDATA_TYPE_UPSND, num, NULL);
    return num;
}
inline int fio_nic_send_sys_rxbuf(struct fio_nic *nic)
{
    int num = fio_rbf_occupy(&nic->sys_rxbuf);
    fun_proc_many fun;
    struct fio_asted_rxs *spec_rxpkts = &nic->spec_rxpkts;
    if (num > 0)
        num = nic->vtbl.send(nic, TXDATA_TYPE_SYSSTACK_RX, num, NULL);
    NOD("tid %d nic %s spec_rxpkts avail=%d", NIC_EXTRA_CONTEXT(nic)->me,
            nic->alise, spec_rxpkts->avail);
    if (spec_rxpkts->avail > 0 && fio_map_find_cpy(g_cbf_tbl, spec_rxpkts->cb_type, &fun) && fun)
    {
        fun(nic, &NIC_EXTRA_CONTEXT(nic)->nics[0], &NIC_EXTRA_CONTEXT(nic)->nics[1], 
                spec_rxpkts->rxds, spec_rxpkts->avail, (void*)(1));
        spec_rxpkts->avail = 0;
    }
    return num;
}

inline int fio_nic_send_txbuf(struct fio_nic *nic)
{
    int num = fio_rbf_occupy(&nic->tx_buf);
    if (num > 0)
        num = nic->vtbl.send(nic, TXDATA_TYPE_NORMAL, num, NULL);
    return num;
}

inline int fio_nic_send_bgbuf(struct fio_nic *nic)
{
    int m = 0;
    if (!fio_rbf_empty(&nic->bg_buf))
    {
        //int oc, txa = nic->txavl;;
        NOD("tid %d will send %d", NIC_EXTRA_CONTEXT(nic)->me, fio_rbf_occupy(&nic->bg_buf));
        //m = fio_nic_send(nic, TXDATA_TYPE_REALTIME, (oc=fio_rbf_occupy(&nic->bg_buf)));
        m = nic->vtbl.send(nic, TXDATA_TYPE_REALTIME, fio_rbf_occupy(&nic->bg_buf), NULL);
        if (!fio_rbf_empty(&nic->bg_buf))
        {
            NOD("tid %d %s slow m %d, oc %d %d, txavail %d", NIC_EXTRA_CONTEXT(nic)->me,
                    nic->alise, m, oc, fio_rbf_occupy(&nic->bg_buf), txa);
            OD( "tid %d %s slow m %d, rest %d", NIC_EXTRA_CONTEXT(nic)->me,
                    nic->alise, m, fio_rbf_occupy(&nic->bg_buf));
            if (sysconfig.is_test)
                assert(0);
        }
    }
    return m;
}

int fio_nic_info(struct nmreq *nmr, const char * const ifname)
{
    int fd = open("/dev/netmap", O_RDWR);
    if (fd == -1) 
    {
        OD( "ifname %s Unable to open /dev/netmap", ifname);
        return -1;
    }
    else
    {
        nmr->nr_version = NETMAP_API;
        if ((ioctl(fd, NIOCGINFO, nmr)) == -1) 
        {
            OD( "Unable to get if info without name");
            //close(fd);
            //return -1;
        }
        else 
            OD( "map size is %d Kb", nmr->nr_memsize >> 10);

        bzero(nmr, sizeof(*nmr));
        nmr->nr_version = NETMAP_API;
        strncpy(nmr->nr_name, ifname, sizeof(nmr->nr_name));
        if ((ioctl(fd, NIOCGINFO, nmr)) == -1) 
            OD( "Unable to get if info for %s", ifname);    
    }
    close(fd);
    return 0;
}

int fio_nic_open(struct fio_nic *nic, char *alise, const char * const ifname, uint16_t rid, uint32_t dir, void *prv)
{
    char mybuf[] = "00:00:00:00:00:00";

    nic->fd = open("/dev/netmap", O_RDWR);
    if (nic->fd == -1) 
    {
        OD( "Unable to open /dev/netmap");
        return -1;
    }

    nic->alise = alise;
    nic->private_ = prv;
    if (source_hwaddr(ifname, mybuf) == -1) 
    {
        OD( "Unable to retrieve source mac %s", ifname);
        goto err;
    }
    bcopy(ether_aton(mybuf), &nic->if_mac, ETH_ALEN);
    OD( "if %s mac %s", ifname, mybuf);
    //if (get_if_ip(ifname, &nic->if_ipdef))
    //{
    //    D("<error> %s get ip", ifname);
    //    goto err;
    //}

    bzero(&nic->nmr, sizeof(struct nmreq));
    strncpy(nic->nmr.nr_name, ifname, sizeof(nic->nmr.nr_name));
    nic->nmr.nr_version = NETMAP_API;
    if((ioctl(nic->fd, NIOCGINFO, &nic->nmr)) == -1)
    {
        OD( "Unable to get info from %s", ifname);
        goto err;
    }

#if 1
    fio_nic_do_ioctl(nic, SIOCGIFFLAGS, 0);
    if ((nic->if_flags & IFF_UP) == 0) {
        OD( "%s is down, bringing up...", ifname);
        nic->if_flags |= IFF_UP;
    }
    if (sysconfig.promisc) 
    {
        nic->if_flags |= IFF_PPROMISC;
        fio_nic_do_ioctl(nic, SIOCSIFFLAGS, 0);
    }
    else
    {
        nic->if_flags &= ~IFF_PPROMISC;
        fio_nic_do_ioctl(nic, SIOCSIFFLAGS, 0);
    }
#endif

    if (nic->mmap_addr == NULL)
    {
        nic->mmap_addr = (struct netmap_d *) mmap(0, nic->nmr.nr_memsize,
                PROT_WRITE | PROT_READ,
                MAP_SHARED, nic->fd, 0);
        if (nic->mmap_addr == MAP_FAILED)
        {
            OD( "Unable to mmap");
            nic->mmap_addr = NULL;
            goto err;
        }
    }

    nic->mmap_size = nic->nmr.nr_memsize;
    bzero(&nic->nmr, sizeof(struct nmreq));
    strncpy(nic->nmr.nr_name, ifname, sizeof(nic->nmr.nr_name));
    nic->nmr.nr_version = NETMAP_API;
    if ((rid & FIO_SYS_RING_MASK) == FIO_SYS_RING)
        nic->nmr.nr_ringid = rid;
    else if ((rid&FIO_LSTN_ALL_RING) == FIO_LSTN_ALL_RING)
        nic->nmr.nr_ringid = 0;
    else
        nic->nmr.nr_ringid = (rid | NETMAP_HW_RING);

    nic->dir = dir;
    if ( !(dir&FIO_DIR_TX) )
        nic->nmr.nr_ringid |= NETMAP_NO_TX_POLL;

    if ((ioctl(nic->fd, NIOCREGIF, &nic->nmr)) == -1) 
    {
        OD( "Unable to register %s", ifname);
        goto err;
    }
    if (rid != FIO_SYS_RING)
    {
        fio_nic_do_ioctl(nic, SIOCETHTOOL, ETHTOOL_STXCSUM);
        fio_nic_do_ioctl(nic, SIOCETHTOOL, ~ETH_FLAG_LRO);
    }
    nic->nifp = NETMAP_IF(nic->mmap_addr, nic->nmr.nr_offset);
    if ((rid & FIO_SYS_RING_MASK) == FIO_SYS_RING)
    {
		nic->qrxfirst = nic->nmr.nr_rx_rings;
		nic->qrxlast = nic->qrxfirst + 1;
		nic->qtxfirst = nic->nmr.nr_tx_rings;
		nic->qtxlast = nic->qtxfirst + 1;
    }
    else if ((rid&FIO_LSTN_ALL_RING) == FIO_LSTN_ALL_RING)
    {
        nic->qrxfirst = 0;
        nic->qrxlast = nic->nmr.nr_rx_rings;
        nic->qtxfirst = 0;
        nic->qtxlast = nic->nmr.nr_tx_rings;
    }
    else
    {
        nic->qrxfirst = rid;
        nic->qrxlast = rid+1;
        nic->qtxfirst = rid;
        nic->qtxlast = rid+1;
    }
    OD( "qrxfirst %d, qrxlast %d", nic->qrxfirst, nic->qrxlast);
    nic->cur_rx_head = nic->qrxfirst;
    nic->cur_tx_head = nic->qtxfirst;

    return 0;
err:
    close(nic->fd);
    return -2;
}

inline struct fio_nic * fio_nic_fromip_nic(struct fio_nic *from, const char *strip)
{
    struct fio_context *ct = NIC_EXTRA_CONTEXT(from);
    int i;
    for (i = 0; i < ct->num_nic; i++)
    {
        if (!strcmp(ct->nics[i].nmr.nr_name, strip))
            return &ct->nics[i];
    }

    return NULL;
}

inline struct fio_nic * fio_nic_fromip(const char *strip)
{
    struct fio_context *ct = g_contexts;
    int i;
    for (i = 0; i < ct->num_nic; i++)
    {
        if (!strcmp(ct->nics[i].nmr.nr_name, strip))
            return &ct->nics[i];
    }

    return NULL;
}

inline int fio_nic_close(struct fio_nic *nic)
{
    OD( "");
    int i;
    fio_rbf_destroy(&nic->tx_buf);
    fio_rbf_destroy(&nic->sys_txbuf);
    fio_rbf_destroy(&nic->sys_rxbuf);
    fio_rbf_destroy(&nic->arp_buf);
    fio_rbf_destroy(&nic->bg_buf);

    if (FIO_MODE_BRG_SINGLE == sysconfig.single_thread)
        fio_rbf_destroy(&nic->upsnd_buf);
    if (nic->mmap_addr)
        munmap(nic->mmap_addr, nic->mmap_size);
    ioctl(nic->fd, NIOCUNREGIF, NULL);
    close(nic->fd);
    if (nic->spec_ip.s_addr != 0)
        for (i = 0; i < MAX_PKT_COPY_ONCE; i++)
            free(nic->spec_rxpkts.rxds[i].pbuf);
    return (0);
}

