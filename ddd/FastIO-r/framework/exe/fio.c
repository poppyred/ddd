//	$Id: fio.c 2013-05-23 likunxiang$
//
#include "fio.h"
#include <assert.h>

int fio_set_txmsg_sin(struct fio_nic *t, struct fio_rxdata *rxdata)
{
    (void)t;
    (void)rxdata;
    //memcpy(rxdata->pbuf+g_payload_offset, "sadlfjasdfjasfasfsfs", 18);
    return 18;
}

//int single_mode_handle(struct fio_nic *from, struct fio_nic *to, 
//        struct fio_rxdata *rxdata, uint32_t sip, uint16_t sport,
//        uint16_t recvport, void *param)
//{
//    sport = ntohs(sport);
//    recvport = ntohs(recvport);
//    //char str_sip[16];
//    ND("tid %d: dsize %d sip %s sport %d recvport %d", 
//            NIC_EXTRA_CONTEXT(from)->me, rxdata->size, 
//            inet_ntop(AF_INET, (struct in_addr*)&sip, str_sip, 16),
//            sport, recvport); 
//
//    //struct pktudp *ppkt = (struct pktudp*)rxdata->pbuf;
//    //ppkt->ip.ip_dst.s_addr = sysconfig.dst_ip.start.s_addr;
//    //ppkt->udp.uh_dport = htons(sysconfig.dst_port);
//    //bcopy(&sysconfig.dst_mac.start, ppkt->eh.ether_dhost, ETH_ALEN);
//    //((struct txinfo*)param)->txslot->len = g_payload_offset+18;
//    //return 0;
//    return fio_nic_sendmsg_imme(to, (struct txinfo*)param,
//            sysconfig.dst_ip[0].start.s_addr, 
//            htons(sysconfig.dst_port), 
//            htons(sysconfig.src_port), 
//            rxdata->pbuf, 
//            htons(fio_set_txmsg_sin(to, rxdata))
//            );
//}

int fio_set_txmsg(struct fio_nic *nic, struct fio_txdata *txdata)
{
    (void)nic;
    (void)txdata->pdata;
#if 0
    int cplen = MAX_PAYLOAD_SIZE;
    char * p_v = "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
    memcpy(txdata->pdata, p_v, cplen);
    return cplen;
#else
    //memcpy(txdata->pdata, "sadlfjasdfj", 16);
    return 18;
#endif
}

int handle_audp(struct fio_nic *from, struct fio_nic *to, struct fio_rxdata *rxdata, 
        uint32_t sip, uint16_t sport, uint16_t recvport, void *useless)
{
    struct fio_txdata *txdata;
    int txcount;
    sport = ntohs(sport);
    recvport = ntohs(recvport);
    //char str_sip[16];
    NOD("tid %d: dsize %d sip %s sport %d recvport %d", 
            NIC_EXTRA_CONTEXT(from)->me, rxdata->size, inet_ntop(AF_INET,
                (struct in_addr*)&sip, str_sip, 16), sport, recvport); 

    txcount = fio_nic_reserve_tx(to, &txdata, rxdata);
    if (txcount == 1)
    {
        fio_nic_sendmsg_direct(to, sysconfig.dst_ip[0].start.s_addr, txdata);
        fio_stack_send_up(to, txdata->pbuf, txdata->size);
    }
    else
    {
        OD("tid %d reserve realtime tx error", NIC_EXTRA_CONTEXT(from)->me);
        exit(1);
    }

    //txcount = fio_reserve_tx(t, 1, &txdata, NULL, 1);
    //if (txcount == 1)
    //{
    //    fio_sendmsg(t, sysconfig.dst_ip.start.s_addr, htons(sysconfig.dst_port), 
    //            htons(sysconfig.src_port), htons(fio_set_txmsg(t, txdata)), txdata);
    //}
    //else
    //{
    //    D("tid %d reserve tx error", t->me);
    //    exit(1);
    //}

    return 0;
}

int handle_udps_2cards(struct fio_nic *from, struct fio_nic *nic0, struct fio_nic *nic1, 
        struct fio_rxdata *rxdata, int avail, void *useless)
{
    (void)useless;
    struct fio_txdata *txdata;
    int txcount, i;
    //uint32_t sip;
    //uint16_t sport, recvport;

    uint32_t dstip = 0;

    if (NIC_EXTRA_CONTEXT(from)->nics[0].fd == from->fd)
       dstip = sysconfig.dst_ip[0].start.s_addr;
    else
       dstip = sysconfig.dst_ip[1].start.s_addr;

    for (i = 0; i < avail; i++, rxdata++)
    {
        txcount = fio_nic_reserve_tx(from, &txdata, rxdata);
        if (nm_unlikely(txcount != 1))
        {
            OD("tid %d reserve realtime tx error", NIC_EXTRA_CONTEXT(from)->me);
        }
        //sport = ntohs(rxdata->sport);
        //recvport = ntohs(rxdata->dport);
        //char str_sip[16];
        NOD("tid %d: dsize %d sip %s sport %d recvport %d", 
                NIC_EXTRA_CONTEXT(from)->me, rxdata->size, inet_ntop(AF_INET,
                    (struct in_addr*)&rxdata->sip, str_sip, 16), sport, recvport); 

        txdata->dstip = dstip;
        txdata->dstport = htons(sysconfig.dst_port);
        txdata->srcip = 0;
        txdata->srcport = htons(sysconfig.src_port);
        fio_send(from, htons(fio_set_txmsg(from, txdata)), txdata, 0);
    }

    return 0;
}

int handle_udps(struct fio_nic *from, struct fio_nic *nic0, struct fio_nic *nic1, 
        struct fio_rxdata *rxdata, int avail, void *useless)
{
    (void)useless;
    struct fio_txdata *txdata;
    int txcount, i;
    //uint32_t sip;
    //uint16_t sport, recvport;

    struct fio_nic *to = nic0;
    if (NIC_EXTRA_TYPEID(from) == NIC_EXTRA_TYPEID(nic0))
        to = nic1;

    for (i = 0; i < avail; i++, rxdata++)
    {
        txcount = fio_nic_reserve_tx(to, &txdata, rxdata);
        if (nm_unlikely(txcount != 1))
        {
            OD("tid %d reserve realtime tx error", NIC_EXTRA_CONTEXT(to)->me);
        }
        //sport = ntohs(rxdata->sport);
		//if (sport == 9998)
		//	assert(0);
        //recvport = ntohs(rxdata->dport);
        //char str_sip[16];
        NOD("tid %d: dsize %d sip %s sport %d recvport %d", 
                NIC_EXTRA_CONTEXT(to)->me, rxdata->size, inet_ntop(AF_INET,
                    (struct in_addr*)&rxdata->sip, str_sip, 16), sport, recvport); 

        txdata->dstip = sysconfig.dst_ip[0].start.s_addr;
        txdata->dstport = htons(sysconfig.dst_port);
        txdata->srcip = 0;
        txdata->srcport = htons(sysconfig.src_port);
        fio_send(to, htons(fio_set_txmsg(to, txdata)), txdata, 0);
        //fio_nic_sendmsg_direct(to, sysconfig.dst_ip.start.s_addr, txdata);
    }

    return 0;
}

int handle_udps_in(struct fio_nic *from, struct fio_nic *nic0, struct fio_nic *nic1, 
        struct fio_rxdata *rxdata, int avail, void *useless)
{
    (void)useless;
    struct fio_txdata *txdata;
    int txcount, i;
    uint16_t sport, recvport;

    for (i = 0; i < avail; i++, rxdata++)
    {
        txcount = fio_nic_reserve_tx(from, &txdata, rxdata);
        if (nm_unlikely(txcount != 1))
        {
            OD("tid %d reserve realtime tx error", NIC_EXTRA_CONTEXT(from)->me);
        }
        sport = ntohs(rxdata->sport);
		//if (sport == 9998)
		//	assert(0);
        recvport = ntohs(rxdata->dport);
        char str_sip[16];
        char str_dip[16];
        OD("tid %d: dsize %d sip %s sport %d dip %s recvport %d", 
                NIC_EXTRA_CONTEXT(from)->me, rxdata->size, 
		inet_ntop(AF_INET, (struct in_addr*)&rxdata->sip, str_sip, 16), 
		sport,
		inet_ntop(AF_INET, (struct in_addr*)&rxdata->dip, str_dip, 16), 
		recvport); 

        txdata->dstip = sysconfig.dst_ip[0].start.s_addr;
        txdata->dstport = htons(sysconfig.dst_port);
        txdata->srcip = 0;
        txdata->srcport = htons(sysconfig.src_port);
        fio_send(from, htons(fio_set_txmsg(from, txdata)), txdata, 0);
        //fio_nic_sendmsg_direct(from, sysconfig.dst_ip.start.s_addr, txdata);
        
        txcount = fio_nic_reserve_tx(from, &txdata, NULL);
        if (nm_unlikely(txcount != 1))
        {
            OD("tid %d reserve normal tx error", NIC_EXTRA_CONTEXT(from)->me);
        }
        else
        {
            txdata->dstip = sysconfig.dst_ip[0].start.s_addr;
            txdata->dstport = htons(sysconfig.dst_port - 1);
            txdata->srcip = 0;
            txdata->srcport = htons(sysconfig.src_port);
            fio_send(from, htons(fio_set_txmsg(from, txdata)), txdata, 0);
        }
    }

    return 0;
}

int handle_dns_resp(struct fio_nic *from, struct fio_nic *nic0, struct fio_nic *nic1,
        struct fio_rxdata *rxdata, int avail, void *useless)
{
    (void)useless;
    int j, i;
    char str_sip[16];
    char str_dip[16];
    char str_x[1500] = {0};
    char str_byte[10];
    OD("tid %d pkt count %d avail", NIC_EXTRA_CONTEXT(from)->me, avail);
    //for (j = 0; j < avail; j++, rxdata++)
    //{
    //    str_x[0] = 0;
    //    inet_ntop(AF_INET, &rxdata->sip, str_sip, 16);
    //    inet_ntop(AF_INET, &rxdata->dip, str_dip, 16);
    //    D("tid %d pkt len %d sip %s, sport %d, dip %s, dport %d", 
    //            NIC_EXTRA_CONTEXT(from)->me, rxdata->size, 
    //            str_sip, ntohs(rxdata->sport),
    //            str_dip, ntohs(rxdata->dport));

    //    for (i = 0; i < rxdata->size; i++)
    //    {
    //        snprintf(str_byte, 10, "%02X ", (uint8_t)rxdata->pbuf[i]);
    //        if (i%8 == 0)
    //            strncat(str_x, "\n", 1500);
    //        strncat(str_x, str_byte, 1500);
    //    }
    //    D("%s", str_x);

    //}
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("argc < 2\n");
        return 0;
    }

    if (0 != parse_config(argv[1]))
        return 0;

    //fio_register_handler(FIO_CBF_SINGLE_THREAD, single_mode_handle);
    //fio_register_handler(IPPROTO_UDP, handle_audp);
    switch (sysconfig.single_thread)
    {
        case FIO_MODE_1_SND_OR_RCV /*2*/:
            fio_register_handler(T_FIO_PKT_INTD, handle_dns_resp);
            break;
        case FIO_MODE_N_BRG_OUT /*4*/:
            fio_register_handler(T_FIO_PKT_INTD, handle_udps);
            break;
        default:
            fio_register_handler(T_FIO_PKT_INTD, handle_udps_in);
            break;
    }

    if (0 != fio_init() || 0 != fio_start())
        return 0;
	signal(SIGINT, fio_sigint_h);
    fio_start_statistics();

    fio_wait();
    fio_shutdown();

    return 0;
}

