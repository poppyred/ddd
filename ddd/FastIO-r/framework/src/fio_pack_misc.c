//	$Id: fio_pack_mise.c 2013-05-23 likunxiang$
//
#include "fio_pack_misc.h"
#include "fio_context.h"
#include "fio_sysconfig.h"

/*
 * create and enqueue a batch of packets on a ring.
 * On the last one set NS_REPORT to tell the driver to generate
 * an interrupt when done.
 */

static const char *default_payload="netmap pkt-gen Luigi Rizzo and Matteo Landi\n"
"http://info.iet.unipi.it/~luigi/netmap/ ";

struct pktudp g_pkt4test;
struct pktudp g_pkt4test2;

void _set_packet_test(struct fio_context *context, struct fio_txdata *txd)
{
    txd->size = sysconfig.pkt_size;
    memcpy(txd->pbuf, &g_pkt4test, sysconfig.pkt_size);
}

void __initialize_dns_packet(char *str_payload, int len)
{
    {
        struct pktudp *pkt = &g_pkt4test;
        struct ether_header *eh;
        struct ip *ip;
        struct udphdr *udp;
        sysconfig.pkt_size = sizeof(*eh) + sizeof(struct ip) + len;

        memcpy(pkt->body, str_payload, len);
        ip = &pkt->ip;

        ip->ip_v = IPVERSION;
        ip->ip_hl = 5;
        ip->ip_id = 0;
        ip->ip_tos = IPTOS_LOWDELAY;
        ip->ip_len = ntohs(sizeof(struct ip) + len);
        ip->ip_id = 0;
        ip->ip_off = htons(IP_DF); /* Don't fragment */
        ip->ip_ttl = IPDEFTTL;
        ip->ip_p = IPPROTO_UDP;
        ip->ip_dst.s_addr = sysconfig.dst_ip[0].start.s_addr;
        ip->ip_src.s_addr = sysconfig.if_ips[0].s_addr;
        //ip->ip_sum = 0;
        //ip->ip_sum = wrapsum(checksum(ip, sizeof(*ip), 0));
        //ip->ip_sum = htons(checksum((uint16_t*)ip, sizeof(*ip)/sizeof(uint16_t)));
        ip->ip_sum = ip_fast_csum((char*)ip, ip->ip_hl);

        udp = &pkt->udp;
        udp->uh_sport = htons(sysconfig.src_port);
        udp->uh_dport = htons(sysconfig.dst_port);
        udp->uh_ulen = htons(len);
        /* Magic: taken from sbin/dhclient/packet.c */
        udp->uh_sum = 0;
        //udp->uh_sum = wrapsum(checksum(udp, sizeof(*udp), checksum(pkt->body,
        //                paylen - sizeof(*udp), checksum(&ip->ip_src, 2 * sizeof(ip->ip_src),
        //                    IPPROTO_UDP + (u_int32_t)ntohs(udp->uh_ulen)))));

        eh = &pkt->eh;
        bcopy(&sysconfig.if_macs[0], eh->ether_shost, 6);
        bcopy(&sysconfig.dst_mac[0].start, eh->ether_dhost, 6);
        eh->ether_type = htons(ETHERTYPE_IP);
    }

    {
        struct pktudp *pkt = &g_pkt4test2;
        struct ether_header *eh;
        struct ip *ip;
        struct udphdr *udp;
        sysconfig.pkt_size = sizeof(*eh) + sizeof(struct ip) + len;

        memcpy(pkt->body, str_payload, len);
        ip = &pkt->ip;

        ip->ip_v = IPVERSION;
        ip->ip_hl = 5;
        ip->ip_id = 0;
        ip->ip_tos = IPTOS_LOWDELAY;
        ip->ip_len = ntohs(sizeof(struct ip) + len);
        ip->ip_id = 0;
        ip->ip_off = htons(IP_DF); /* Don't fragment */
        ip->ip_ttl = IPDEFTTL;
        ip->ip_p = IPPROTO_UDP;
        ip->ip_dst.s_addr = sysconfig.dst_ip[1].start.s_addr;
        ip->ip_src.s_addr = sysconfig.if_ips[1].s_addr;
        //ip->ip_sum = 0;
        //ip->ip_sum = wrapsum(checksum(ip, sizeof(*ip), 0));
        //ip->ip_sum = htons(checksum((uint16_t*)ip, sizeof(*ip)/sizeof(uint16_t)));
        ip->ip_sum = ip_fast_csum((char*)ip, ip->ip_hl);

        udp = &pkt->udp;
        udp->uh_sport = htons(sysconfig.src_port);
        udp->uh_dport = htons(sysconfig.dst_port);
        udp->uh_ulen = htons(len);
        /* Magic: taken from sbin/dhclient/packet.c */
        udp->uh_sum = 0;
        //udp->uh_sum = wrapsum(checksum(udp, sizeof(*udp), checksum(pkt->body,
        //                paylen - sizeof(*udp), checksum(&ip->ip_src, 2 * sizeof(ip->ip_src),
        //                    IPPROTO_UDP + (u_int32_t)ntohs(udp->uh_ulen)))));

        eh = &pkt->eh;
        bcopy(&sysconfig.if_macs[1], eh->ether_shost, 6);
        bcopy(&sysconfig.dst_mac[1].start, eh->ether_dhost, 6);
        eh->ether_type = htons(ETHERTYPE_IP);
    }
}

/*
 * Fill a packet with some payload.
 * We create a UDP packet so the payload starts at
 *	14+20+8 = 42 bytes.
 */
void __initialize_packet()
{
    struct pktudp *pkt = &g_pkt4test;
    struct ether_header *eh;
    struct ip *ip;
    struct udphdr *udp;
    uint16_t paylen = sysconfig.pkt_size-1 - sizeof(*eh) - sizeof(struct ip);
    int i, l, l0 = strlen(default_payload);

    for (i = 0; i < paylen;) {
        l = min(l0, paylen - i);
        bcopy(default_payload, pkt->body + i, l);
        i += l;
    }
    pkt->body[i] = '\0';
    ip = &pkt->ip;

    ip->ip_v = IPVERSION;
    ip->ip_hl = 5;
    ip->ip_id = 0;
    ip->ip_tos = IPTOS_LOWDELAY;
    ip->ip_len = ntohs(sysconfig.pkt_size - sizeof(*eh));
    ip->ip_id = 0;
    ip->ip_off = htons(IP_DF); /* Don't fragment */
    ip->ip_ttl = IPDEFTTL;
    ip->ip_p = IPPROTO_UDP;
    ip->ip_dst.s_addr = sysconfig.dst_ip[0].start.s_addr;
    ip->ip_src.s_addr = sysconfig.if_ips[1].s_addr;
    //ip->ip_sum = 0;
    //ip->ip_sum = wrapsum(checksum(ip, sizeof(*ip), 0));
    //ip->ip_sum = htons(checksum((uint16_t*)ip, sizeof(*ip)/sizeof(uint16_t)));
    ip->ip_sum = ip_fast_csum((char*)ip, ip->ip_hl);

    udp = &pkt->udp;
    udp->uh_sport = htons(sysconfig.src_port);
    udp->uh_dport = htons(sysconfig.dst_port);
    udp->uh_ulen = htons(paylen);
    /* Magic: taken from sbin/dhclient/packet.c */
    udp->uh_sum = 0;
    //udp->uh_sum = wrapsum(checksum(udp, sizeof(*udp), checksum(pkt->body,
    //                paylen - sizeof(*udp), checksum(&ip->ip_src, 2 * sizeof(ip->ip_src),
    //                    IPPROTO_UDP + (u_int32_t)ntohs(udp->uh_ulen)))));

    eh = &pkt->eh;
    bcopy(&sysconfig.if_macs[1], eh->ether_shost, 6);
    bcopy(&sysconfig.dst_mac[0].start, eh->ether_dhost, 6);
    eh->ether_type = htons(ETHERTYPE_IP);
}

