//	$Id: fio_mac_manager.h 2013-05-23 likunxiang$
//
#ifndef __MAC_MANAGER_H__
#define __MAC_MANAGER_H__

#include <inttypes.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include "fio_ringbuf.h"
#include "fio_list.h"

#define MAC_BRCAST {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
#define MAC_ZERO {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

struct sysconfig;
struct fio_rxdata;
struct fio_nic;
struct fio_context;

struct arp_packet
{
	//DLC Header
	//接收方mac
	uint8_t mac_target[ETH_ALEN];
	//发送方mac
	uint8_t mac_source[ETH_ALEN];
	//Ethertype - 0x0806是ARP帧的类型值
	uint16_t ethertype;

	//ARP Frame
	//硬件类型 - 以太网类型值0x1
	uint16_t hw_type;
	//上层协议类型 - IP协议(0x0800)
	uint16_t proto_type;
	//MAC地址长度
	uint8_t mac_addr_len;
	//IP地址长度
	uint8_t ip_addr_len;
	//操作码 - 0x1表示ARP请求包,0x2表示应答包
	uint16_t operation_code;
	//发送方mac
	uint8_t mac_sender[ETH_ALEN];
	//发送方ip
	uint8_t ip_sender[4];
	//接收方mac
	uint8_t mac_receiver[ETH_ALEN];
	//接收方ip
	uint8_t ip_receiver[4];
	//填充数据
	uint8_t padding[18];
};
extern const int g_arppkt_size;

enum
{
    MAC_EXIST,
    MAC_NOTEXIST,
    MAC_RESOLVING,
    MAC_UPDATING,
};

struct fio_mac_info
{
    uint32_t ip;
    struct ether_addr mac;
    int8_t stat;
    int32_t    fail_time;
};

struct fio_mac_pkt_cache
{
    struct fio_list_head *pkt_list_nic;
    struct fio_mac_info mac_info;
};
struct fio_mac_pkt_data
{
    struct fio_list_head free_link;
    struct fio_list_head nic_link;
    struct ether_addr mac;
    int size;
    char data[MAX_PKT_SIZE];
};
struct fio_mac_pkt_factory
{
#define FIO_MAC_MAX_FREEPKT 2000
    struct fio_list_head free_list;
    struct fio_mac_pkt_data *mem_;
    uint32_t cnt;
    int inited;
    int num_nic;
};
struct fio_mac_mgr
{
    uint64_t mac_lifetime;    
#ifdef __old_mac_mgr__
    uint32_t vlan_size;
    uint32_t vlan_mask;
    uint32_t vlan_prefix;
    struct fio_mac_info *cur;
    struct fio_mac_info *info1;
    struct fio_ringbuf *pkt_cache_by_dip;
    struct fio_ringbuf *tail_pkt_cache_by_dip;
    struct fio_ringbuf *free_pkt_cache_by_dip;
#endif
};
#ifdef __old_mac_mgr__
int32_t fio_mac_init(struct fio_mac_mgr *mgr, char *vlan/*, uint64_t itvl*/);
void fio_mac_init_pkt_cache(struct fio_ringbuf *rbf, struct fio_txdata *txb, int tid, int _id);
#endif

int32_t fio_mac_gen_pack(struct arp_packet *ap, struct in_addr *ip_snd, struct ether_addr *mac_snd, 
        struct in_addr *ip_dst, struct ether_addr *mac_dst, uint16_t ar_op);
int32_t init_mac_tbl(struct sysconfig *sysconfig);
int32_t fio_mac_get_bynip(struct fio_nic *nic, uint32_t nip, struct ether_addr **mac, uint32_t isdefgw);
void prmac(uint8_t *ptr);
inline void update_stat_lock(struct fio_mac_info *cur, int8_t stat);
int fio_mac_handle_arp(struct fio_nic *from, struct fio_nic *to, struct fio_rxdata *rxdata, int pre_realtime, void *param);
inline int fio_mac_check_cache(struct fio_nic *nic);
inline int fio_mac_cache_pkt(struct fio_nic *nic, uint32_t nip, struct fio_txdata *txd);
struct ether_addr * fio_mac_byip(struct fio_nic *nic, uint32_t nip);
void fio_mac_pkt_factory_init(struct fio_mac_pkt_factory *factory, int num_nic);
void fio_mac_set_buddy_mac(struct fio_nic *nic, uint32_t ip, struct fio_mac_info **cur, struct ether_addr *ptr_mac);
void fio_mac_del_buddy_mac(struct fio_nic *nic, uint32_t ip);
void fio_mac_pkt_data_del(struct fio_mac_pkt_factory *factory, struct fio_mac_pkt_data *data);
struct fio_mac_pkt_data * fio_mac_pkt_data_create(struct fio_mac_pkt_factory *factory);
void fio_mac_init_time_mgr(struct sysconfig *_sysconfig);
void fio_mac_destroy_buddy_mac(struct fio_context *mycontext);
void fio_mac_pkt_factory_destroy(struct fio_mac_pkt_factory *factory);
void fio_mac_pktcache_list_destroy(struct fio_mac_pkt_factory *factory, struct fio_list_head *head);

#endif
