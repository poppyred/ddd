//	$Id: fio_icmp.h 2013-05-23 likunxiang$
//
#ifndef _FIO_ICMP_MISC_H_
#define _FIO_ICMP_MISC_H_

#include "fio_nm_util.h"
#include <netinet/ip_icmp.h>

struct fio_nic;
struct fio_rxdata;

struct icmp_packet
{
    struct ether_header eh;
    struct ip ip;
    struct icmp icmp;
    uint8_t padding[100];
} __attribute__((__packed__));

extern int g_icmp_pkt_size;

int32_t fio_icmp_gen_req(struct fio_nic *nic, struct icmp_packet *icmp_packet, uint32_t dip, struct ether_addr *dmac, uint32_t sip_network);
int32_t fio_icmp_gen_resp(struct fio_nic *nic, struct icmp_packet *icmp_from, struct icmp_packet *icmp_to, struct fio_rxdata *rxd, int realtime);
inline int fio_icmp_handle_req(struct fio_nic *nic, struct fio_rxdata *rxds, void *param);

#endif
