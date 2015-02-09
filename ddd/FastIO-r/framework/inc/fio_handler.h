//	$Id: fio_handler.h 2013-05-23 likunxiang$
//
#ifndef __FIO_HANDLER_H__
#define __FIO_HANDLER_H__

#include <inttypes.h>

struct fio_nic;
struct fio_rxdata;
struct fio_asted_rxs;
struct fio_notify_msg;
struct fio_context;
struct fio_map_t;
typedef int (*fun_proc_many) (struct fio_nic*, struct fio_nic*, struct fio_nic*, struct fio_rxdata*, int avail, void *param);
typedef int (*fun_proc_def) (struct fio_nic*, struct fio_rxdata*, void *param);

extern struct fio_map_t *g_cbf_tbl;
extern struct fio_map_t *g_eth_handles;
extern struct fio_map_t *g_interested_tbl;
extern struct fio_map_t *g_def_handles;
extern struct fio_map_t *g_defcbf_tbl;

typedef int (*eth_handle)(struct fio_nic *from, struct fio_nic *to, struct fio_rxdata *rxdata,
        int pre_realtime, void *param);

inline void eth_analyse(struct fio_nic *from, struct fio_nic *to, 
        struct fio_asted_rxs *bufs, int immediately, int pre_realtime, void *param);
inline int process_data(struct fio_nic *from, struct fio_nic *to, struct fio_nic **nics, int num_nic,
        struct fio_asted_rxs *rxds, int pre_realtime, uint32_t needcp);
int fio_register_handler(int ip_proto_id, fun_proc_many handler);
void fio_create_cbf_tbl();
void fio_handle_ctmsg(struct fio_context *mycontext, struct fio_notify_msg *msg);
void fio_destroy_cbf_tbl();

#endif
