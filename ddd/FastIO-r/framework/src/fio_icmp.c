
#include "fio_icmp.h"
#include "fio_pack_misc.h"
#include "fio_sysconfig.h"
#include "fio_context.h"
#include "fio_sysstack.h"
//#include "log_log.h"

int g_icmp_pkt_size = sizeof(struct icmp_packet);

int32_t fio_icmp_gen_req(struct fio_nic *nic, struct icmp_packet *icmp_packet, uint32_t dip, struct ether_addr *dmac, uint32_t sip_network)
{
    memcpy(icmp_packet, &g_pkt4test, g_sport_offset); 

    memcpy(&icmp_packet->eh.ether_dhost, dmac, ETH_ALEN);
    memcpy(&icmp_packet->eh.ether_shost, &nic->if_mac, ETH_ALEN);

    struct ip *ip = &icmp_packet->ip;
    ip->ip_len = ntohs(g_icmp_pkt_size - sizeof(struct ether_header));
    ip->ip_dst.s_addr = dip;
    ip->ip_src.s_addr = sip_network;
    ip->ip_sum = 0;
    ip->ip_sum = ip_fast_csum((char*)ip, ip->ip_hl);

    struct icmp *icmp = &icmp_packet->icmp;
    icmp->icmp_type = ICMP_ECHO;//8
    icmp->icmp_code = 0;
    icmp->icmp_id = nic->fd;
    icmp->icmp_seq = nic->seq_num++;
    icmp->icmp_cksum = 0;
    icmp->icmp_cksum = in_cksum((uint16_t*)icmp, sizeof(struct icmp));
    return 0;
}

int32_t fio_icmp_gen_resp(struct fio_nic *nic, struct icmp_packet *icmp_from, struct icmp_packet *icmp_to, struct fio_rxdata *rxd, int realtime)
{
    if (TXDATA_TYPE_REALTIME != realtime)
        memcpy(icmp_to, icmp_from, rxd->size); 

    memcpy(&icmp_to->eh.ether_dhost, &rxd->smac, ETH_ALEN);
    memcpy(&icmp_to->eh.ether_shost, &nic->if_mac, ETH_ALEN);

    struct ip *ip = &icmp_to->ip;
    ip->ip_len = ntohs(rxd->size - sizeof(struct ether_header));
    ip->ip_dst.s_addr = rxd->sip;
    ip->ip_src.s_addr = rxd->dip;
    ip->ip_sum = 0;
    ip->ip_sum = ip_fast_csum((char*)ip, ip->ip_hl);

    struct icmp *icmp = &icmp_to->icmp;
    OD( "icmp_type = %x", icmp->icmp_type);
    icmp->icmp_type = ICMP_ECHOREPLY;//0
    uint8_t *ptr = (uint8_t*)&icmp->icmp_cksum;
    if (*ptr > (0xff-0x08))
	    (*(ptr+1))++;
    *ptr += 0x08;
    
    //snprintf((char*)icmp_to->padding, 100, "%s", nic->alise);
    return 0;
}

inline int32_t fio_icmp_handle_req(struct fio_nic *nic, struct fio_rxdata *rxd, void *param)
{
    NOD("");
    //fprintf(stderr, "fio_icmp_handle_req\n");
    struct fio_txdata *m;
    struct icmp_packet *ppkt_rx = (struct icmp_packet*)rxd->pbuf;
    if ((uint8_t)ICMP_ECHOREPLY == ppkt_rx->icmp.icmp_type
            || !fio_nic_is_myip(nic, rxd->dip))
    {
        fio_stack_deliver(nic, rxd);
        goto suc;
    }

    uint32_t sip = 0, nip, get_ret = 0;
    int dev_idx;
    if (0 != fio_route_find(nic, ntohl(rxd->sip), &sip, &dev_idx, &nip))
        goto err;

    if (dev_idx != nic->type_id)
    {
        fio_stack_deliver(nic, rxd);
        goto suc;
    }
    if (fio_nic_reserve_tx(nic, &m, rxd) < 1)
        goto err;
    get_ret = fio_mac_get_bynip(nic, nip, &m->dmac, ntohl(rxd->dip));
    struct icmp_packet *ppkt_tx = (struct icmp_packet*)m->pbuf;
    struct ether_header *eh = &ppkt_tx->eh;
    if (fio_icmp_gen_resp(nic, ppkt_rx, ppkt_tx, rxd, m->type))
    {
        OD( "tid %d generate icmp reply packet error", NIC_EXTRA_CONTEXT(nic)->me);
        goto err;
    }
    m->size = rxd->size;
    if (0 == get_ret)
    {
        prmac((uint8_t*)m->dmac);
        bcopy(m->dmac, eh->ether_dhost, ETH_ALEN);
        fio_nic_commit(nic, m, 1);
    }
    else
    {
        OD( "tid %d generate icmp reply no dst mac", NIC_EXTRA_CONTEXT(nic)->me);
        memset(eh->ether_dhost, 0, ETH_ALEN);
        fio_mac_cache_pkt(nic, nip, m);
    }

suc:
    return 0;
err:
    return -1;
}

