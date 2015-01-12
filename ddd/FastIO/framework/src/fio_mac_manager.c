//	$Id: fio_mac_manager.c 2013-05-23 likunxiang$
//

#if (defined _TEST_MAC_ || defined _TEST_MAC_1)
#include "fio_mac_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <netinet/udp.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <errno.h>
#include <sys/time.h>
#include <netdb.h>

//gcc -o fio_mac_manager fio_mac_manager.c -D_TEST_MAC_ -I../inc -lpthread
//gcc -o fio_mac_manager fio_mac_manager.c -D_TEST_MAC_1 -I../inc -lpthread

#define ND(format, ...)	do {} while(0)
#define D(format, ...)				\
    fprintf(stderr, "%s [%d] " format "\n", 	\
            __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define MAC_ZERO {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define MAC_SOURCE { 0x90,0xE2,0xBA,0x39,0x1D,0x20}
//冒充的IP
#define IP_TRICK "192.168.23.151"
//目标机器的MAC
#define MAC_TARGET {0xD4, 0xBE, 0xD9, 0xAA, 0x75, 0xF9}
//目标机器的IP
#define IP_TARGET "192.168.23.126"
//#define IP_TARGET "255.255.255.255"

void prmac(uint8_t *ptr);
int mac_pack(struct in_addr *ip_snd, struct ether_addr *mac_snd, struct in_addr *ip_dst, 
        struct ether_addr *mac_dst, uint16_t ar_op, const char * const if_from)
{
    int sfd, len;
    struct arp_packet ap;
    //struct in_addr inaddr_sender, inaddr_receiver;
    struct sockaddr_ll sl;

    //sfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    sfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_RARP));
    if(-1 == sfd)
    {
        D("create socket error %s", strerror(errno));
        return -1;
    }

    memset(&ap, 0, sizeof(struct arp_packet));
    //ap.mac_target = MAC_TARGET;
    if (ar_op == ARPOP_REQUEST && !mac_dst)
        memcpy(ap.mac_target, &(uint8_t[])MAC_BRCAST, ETH_ALEN);
    else
        memcpy(ap.mac_target, mac_dst, ETH_ALEN);
    //ap.mac_source = MAC_ZERO;
    memcpy(ap.mac_source, mac_snd, ETH_ALEN);
    ap.ethertype = htons(ETHERTYPE_ARP); //htons(0x0806)
    ap.hw_type = htons(0x1);
    ap.proto_type = htons(0x0800);
    ap.mac_addr_len = ETH_ALEN;
    ap.ip_addr_len = 4;
    ap.operation_code = htons(ar_op);
    //ap.mac_sender = MAC_ZERO;
    memcpy(ap.mac_sender, mac_snd, ETH_ALEN);
    //inet_aton(IP_TRICK, &inaddr_sender);
    //memcpy(&ap.ip_sender, &inaddr_sender, sizeof(inaddr_sender));
    memcpy(ap.ip_sender, ip_snd, sizeof(ap.ip_sender));
    //ap.mac_receiver = MAC_TARGET;
    if (ar_op == ARPOP_REQUEST && !mac_dst)
        memcpy(ap.mac_receiver, &(uint8_t[])MAC_ZERO, ETH_ALEN);
    else
        memcpy(ap.mac_receiver, mac_dst, ETH_ALEN);
    //inet_aton(IP_TARGET, &inaddr_receiver);
    //memcpy(&ap.ip_receiver, &inaddr_receiver, sizeof(inaddr_receiver));
    memcpy(ap.ip_receiver, ip_dst, sizeof(ap.ip_receiver));

    memset(&sl, 0, sizeof(sl));
    sl.sll_family = AF_PACKET;
    //sl.sll_addr = MAC_SOURCE;
    if (!if_from || strlen(if_from) < 1)
        sl.sll_ifindex = IFF_BROADCAST;//非常重要
    else
    {
        sl.sll_halen = ETH_ALEN;
        sl.sll_ifindex = if_nametoindex(if_from);
    }

    len = sendto(sfd, &ap, sizeof(ap), 0, (struct sockaddr*)&sl, sizeof(sl));
    if(-1 == len)
    {
        D("%s", strerror(errno));
        close(sfd);
        return -1;
    }

    close(sfd);
    return 0;
}

#else

#include "fio_context.h"
#include "fio_timer.h"
#include "fio_route_manager.h"
#include "fio_sysconfig.h"
#include "fio_sysstack.h"
#include "fio_map32.h"
//#include "log_log.h"
#include <assert.h>

#define STR_SHUTDOWN "c2h1dGRvd24="
#define STR_STARTUP "c2h1dGRvd25="
struct fio_mac_time_mgr
{
    uint64_t mac_elapse;    
    uint64_t mac_lifetime;
    uint64_t mac_buddy_elapse;    
    uint64_t mac_buddy_lifetime;
};
struct fio_mac_time_mgr g_mac_time_mgr;

const int g_arppkt_size = sizeof(struct arp_packet);

void fio_mac_pkt_factory_destroy(struct fio_mac_pkt_factory *factory)
{
    if ( factory->mem_)
        free(factory->mem_);
}

void fio_mac_pkt_factory_init(struct fio_mac_pkt_factory *factory, int num_nic)
{
    if (factory->inited)
        return;

    int i;
    factory->mem_ = calloc(FIO_MAC_MAX_FREEPKT, sizeof(struct fio_mac_pkt_data));
    FIO_INIT_LIST_HEAD(&factory->free_list); 
    for (i = 0; i < FIO_MAC_MAX_FREEPKT; i++)
        fio_list_add(&factory->mem_[i].free_link, &factory->free_list);
    factory->cnt = FIO_MAC_MAX_FREEPKT;
    factory->num_nic = num_nic;

    factory->inited = 1;
}

struct fio_mac_pkt_data * fio_mac_pkt_data_create(struct fio_mac_pkt_factory *factory)
{
    struct fio_list_head *pos;
    struct fio_mac_pkt_data *data = NULL;
    if (factory->cnt > 0)
    {
        pos = factory->free_list.next;
        data = fio_list_entry(pos, struct fio_mac_pkt_data, free_link);
        fio_list_del(&data->free_link);
        factory->cnt--;
    }
    else
        data = malloc(sizeof(struct fio_mac_pkt_data));
    return data;
}
void fio_mac_pkt_data_del(struct fio_mac_pkt_factory *factory, struct fio_mac_pkt_data *data)
{
    int n_index;
    fio_list_del(&data->nic_link);
    n_index = data - factory->mem_;
    if (n_index >= 0 && (unsigned long)n_index < FIO_MAC_MAX_FREEPKT)
    {
        fio_list_add(&data->free_link, &factory->free_list);
        factory->cnt++;
    }
    else
        free(data);
}
struct fio_mac_pkt_cache * fio_mac_pkt_cache_create(struct fio_mac_pkt_factory *factory)
{
    struct fio_mac_pkt_cache *cache;
    int j;
    cache = calloc(1, sizeof(struct fio_mac_pkt_cache));
    assert(cache);
    cache->pkt_list_nic = malloc(factory->num_nic*sizeof(struct fio_list_head));
    for (j = 0; j < factory->num_nic; j++)
        FIO_INIT_LIST_HEAD(cache->pkt_list_nic+j);
    return cache;
}
void fio_mac_pkt_cache_del(struct fio_mac_pkt_factory *factory, struct fio_mac_pkt_cache *cache)
{
    int i;
    for (i = 0; i < factory->num_nic; i++)
    {
        struct fio_list_head *head = cache->pkt_list_nic+i;
        while(!fio_list_empty(head)) 
            fio_mac_pkt_data_del(factory, 
                    fio_list_entry(head->next, struct fio_mac_pkt_data, nic_link));
    }
    free(cache->pkt_list_nic);
}

void fio_mac_destroy_buddy_mac(struct fio_context *mycontext)
{
    struct fio_node_t *nd;
    //uint32_t buddyip;
    struct fio_mac_pkt_cache *ptr_cache;
    for (nd = fio_map_first(mycontext->buddy_macs); nd; nd = fio_map_next(mycontext->buddy_macs, nd))
    {
        //buddyip = (uint32_t)nd->key;
        ptr_cache = (struct fio_mac_pkt_cache*)nd->val;
        fio_mac_pkt_cache_del(&mycontext->buddy_mac_pkt_factory, ptr_cache);
    }
    fio_map_destroy(mycontext->buddy_macs);
}

void fio_mac_del_buddy_mac(struct fio_nic *nic, uint32_t ip)
{
    struct fio_mac_pkt_cache *ptr_cache = NULL;
    if (fio_map_find_ptr(nic->buddy_macs, ip, (unsigned long*)&ptr_cache))
    {
        fio_mac_pkt_cache_del(nic->buddy_mac_pkt_factory, ptr_cache);
        fio_map_del(nic->buddy_macs, ip);
    }
}

void fio_mac_set_buddy_mac(struct fio_nic *nic, uint32_t ip, struct fio_mac_info **cur, struct ether_addr *ptr_mac)
{
    struct fio_mac_pkt_cache *ptr_cache = NULL;
    if (!fio_map_find_ptr(nic->buddy_macs, (unsigned long)ip, (unsigned long*)&ptr_cache))
    {
        ptr_cache = fio_mac_pkt_cache_create(nic->buddy_mac_pkt_factory);
        fio_map_set_ptr(nic->buddy_macs, (unsigned long)ip, ptr_cache);
    }
    ptr_cache->mac_info.ip = ip;
    memcpy(&ptr_cache->mac_info.mac, ptr_mac, ETH_ALEN);
    update_stat_lock(&ptr_cache->mac_info, MAC_EXIST);
    ptr_cache->mac_info.fail_time = 0;
    *cur = &ptr_cache->mac_info;
    struct fio_list_head *pkt_list = &ptr_cache->pkt_list_nic[nic->type_id], *pos,
                         *list_cache = &nic->pktcache_list_nic[nic->type_id];
    fio_list_for_each(pos, pkt_list) 
    {
        memcpy(&fio_list_entry(pos, struct fio_mac_pkt_data, nic_link)->mac, ptr_mac, ETH_ALEN);
    }
    fio_list_splice_tail(pkt_list, list_cache);
    FIO_INIT_LIST_HEAD(pkt_list);
}

int32_t fio_mac_gen_pack(struct arp_packet *ap, struct in_addr *ip_snd, struct ether_addr *mac_snd, 
        struct in_addr *ip_dst, struct ether_addr *mac_dst, uint16_t ar_op)
{
    memset(ap, 0, g_arppkt_size);
    if (ar_op == ARPOP_REQUEST && !mac_dst)
        memcpy(ap->mac_target, &(uint8_t[])MAC_BRCAST, ETH_ALEN);
    else
        memcpy(ap->mac_target, mac_dst, ETH_ALEN);
    memcpy(ap->mac_source, mac_snd, ETH_ALEN);
    ap->ethertype = g_arp_type/*htons(ETHERTYPE_ARP)*/;
    ap->hw_type = htons(0x1);
    ap->proto_type = htons(0x0800);
    ap->mac_addr_len = ETH_ALEN;
    ap->ip_addr_len = 4;
    ap->operation_code = htons(ar_op);
    memcpy(ap->mac_sender, mac_snd, ETH_ALEN);
    memcpy(ap->ip_sender, ip_snd, sizeof(ap->ip_sender));
    if (ar_op == ARPOP_REQUEST && !mac_dst)
        memcpy(ap->mac_receiver, &(uint8_t[])MAC_ZERO, ETH_ALEN);
    else
        memcpy(ap->mac_receiver, mac_dst, ETH_ALEN);
    memcpy(ap->ip_receiver, ip_dst, sizeof(ap->ip_receiver));

    return 0;
}

inline void update_stat_lock(struct fio_mac_info *cur, int8_t stat)
{
    cur->stat = stat;
}

inline int32_t dst_ip_is_gw(uint32_t ip)
{
    return (fio_route_defgw_stat() == fio_route_on 
            && fio_route_defgw() == ip);
}


#if 0
extern int fio_nic_backoff(struct fio_nic *nic, int buf_type, void *param);
static int fio_mac_reserve_pkt_cache(struct fio_nic *nic, struct fio_ringbuf *rbf, struct fio_txdata **m)
{
    int ret = fio_rbf_reserve_one(rbf, m);

    if (ret != 1)
    {
        if (fio_rbf_size(rbf) < MAX_PKT_COPY_ONCE)
        {
            if (0 != fio_rbf_expand(rbf, NIC_EXTRA_CONTEXT(nic)->me))
            {
                D("pkt_cache ring buf expend error!!!!");
                fio_nic_backoff(nic, TXDATA_TYPE_PKT_CACHE, rbf);
                assert(0);
            }
        }
        else
            fio_nic_backoff(nic, TXDATA_TYPE_PKT_CACHE, rbf);
        ret = fio_rbf_reserve_one(rbf, m);
    }

    (*m)->vtbl.reset(*m);

    return ret;
}
#endif

#if 1
inline int fio_mac_cache_pkt(struct fio_nic *dnic, uint32_t nip, struct fio_txdata *txd)
{
	NOD("*********");
    int ret = 0;
    struct fio_mac_pkt_cache *ptr_cache = NULL;
    if (!fio_map_find_ptr(dnic->buddy_macs, (unsigned long)nip, (unsigned long*)&ptr_cache))
        ret = -1;
    struct fio_mac_pkt_data *ptr_data = fio_mac_pkt_data_create(dnic->buddy_mac_pkt_factory);
    assert(ptr_data);
    memcpy(ptr_data->data, txd->pbuf, txd->size);
    ptr_data->size = txd->size;
    fio_list_add_tail(&ptr_data->nic_link, &ptr_cache->pkt_list_nic[dnic->type_id]);
    ret = 1;
    return ret;
}
#else
inline int fio_mac_cache_pkt(struct fio_nic *nic, uint32_t nip, struct fio_txdata *txd)
{
	NOD("*********");
    struct fio_mac_mgr *mac_mgr = &nic->mac_mgr;
    uint32_t ip_pos = (ntohl(nip)&(mac_mgr->vlan_size-1));
    struct fio_ringbuf *rbf = mac_mgr->pkt_cache_by_dip->next;
    int ret = 0;
    for (; rbf; rbf = rbf->next)
    {
        if (rbf->bf_id == ip_pos)
        {
            struct fio_txdata *txd_cache = NULL;
            if (fio_mac_reserve_pkt_cache(nic, rbf, &txd_cache) > 0)
            {
                memcpy(txd_cache->pbuf, txd->pbuf, txd->size);
                txd_cache->size = txd->size;
                ret = fio_rbf_commit(rbf, 1);
            }
            else
            {
                D("tid %d alloc pkt cache error", NIC_EXTRA_CONTEXT(nic)->me);
                ret = -1;
            }
            break;
        }
    }

    if (!rbf)
    {
        rbf = mac_mgr->free_pkt_cache_by_dip;
        mac_mgr->free_pkt_cache_by_dip = rbf->next;
        rbf->next = mac_mgr->tail_pkt_cache_by_dip->next;
        mac_mgr->tail_pkt_cache_by_dip->next = rbf;
        mac_mgr->tail_pkt_cache_by_dip = rbf;
        rbf->bf_id = ip_pos;
        struct fio_txdata *txd_cache = NULL;
        if (fio_rbf_reserve_one(rbf, &txd_cache) > 0)
        {
            memcpy(txd_cache->pbuf, txd->pbuf, txd->size);
            txd_cache->size = txd->size;
            ret = fio_rbf_commit(rbf, 1);
        }
        else
        {
            D("tid %d alloc pkt cache error at new", NIC_EXTRA_CONTEXT(nic)->me);
            ret = -1;
        }
    }

    return ret;
}
#endif

#if 1
int32_t fio_mac_get_bynip(struct fio_nic *dnic, uint32_t nip, struct ether_addr **mac, uint32_t sip)
{
    struct fio_mac_pkt_cache *ptr_cache = NULL;
    struct fio_mac_info *info;
    int stat;
    if (!fio_map_find_ptr(dnic->buddy_macs, (unsigned long)nip, (unsigned long*)&ptr_cache))
    {
        ptr_cache = fio_mac_pkt_cache_create(dnic->buddy_mac_pkt_factory);
        fio_map_set_ptr(dnic->buddy_macs, (unsigned long)nip, ptr_cache);
        ptr_cache->mac_info.ip = nip;
        update_stat_lock(&ptr_cache->mac_info, MAC_NOTEXIST);
        ptr_cache->mac_info.fail_time = 0;
    }
    info = &ptr_cache->mac_info;
    stat = info->stat;
    if (MAC_NOTEXIST == stat)
    {
        struct in_addr sin_nip, sin_sip;
        char str_nip_debug[16], str_sip_debug[16];
        sin_nip.s_addr = htonl(nip);
        inet_ntop(AF_INET, &sin_nip, str_nip_debug, 16);
        sin_sip.s_addr = htonl(sip);
        inet_ntop(AF_INET, &sin_sip, str_sip_debug, 16);
        info->fail_time = 0;
        info->stat = MAC_RESOLVING;
        OD( "tid %d %s request nip %s's mac from sip %s, stat %d->%d, fail_time %d", 
                NIC_EXTRA_CONTEXT(dnic)->me, dnic->alise, str_nip_debug, str_sip_debug, stat,
                info->stat, info->fail_time);
        fio_nic_send_one_mac(dnic, ARPOP_REQUEST, NULL, sin_nip.s_addr, sin_sip.s_addr,
                TXDATA_TYPE_PKT_ARP, NULL);
        goto ret_sendarp;
    }
    else if (MAC_RESOLVING == stat)
        goto ret_resolving;
    else
    {
        *mac = &info->mac;
        return 0;
    }
ret_resolving:
    if (dst_ip_is_gw(nip))
        goto ret_use_gw_cache;
    return 2;
ret_sendarp:
    if (dst_ip_is_gw(nip))
        goto ret_use_gw_cache;
    return 1;
ret_use_gw_cache:
    *mac = fio_route_defgw_mac();
    return 0;
}
#else
inline int32_t dst_ippos_is_gw(struct fio_mac_mgr *mac_mgr, uint32_t ip_pos)
{
    uint32_t ip_defgwpos = (fio_route_defgw()&(mac_mgr->vlan_size-1));
    return (fio_route_defgw_stat() == fio_route_on 
            && ip_defgwpos == ip_pos);
}
int32_t fio_mac_get_bynip(struct fio_nic *nic, uint32_t nip, struct ether_addr **mac, int isdefgw)
{
    if (isdefgw)
        goto ret_use_gw_cache;

    struct fio_mac_mgr *mac_mgr = &nic->mac_mgr;
    uint32_t ip_pos = (ntohl(nip)&(mac_mgr->vlan_size-1));
    struct fio_mac_info *cur = &mac_mgr->cur[ip_pos];

    int stat = cur->stat;
    if (MAC_NOTEXIST == stat)
    {
        cur->fail_time = 0;
        cur->stat = MAC_RESOLVING;
        OD( "tid %d %s send ip %d arp stat %d->%d, fail_time %d", 
                NIC_EXTRA_CONTEXT(nic)->me, nic->alise, ip_pos, stat, cur->stat, cur->fail_time);
        fio_nic_send_one_mac(nic, ARPOP_REQUEST, NULL, nip, nic->if_ipdef.s_addr,
                TXDATA_TYPE_PKT_ARP, NULL);
        goto ret_sendarp;
    }
    else if (MAC_RESOLVING == stat)
        goto ret_resolving;
    else
    {
        *mac = &cur->mac;
        return 0;
    }
ret_resolving:
    if (dst_ippos_is_gw(mac_mgr, ip_pos))
        goto ret_use_gw_cache;
    return 2;
ret_sendarp:
    if (dst_ippos_is_gw(mac_mgr, ip_pos))
        goto ret_use_gw_cache;
    return 1;
ret_use_gw_cache:
    *mac = fio_route_defgw_mac();
    return 0;
}
#endif

#ifdef __old_mac_mgr__
int32_t fio_mac_init(struct fio_mac_mgr *mgr, char *vlan/*, uint64_t itvl*/)
{
    if (!vlan || strlen(vlan) < 1)
        return -1;

    //mgr->mac_lifetime = itvl;

    uint32_t dst_raw, mask, range;
    ip2net_mask(vlan, &dst_raw, &mask, &range);
    mgr->vlan_size = range;
    mgr->vlan_mask = mask;
    mgr->vlan_prefix = (dst_raw & mask);

    mgr->info1 = malloc(range*sizeof(*mgr->info1));
    //g_mac_mgr.info2 = malloc(range*sizeof(*g_mac_mgr.info2));

    uint32_t i;
    if (!mgr->info1)
    {
        OD("alloc mac info array error");
        return -2;
    }
    for (i = 0; i < range; i++)
    {
        mgr->info1[i].ip = (i|mgr->vlan_prefix);
        //memcpy(&g_mac_mgr.info1[i].mac, &(uint8_t[])MAC_BRCAST, sizeof(struct ether_addr));
        mgr->info1[i].stat = MAC_NOTEXIST;
        //memcpy(&g_mac_mgr.info2[i].mac, &(uint8_t[])MAC_BRCAST, sizeof(struct ether_addr));
        //g_mac_mgr.info2[i].stat = MAC_NOTEXIST;
        mgr->info1[i].fail_time = 0;
    }
    mgr->cur = mgr->info1;

    //init_sig_mac(&mgr->sig);
    mgr->free_pkt_cache_by_dip = calloc(range, sizeof(*mgr->free_pkt_cache_by_dip));
    if (!mgr->free_pkt_cache_by_dip)
    {
        OD("alloc pkt cache array error");
        return -3;
    }
    for (i = 0; i < range-1; i++)
        mgr->free_pkt_cache_by_dip[i].next = &mgr->free_pkt_cache_by_dip[i+1];
    mgr->tail_pkt_cache_by_dip = mgr->pkt_cache_by_dip = calloc(1, sizeof(*mgr->pkt_cache_by_dip));

    return 0;
}
#endif

static void fio_mac_update_all(struct fio_nic *from, struct fio_mac_info **cur, uint32_t ip, struct arp_packet *pr_arp)
{
    if (0 != fio_mp_mask_add_domainip(from->mp_ipinfo.arr_mask_idx, from->mp_ipinfo.mask_cnt, ip))
        return;
    fio_mac_set_buddy_mac(from, ip, cur, (struct ether_addr*)pr_arp->mac_sender);
    struct fio_notify_msg msg = {.msg_id = FIO_MSG_UPMAC, .nic_id = from->type_id};
    struct fio_mac_info *pmac_info = malloc(sizeof(struct fio_mac_info));
    memcpy(pmac_info, *cur, sizeof(struct fio_mac_info));
    msg.abstract = (unsigned long)pmac_info;
    fio_stack_notify_systhread(from, &msg);
}
int fio_mac_handle_arp(struct fio_nic *from, struct fio_nic *to, struct fio_rxdata *rxdata, int pre_realtime, void *param)
{
    uint32_t ip;
    struct arp_packet *pr_arp = (struct arp_packet*)rxdata->pbuf;
    uint16_t eh_flag = ntohs(pr_arp->operation_code);
    struct fio_mac_info *cur = NULL;
    int ret = 2;
    fio_stack_deliver(from, rxdata);
    if ( eh_flag == ARPOP_REPLY ) 
    {
        char rcv_addr[16];
        char snd_addr[16];
        inet_ntop(AF_INET, (struct in_addr*)(pr_arp->ip_receiver), rcv_addr, 16);
        inet_ntop(AF_INET, (struct in_addr*)(pr_arp->ip_sender), snd_addr, 16);
        if (sysconfig.prsys_stack)
            OD( "tid %d %s recv arp reply dstip %s, sndip %s", NIC_EXTRA_CONTEXT(from)->me,
                    from->alise, rcv_addr, snd_addr);
        ip = ntohl(((struct in_addr*)(pr_arp->ip_sender))->s_addr);
        fio_mac_update_all(from, &cur, ip, pr_arp);
        if (cur)
            fio_route_update_defgw_mac(ip, &cur->mac);
    }
    else if ( eh_flag == ARPOP_REQUEST ) 
    {
        struct arp_packet r_arp;
        struct arp_packet *preq_arp = pr_arp;
        if (TXDATA_TYPE_REALTIME == pre_realtime)
        {
            memcpy(&r_arp, preq_arp, g_arppkt_size);
            preq_arp = &r_arp;
        }
        struct in_addr *ip_recv = (struct in_addr*)(preq_arp->ip_receiver);
        //struct ether_addr *mac_send = NULL;
        char rep_addr[16];
        char req_addr[16];
        inet_ntop(AF_INET, ip_recv, rep_addr, 16);
        inet_ntop(AF_INET, (struct in_addr*)(preq_arp->ip_sender), req_addr, 16);
        NOD("tid %d %s recv arp request dstip %s, sndip %s", NIC_EXTRA_CONTEXT(from)->me,
                from->alise, rep_addr, req_addr);
        if (!strncmp((const char*)preq_arp->padding, STR_SHUTDOWN, strlen(STR_SHUTDOWN)))
        {   
            sysconfig.working = 2;
        }  
        else if (!strncmp((const char*)preq_arp->padding, STR_STARTUP, strlen(STR_STARTUP)))
        {   
            sysconfig.working = 1;
        }  
        ip = ip_recv->s_addr;
        if (fio_map_exist(from->mp_ipinfo.if_localaddrs, ntohl(ip)))
        {
            NOD("tid %d %s find local ip %s, sndip %s", NIC_EXTRA_CONTEXT(from)->me,
                from->alise, rep_addr, req_addr);
            //mac_send = &from->if_mac;
        }
        else
            return ret;
        NOD("");
        fio_nic_send_one_mac(from, ARPOP_REPLY, (struct ether_addr*)preq_arp->mac_sender, 
                *(uint32_t*)preq_arp->ip_sender, ip, 
                (TXDATA_TYPE_REALTIME==pre_realtime) ? pre_realtime : TXDATA_TYPE_PKT_ARP, rxdata);
        OD( "tid %d %s generate arp packet to %s realtime %d", NIC_EXTRA_CONTEXT(from)->me, from->alise,
                inet_ntop(AF_INET, (struct in_addr*)(preq_arp->ip_sender), rep_addr, 16), pre_realtime);
        ip = ntohl(*(uint32_t*)preq_arp->ip_sender);

        fio_mac_update_all(from, &cur, ip, preq_arp);

        if (cur)
            fio_route_update_defgw_mac(ip, &cur->mac);
    }

    return ret;
}


#ifdef __old_mac_mgr__
void fio_mac_init_pkt_cache(struct fio_ringbuf *rbf, struct fio_txdata *txb, int tid, int _id)
{
    txb->parent = rbf;
    txb->id_ = ((TXDATA_TYPE_PKT_CACHE<<TXDATA_TYPE_MASK_OFFSET) | (tid<<TXDATA_TYPE_CACHEID_OFFSET) | _id);
    txb->type = TXDATA_TYPE_PKT_CACHE;
    txb->pbuf = &rbf->buf_if_necessary[_id * MAX_PKT_SIZE];
    txb->pdata = txb->pbuf+g_payload_offset;
    txb->size = 0;
}
#endif

#if 1
inline int fio_mac_check_cache(struct fio_nic *nic)
{
    nic->vtbl.send_pktcache(nic);
    return 0;
}

void fio_mac_pktcache_list_destroy(struct fio_mac_pkt_factory *factory, struct fio_list_head *head)
{
    while(!fio_list_empty(head)) 
    {
        fio_mac_pkt_data_del(factory, fio_list_entry(head->next, struct fio_mac_pkt_data, nic_link));
    }
}

struct ether_addr * fio_mac_byip(struct fio_nic *nic, uint32_t nip)
{
    uint32_t hip = ntohl(nip);
    struct fio_mac_pkt_cache *ptr_cache = NULL;
    struct fio_mac_info *info;
    if (!fio_map_find_ptr(nic->buddy_macs, (unsigned long)hip, (unsigned long*)&ptr_cache))
    {
        ptr_cache = fio_mac_pkt_cache_create(nic->buddy_mac_pkt_factory);
        fio_map_set_ptr(nic->buddy_macs, (unsigned long)hip, ptr_cache);
        ptr_cache->mac_info.ip = hip;
        update_stat_lock(&ptr_cache->mac_info, MAC_NOTEXIST);
        ptr_cache->mac_info.fail_time = 0;
    }
    info = &ptr_cache->mac_info;
    if (MAC_EXIST == info->stat || MAC_UPDATING == info->stat)
        return &info->mac;
    else
    {
        if (0 == fio_mp_mask_add_domainip(nic->mp_ipinfo.arr_mask_idx,
                    nic->mp_ipinfo.mask_cnt, hip))
        {
            update_stat_lock(info, MAC_RESOLVING);
            nic->spec_tasks |= FIO_SPECTSK_SNDMAC;
        }
    }
    return NULL;
}
#else
inline int fio_mac_check_cache(struct fio_nic *nic)
{
    int cur_stat;
    struct fio_mac_mgr *mac_mgr = &nic->mac_mgr;
    struct fio_ringbuf *rbf = mac_mgr->pkt_cache_by_dip->next;
    struct fio_ringbuf *rbf_pre = mac_mgr->pkt_cache_by_dip;
    for (; rbf; rbf = rbf_pre->next)
    {
        cur_stat = mac_mgr->cur[rbf->bf_id].stat;
        rbf->pmac = &mac_mgr->cur[rbf->bf_id].mac;
        if (MAC_NOTEXIST != cur_stat && MAC_RESOLVING != cur_stat)
            if (nic->vtbl.send(nic, TXDATA_TYPE_PKT_CACHE, fio_rbf_occupy(rbf), rbf) < 1)
                break;
        if (fio_rbf_occupy(rbf) < 1)
        {
            rbf_pre->next = rbf->next;
            rbf->next = mac_mgr->free_pkt_cache_by_dip;
            mac_mgr->free_pkt_cache_by_dip = rbf;
            if (mac_mgr->tail_pkt_cache_by_dip == rbf)
                mac_mgr->tail_pkt_cache_by_dip = rbf_pre;
        }
        else
            rbf_pre = rbf;
    }
    return 0;
}

struct ether_addr * fio_mac_byip(struct fio_nic *nic, uint32_t nip)
{
    uint32_t hip = ntohl(nip);
    struct fio_mac_mgr *mac_mgr = &nic->mac_mgr;
    struct fio_mac_info *cur;
    if ((hip&mac_mgr->vlan_mask) == mac_mgr->vlan_prefix)
    {
        cur = &mac_mgr->cur[(hip & (mac_mgr->vlan_size-1))];
        if (MAC_EXIST == cur->stat || MAC_UPDATING == cur->stat)
            return &cur->mac;
        else
        {
            update_stat_lock(cur, MAC_RESOLVING);
            nic->spec_tasks |= FIO_SPECTSK_SNDMAC;
        }
    }
    return NULL;
}
#endif
static int fio_mac_timeout(uint64_t interval, uint64_t *const history, uint64_t elapse) 
{
    *history += elapse; 
    if (*history < interval)
        return 0; 
    *history -= interval; 
    return 1;
}
void fio_mac_brcast(uint64_t elapse)
{
    static int pos = 0;
    int i, j;
    struct fio_context *context;
    if (!fio_mac_timeout(g_mac_time_mgr.mac_lifetime, &g_mac_time_mgr.mac_elapse, elapse))
        return ;
    for (i = 0; i < sysconfig.nthreads; i++)
    {
        context = g_contexts+(pos++%sysconfig.nthreads);
        if (sysconfig.prmac_info)
            sysconfig.maclog.vtbl.print(&sysconfig.maclog, "tid %d txblock %d\n", 
                    context->me, context->txblock);
        if (!context->txblock)
        {
            for (j = 0; j < context->num_nic; j++)
                context->nics[j].spec_tasks |= FIO_SPECTSK_SNDMAC;
            break;
        }
    }
}
void fio_mac_clear_buddymac(uint64_t elapse)
{
    static int pos = 0;
    struct fio_context *context;
    if (!fio_mac_timeout(g_mac_time_mgr.mac_buddy_lifetime, &g_mac_time_mgr.mac_buddy_elapse, elapse))
        return ;
    context = g_contexts+(pos++%sysconfig.nthreads);
    context->nics[0].spec_tasks |= FIO_SPECTSK_CLEARMAC;
}
void fio_mac_init_time_mgr(struct sysconfig *_sysconfig)
{
    g_mac_time_mgr.mac_lifetime = _sysconfig->mac_lifetime;
    g_mac_time_mgr.mac_buddy_lifetime = _sysconfig->mac_buddy_lifetime;
    fio_register_timer_handler(FIO_T_CHECK_MAC, fio_mac_brcast);
    fio_register_timer_handler(FIO_T_CHECK_BUDDYMAC, fio_mac_clear_buddymac);
}

#endif

#if (defined _TEST_MAC_1)
int get_if_ip(const char * const eth, struct in_addr *ipaddr)
{
    int sock_fd;
    struct  sockaddr_in my_addr;
    struct ifreq ifr;

    /**//* Get socket file descriptor */
    if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
    {
        D("open sock dgram, errno %s", strerror(errno));
        return -1;
    }

    /**//* Get IP Address */
    strncpy(ifr.ifr_name, eth, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ-1]='\0';

    if (ioctl(sock_fd, SIOCGIFADDR, &ifr) < 0)
    {
        D("No Such Device %s",eth);
        return -1;
    }

    memcpy(&my_addr, &ifr.ifr_addr, sizeof(my_addr));
    *ipaddr = my_addr.sin_addr;
    D("if %s, ip %s", eth, inet_ntoa(*ipaddr));
    close(sock_fd);
    return 0;
}

int ip2net_mask(const char *str, uint32_t *src_raw, uint32_t *mask, uint32_t *range)
{
    __label__ out_free, out_err;
    char *c, *s, *s_raw;
    int ret_val;
    struct sockaddr_in sa;

    s = strdup(str);
    s_raw = s;
    if (!s_raw)
        return 0;

    memset(&sa, 0, sizeof(sa));
    *src_raw = 0;
    *mask = ~0U;
    *range = 1;

    if ((c = strrchr(s, '/')) != NULL) {
        *c++ = '\0';
        /* c points to the mask */
        char *err;
        if(inet_pton(AF_INET, s, &sa.sin_addr))
        {
            *src_raw = ntohl((unsigned long)(sa.sin_addr.s_addr));
            *mask = (~0U << (32-strtol(c, &err, 10)));
            *range = (1U << (32-strtol(c, &err, 10)));
        }
        else
            goto out_err;
    }
    else
    {
        if (!inet_pton(AF_INET, s, &sa.sin_addr)) {
            struct hostent *he;

            if ((he = gethostbyname(s)) == NULL) {
                goto out_err;
            }
            else
            {
                sa.sin_addr = *(struct in_addr *) *(he->h_addr_list);
                *src_raw = ntohl((unsigned long)(sa.sin_addr.s_addr));
            }
        }
    }

    //sa.sin_family = AF_INET;
    ret_val = 1;
out_free:
    free(s_raw);
    return ret_val;
out_err:
    ret_val = 0;
    goto out_free;
}

int source_hwaddr(const char *ifname, char *buf)
{
    struct ifaddrs *ifaphead, *ifap;
    int l = sizeof(ifap->ifa_name);

    if (getifaddrs(&ifaphead) != 0) {
        printf("getifaddrs %s failed\n", ifname);
        return (-1);
    }

    for (ifap = ifaphead; ifap; ifap = ifap->ifa_next) {
        struct sockaddr_ll *sdl =
            (struct sockaddr_ll *)ifap->ifa_addr;
        uint8_t *mac;

        if (!sdl || sdl->sll_family != AF_PACKET)
            continue;
        if (strncmp(ifap->ifa_name, ifname, l) != 0)
            continue;
        mac = (uint8_t *)sdl->sll_addr;
        sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
                mac[0], mac[1], mac[2],
                mac[3], mac[4], mac[5]);
        break;
    }
    freeifaddrs(ifaphead);
    return ifap ? 0 : 1;
}

void usage_quit(char *arg0)
{
    OD( "Usage: %s ", arg0);
    exit(1);
}

int32_t broadcast_arp(char *ip, char *ifname)
{
    socklen_t salen;
    int recvfd, n, try; 
    char str_mac[] = "00:00:00:00:00:00";
    char mac[ETH_ALEN];
    char recv_buf[120], rep_addr[16], req_addr[16];
    struct in_addr ip_sender, ip_receiver;
    struct sockaddr_ll repsa;
    uint8_t mac_receiver[ETH_ALEN] = MAC_ZERO;
    struct pollfd fds[1];
    uint32_t dst_raw, mask, range, i;

    ip2net_mask(ip, &dst_raw, &mask, &range);

    if(get_if_ip(ifname, &ip_sender)) {
        D("Error: Get host’s information failed");
        return -1;
    }

    source_hwaddr(ifname, str_mac);
    bcopy(ether_aton(str_mac), mac, ETH_ALEN);

    for (i = 0; i < range; i++)
    {
        //inet_aton(argv[1], &ip_receiver);
        ip_receiver.s_addr = htonl((dst_raw & mask) | i);
        inet_ntop(AF_INET, &ip_receiver, req_addr, 16);

        mac_pack(&ip_sender, (struct ether_addr *)mac, &ip_receiver, 
                (struct ether_addr*)mac_receiver, ARPOP_REQUEST, ifname);

        printf("Broadcast arp request of %s\n\n", req_addr);
        recvfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
        bzero(recv_buf, sizeof(recv_buf));
        bzero(&repsa, sizeof(repsa));
        salen = sizeof(struct sockaddr_ll);
        memset(fds, 0, sizeof(fds));
        fds[0].fd = recvfd;
        fds[0].events = POLLIN;
        try = 0;
        while (try++ < 10)
        {
            if ((n = poll(fds, 1, 2000)) > 0)
            {
                if((n = recvfrom(recvfd, recv_buf, sizeof(struct arp_packet), 0, 
                                (struct sockaddr *)&repsa, &salen)) <= 0)
                {
                    D("Recvfrom error");
                    continue;
                }
                if( ntohs(*(__be16 *)(recv_buf + 20))==2 && !memcmp(&ip_receiver, recv_buf + 28, 4) ) 
                {
                    inet_ntop(AF_INET, (struct in_addr *)(recv_buf + 28), rep_addr, 16);
                    printf("Response from %s, %d bytes received\n", req_addr, n);
                    printf(" Peer IP is: %s\n", rep_addr);
                    prmac( (u_char *)(recv_buf + 22) ); //prmac( (u_char *)(recv_buf + 6) );
                }
            }
        }
        close(recvfd);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if(argc != 3)
        usage_quit(argv[0]);

    broadcast_arp(argv[1], argv[2]);

    return 0;
}
#endif

void prmac(uint8_t *ptr)
{
    OD( " MAC is: %02x:%02x:%02x:%02x:%02x:%02x",
            *ptr,*(ptr+1),*(ptr+2),*(ptr+3),*(ptr+4),*(ptr+5));
}

#ifdef _TEST_MAC_
int main()
{
    struct in_addr inaddr_sender, inaddr_receiver;
    uint8_t mac_sender[ETH_ALEN] = MAC_SOURCE;
    uint8_t mac_receiver[ETH_ALEN] = MAC_TARGET;

    D("***** %d ", sizeof(struct ether_header)+sizeof(struct ip)+sizeof(struct udphdr));

    inet_aton(IP_TARGET, &inaddr_receiver);
    inet_aton(IP_TRICK, &inaddr_sender);
    mac_pack(&inaddr_sender, (struct ether_addr*)mac_sender, &inaddr_receiver,
            NULL/*(struct ether_addr*)mac_receiver*/, ARPOP_REQUEST /*ARPOP_REPLY*/, "eth3");

    return 0;
}
#endif

