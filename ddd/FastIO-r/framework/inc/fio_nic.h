//	$Id: fio_nic.h 2013-05-23 likunxiang$
//

#ifndef _FIO_NIC_H_
#define _FIO_NIC_H_

#include "fio_mac_manager.h"
#include "fio_nm_util.h"

#define FIO_DIR_TX              (0x1)
#define FIO_DIR_RX              (0x2)
#define __FIO_DIR_RXOPTIMIZED   (0x4)
#define FIO_DIR_RXOPTIMIZED     (FIO_DIR_RX | __FIO_DIR_RXOPTIMIZED)

#define FIO_LIMIT_FROM          0
#define FIO_LIMIT_TO            1

//struct txinfo;

struct fio_nic_vtbl
{
    int (*send)(struct fio_nic *nic, int realtime, int limit, void *param);
    int (*recv)(struct fio_nic *from, struct fio_asted_rxs *bufs, uint32_t limit);
    int (*send_sys)(struct fio_nic *sys_nic, struct fio_nic *nic, int limit, void *param);
    int (*send_pktcache)(struct fio_nic *nic);
};

struct fio_nic 
{ 
    struct fio_nic_vtbl vtbl;
    int fd; 
    int type_id; 
    struct nmreq nmr; 
    struct netmap_if *nifp; 
    uint32_t dir;
    uint16_t qrxfirst, qtxfirst, qrxlast, qtxlast, cur_rx_head, cur_tx_head;
    int rxavl, nictxavl, txfast, txslow;
    struct ether_addr if_mac; 
    struct in_addr spec_ip;
    struct fio_asted_rxs spec_rxpkts;
    struct fio_mp_ipinfo mp_ipinfo;
    struct fio_map_t *buddy_macs;
    struct fio_route_context *route_context;
    struct fio_route_mask *defroute_context;
    struct fio_list_head *pktcache_list_nic;
    struct fio_mac_pkt_factory *buddy_mac_pkt_factory;
    uint64_t rxcount, txcount, unuse_count, sysstack_count; 
#ifdef __old_mac_mgr__
    struct fio_mac_mgr mac_mgr;
#endif
    uint32_t spec_tasks;
    void *private_; 
    char *alise; 
    uint32_t if_flags;

    struct fio_ringbuf bg_buf;
    struct fio_ringbuf tx_buf;
    struct fio_ringbuf upsnd_buf;
    struct fio_ringbuf arp_buf;
    struct fio_ringbuf sys_txbuf;
    struct fio_ringbuf sys_rxbuf;
    
    void *mmap_addr;
    uint32_t mmap_size;
    uint16_t seq_num;
    struct pktudp *pkt_test;
};

inline struct fio_map_t * fio_nic_get_localips(struct fio_nic *nic);
inline int fio_nic_is_myip(struct fio_nic *nic, uint32_t ip_network);
inline struct fio_mapstr_t * fio_nic_get_arpa(struct fio_nic *nic);
inline int fio_nic_is_myarpa(struct fio_nic *nic, const char *strkey);
int fio_nic_snd_alldomain_mac(struct fio_nic *nic);
int fio_nic_send_one_mac(struct fio_nic *nic, uint16_t op, struct ether_addr *recv_mac,
        uint32_t recv_ip, uint32_t snd_ip, int realtime, void *param);
int fio_nic_sendmsg_direct(struct fio_nic *nic, uint32_t dstip, struct fio_txdata *txd);
int fio_send(struct fio_nic *dnic, uint16_t data_len, struct fio_txdata *txd, int need_csum);
int fio_upsend(int nic_id, uint16_t data_len, struct fio_txdata *txd, int need_csum);
void fio_flushup(int nic_id);
inline int fio_nic_reserve_tx(struct fio_nic *nic, struct fio_txdata **m, void *param);
inline int fio_nic_ioctltx(struct fio_nic *nic);
inline int fio_nic_txlimit(struct fio_nic *nic);
inline int fio_nic_rxlimit(struct fio_nic *nic);
inline int fio_nic_send_txbuf(struct fio_nic *nic);
inline int fio_nic_send_sys_rxbuf(struct fio_nic *nic);
inline int fio_nic_send_upsndbuf(struct fio_nic *nic);
inline int fio_nic_send_arpbuf(struct fio_nic *nic);
inline int fio_nic_send_bgbuf(struct fio_nic *nic);
inline int fio_nic_commit(struct fio_nic *nic, struct fio_txdata *txd, int count);
int fio_nic_info(struct nmreq *nmr, const char * const ifname);
int fio_nic_open(struct fio_nic *nic, char *alise, const char * const ifname, uint16_t rid, uint32_t dir, void *prv);
inline void fio_nic_init(struct fio_nic *nic, int txb_size, int bg_size/*, struct fio_mac_mgr *mac_mgr*/);
inline int fio_nic_close(struct fio_nic *nic);
int fio_recv_pkts(struct netmap_ring *ring, struct fio_nic *nic, struct fio_asted_rxs *bufs, uint32_t limit);
inline struct fio_nic * fio_nic_fromip(const char *strip);
inline struct fio_nic * fio_nic_fromip_nic(struct fio_nic *from, const char *strip);
int fio_nic_send_pkts(struct netmap_ring *ring, struct fio_nic *nic, struct fio_ringbuf *rbf,
        int txd_type, uint32_t count);
inline int fio_upsend_reserve_tx(int nic_id, struct fio_txdata **m);
#endif
