#ifndef __FIO_SYSSTACK_H__
#define __FIO_SYSSTACK_H__

struct fio_nic; 
struct fio_ringbuf;
struct fio_txdata;
struct fio_rxdata;
struct fio_notify_msg;
struct fio_context;

int fio_stack_deliver(struct fio_nic *nic, struct fio_rxdata *rxd);
int fio_stack_reserve_tx(struct fio_ringbuf *rbf, struct fio_txdata **m);
int fio_stack_start();
void fio_stack_init_rxbuf(struct fio_ringbuf *rbf, struct fio_txdata *txb, int tid, int _id);
void fio_stack_init_txbuf(struct fio_ringbuf *rbf, struct fio_txdata *txb, int tid, int _id);
int fio_stack_send_nic(struct fio_nic *sys_nic, struct fio_nic *nic, int limit, void *param);
inline void fio_stack_notify_systhread(struct fio_nic *nic, struct fio_notify_msg *msg);
inline void fio_stack_notify_workthread(struct fio_context *syscontext, int thread_id, struct fio_notify_msg *msg);
inline int fio_stack_send_up(struct fio_nic *nic, char * const pbuf, int pkt_size);

#endif

