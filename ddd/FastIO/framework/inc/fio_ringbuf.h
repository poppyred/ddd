//	$Id: fio_ringbuf.h 2013-05-23 likunxiang$
//
#ifndef __FIO_RINGBUF_HEAD__
#define __FIO_RINGBUF_HEAD__

#include <inttypes.h>
#include <linux/if_ether.h>
#include <netinet/ether.h>

//#define USE_BF_OCCUPY
#define MAX_PKT_SIZE ETH_FRAME_LEN
extern const int MAX_PAYLOAD_SIZE;
#define MAX_PKT_COPY_ONCE (512*1)
#define MAX_ARP_CACHE (10)

#define RBF_DELTA        2

#define TXDATA_TYPE_MASK_OFFSET 29
#define TXDATA_TYPE_CACHEID_OFFSET 24
#define TXDATA_TYPE_MASK (1 << TXDATA_TYPE_MASK_OFFSET)
#define TXDATA_TYPE_NORMAL 0
#define TXDATA_TYPE_REALTIME 1
#define TXDATA_TYPE_PKT_CACHE 2
#define TXDATA_TYPE_PKT_ARP 3
#define TXDATA_TYPE_SYSSTACK_TX 4
#define TXDATA_TYPE_SYSSTACK_RX 5
#define TXDATA_TYPE_PKT_MACCHE  6
#define TXDATA_TYPE_UPSND 7

enum
{
    T_FIO_PKT_INTD = 0,
    T_FIO_PKT_ETH,
    T_FIO_PKT_DEF,
    T_FIO_PKT_MAX,
    T_FIO_PKT_DISCARD,
};

struct fio_rxdata
{
    int size;
    uint16_t eh_type;
    uint8_t ip_type; 
    struct ether_addr smac;
    uint32_t sip;
    uint32_t dip;
    uint16_t sport;
    uint16_t dport;
    char *pbuf;

    struct netmap_slot *slot_rx;
    //char buf[MAX_PAYLOAD_SIZE];
};

struct fio_txdata;
struct fio_ringbuf;
struct _txdata_vtbl
{
    void (*init)(struct fio_ringbuf *rbf, struct fio_txdata *txb,  int _tid, int _id);
    void (*set_buf_ptr)(struct fio_txdata *txb, struct fio_rxdata *rxb);
    void (*reset)(struct fio_txdata *txb);
};

struct fio_txdata
{
    struct _txdata_vtbl vtbl;
    int id_;
    int type;
    struct ether_addr *dmac;
    uint32_t dstip;
    uint16_t dstport;
    //struct ether_addr *smac;
    uint32_t srcip;
    uint16_t srcport;
    int size;
    void *parent;
    struct netmap_slot *slot_rx;
    char *pdata;
    char *pbuf;
};

struct fio_asted_rxs
{
    struct fio_rxdata rxds[MAX_PKT_COPY_ONCE];
    int avail;
    int cb_type;
};

struct _ringbuf_vtbl
{
    void (*alloc_if_buf)(struct fio_ringbuf *rbf, int total);
    void (*realloc_if_buf)(struct fio_ringbuf *rbf, int presize);
};

struct fio_ringbuf
{
    struct _ringbuf_vtbl vtbl;
    int bf_id;
    int bf_size; 
    void *owner;
    struct fio_txdata *bf_head; 
    struct fio_txdata *bf_tail; 
    struct fio_txdata *bf_end; 
    struct fio_txdata *bf_bufs;  //目前单线程使用
#ifdef USE_BF_OCCUPY
    int bf_occupy;
	int lock;
#endif
    struct ether_addr *pmac;
    struct fio_ringbuf *next;
    char   *buf_if_necessary;
};

inline int fio_rbf_init(struct fio_ringbuf *bf, int tid, int bf_size, struct _ringbuf_vtbl *rbfvtbl,
        struct _txdata_vtbl *txdvtbl);
inline int fio_rbf_empty(struct fio_ringbuf *bf);
inline int _fio_rbf_empty(struct fio_txdata *_head, struct fio_txdata *_tail);
inline int fio_rbf_full(struct fio_ringbuf *bf);
inline int fio_rbf_reserve_one(struct fio_ringbuf *bf, struct fio_txdata **m);
inline int fio_rbf_occupy(struct fio_ringbuf *bf);
inline int fio_rbf_size(struct fio_ringbuf *bf);
inline int fio_rbf_avail(struct fio_ringbuf *bf);
inline int fio_rbf_reserve_all(struct fio_ringbuf *bf, struct fio_txdata **m);
inline int fio_rbf_expand(struct fio_ringbuf *bf, int tid);
inline int fio_rbf_commit(struct fio_ringbuf *bf, int len);
inline struct fio_txdata * fio_rbf_seek(struct fio_txdata **_head, struct fio_txdata *_end,
        struct fio_txdata *_txbufs, int num);
inline struct fio_txdata * fio_rbf_next(struct fio_txdata *m);
inline int fio_rbf_consume(struct fio_ringbuf *bf, int num);
inline int _fio_rbf_consume(struct fio_ringbuf *bf, struct fio_txdata *_head, int num);
void fio_rbf_destroy(struct fio_ringbuf *bf);
#endif

