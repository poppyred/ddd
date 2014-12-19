//	$Id: fio_context.h 2013-05-23 likunxiang$
//

#ifndef _FIO_CONTEXT_H_
#define _FIO_CONTEXT_H_

#include "fio_nic.h"
#include "fio_conf.h"
#include "fio_lfqueue.h"
#include "fio_route_manager.h"

#define FIO_LSTN_ALL_RING NETMAP_RING_MASK
#define FIO_SYS_RING_MASK 0xf000
#define FIO_SYS_RING      NETMAP_SW_RING
#define NIC_EXTRA_CONTEXT(nic) ((struct fio_context*)((nic)->private_))
#define NIC_EXTRA_TYPEID(nic) ((nic)->type_id)

#define FIO_MODE_N_BRG_IN           0
#define FIO_MODE_1_SND_AND_RCV      1
#define FIO_MODE_1_SND_OR_RCV       2
#define FIO_MODE_2CARDS             3
#define FIO_MODE_N_BRG_OUT          4
#define FIO_MODE_1CPY_2SND_N_PROC   5
#define FIO_MODE_TEST_SND           6
#define FIO_MODE_BRG_SINGLE         7
#define FIO_MODE_RAWSOCK            99

#define FIO_MSG_SYSSTACK            0
#define FIO_MSG_UPMAC               1
#define FIO_MSG_DEL_BUDDYIP         2
#define FIO_MSG_CLEAR_MACPOOL       3
#define FIO_MSG_UPSNDBUF            4

struct fio_notify_msg
{
    uint32_t msg_id;
    int nic_id;
    unsigned long abstract;
};

//struct txinfo
//{
//    struct netmap_ring *txring; 
//    struct netmap_slot *txslot;
//    uint32_t *cur_tx;
//    uint32_t *m_mov;
//};

struct fio_poll_data
{
    struct fio_nic **nics;
    struct pollfd *fds;
    uint32_t *dirs;
    int num_nic;
    int *notifyfds;
    int num_notify;
};
struct fio_ct_vtbl
{
    int (*polls)(struct fio_poll_data *pd, int sec);
    int (*recv)(struct fio_nic *from, struct fio_nic *to, struct fio_asted_rxs *bufs, uint32_t limit, int accord);
};

struct fio_context {
    struct fio_ct_vtbl vtbl;
    int me;
    struct fio_nic nics[FIO_MAX_NICS];
    int num_nic;
    struct timeval tic, toc;
    //struct fio_mac_mgr mac_mgr;
    int txblock;
    //int8_t ip_hashtbl[0xffff];

    struct fio_map_t *buddy_macs;
    struct fio_route_context route_context;
    struct fio_route_mask defroute_context;
    struct fio_mac_pkt_factory buddy_mac_pkt_factory;
    struct fio_list_head *pktcache_list_nic;
    pthread_t thread;
    int used;
    int *notifyfds;
    LFQueue *q_recv;
    //just for test
    int n_test;
};
extern struct fio_context *g_contexts;
extern struct fio_context *g_syscontext;

inline int fio_assort(struct fio_nic *nic, char *pkt, uint16_t *eh_type, uint8_t *ip_type);

int fio_init();
int fio_start();
int fio_shutdown();
int fio_wait();
int fio_start_statistics();
struct fio_context * fio_get_hwl();
void fio_sigint_h(int sig);

#endif
