//	$Id: fio_handler.c 2013-05-23 likunxiang$
//
#include "fio_handler.h"
#include "fio_sysconfig.h"
#include "fio_nic.h"
#include "fio_mac_manager.h"
#include "fio_map32.h"
#include "fio_context.h"
#include "fio_icmp.h"
#include "fio_sysstack.h"
//#include "log_log.h"

struct fio_map_t *g_cbf_tbl;
struct fio_map_t *g_eth_handles;
struct fio_map_t *g_interested_tbl;
struct fio_map_t *g_def_handles;
struct fio_map_t *g_defcbf_tbl;

inline void eth_analyse(struct fio_nic *from, struct fio_nic *to, 
        struct fio_asted_rxs *bufs, int immediately, int pre_realtime, void *param)
{
    eth_handle fun;
    int i, end = bufs->avail;
    struct fio_rxdata *rxd = bufs->rxds;
    NOD("tid %d eth avail %d", NIC_EXTRA_CONTEXT(from)->me, end);
    for (i = 0; i < end; i++, rxd++)
    {
        if (fio_map_find_cpy(g_eth_handles, rxd->eh_type, &fun) && fun)
            fun(from, to, rxd, pre_realtime, param);
    }
    bufs->avail = 0;
}  

inline int process_data(struct fio_nic *from, struct fio_nic *to, struct fio_nic **nics, int num_nic, struct fio_asted_rxs *rxds,
        int pre_realtime, uint32_t needcp)
{
    NOD("rxds %p", rxds);
    int pos, ret = 0;
    fun_proc_many fun;
    fun_proc_def fun_def;

    eth_analyse(from, to, &rxds[T_FIO_PKT_ETH], 0, pre_realtime, NULL);
	NOD("tid %d txavail %d %d", NIC_EXTRA_CONTEXT(from)->me, from->txavl, to->txavl);

    struct fio_asted_rxs *defrxds = &rxds[T_FIO_PKT_DEF];
    struct fio_rxdata *rxdatas = defrxds->rxds;
    for (pos = 0; pos < defrxds->avail; pos++, rxdatas++)
    {
        if (fio_map_find_cpy(g_defcbf_tbl, rxdatas->ip_type, &fun_def) && fun_def)
        {
            fun_def(from, rxdatas, NULL);
        }
    }
	defrxds->avail = 0;

    for (pos = T_FIO_PKT_INTD; pos < T_FIO_PKT_ETH; pos++, rxds++)
    {
        NOD("tid %d rxds %p, intd avail %d ", NIC_EXTRA_CONTEXT(from)->me, rxds, rxds->avail);
        if (rxds->avail > 0 && fio_map_find_cpy(g_cbf_tbl, rxds->cb_type, &fun) && fun)
        {
            fun(from, nics[0], nics[1], rxds->rxds, rxds->avail, NULL);
        }
        rxds->avail = 0;
		NOD("tid %d txavail %d %d", NIC_EXTRA_CONTEXT(from)->me, from->txavl, to->txavl);
    }

    for (; pos < T_FIO_PKT_MAX; pos++, rxds++);
    if (rxds->avail > 0)
    {
        if (sysconfig.enable_sysstack)
        {
            struct fio_notify_msg msg;
            msg.msg_id = FIO_MSG_SYSSTACK;
            msg.nic_id = from->type_id;
            msg.abstract = rxds->avail;
            fio_stack_notify_systhread(from, &msg);
        }
        rxds->avail = 0;
    }
    return ret;
}

int fio_register_handler(int ip_proto_id, fun_proc_many handler)
{
    if (ip_proto_id == IPPROTO_MAX)
        return -1; //full

    fio_map_set_cpy(g_cbf_tbl, ip_proto_id, &handler);
    return 0;
}

void fio_create_cbf_tbl()
{
    g_cbf_tbl = fio_map_create(sizeof(fun_proc_many));
    fio_map_init(g_cbf_tbl, MAX_CALLBACK_FUN+1, MAX_CALLBACK_FUN+1, "g_cbf_tbl");

    eth_handle _eth_handle = fio_mac_handle_arp;
    g_eth_handles = fio_map_create(sizeof(eth_handle));
    fio_map_init(g_eth_handles, MAX_CALLBACK_FUN+1, MAX_CALLBACK_FUN+1, "g_def_hdl_tbl");
    fio_map_set_cpy(g_eth_handles, g_arp_type, &_eth_handle);

    g_interested_tbl = fio_map_create(sizeof(unsigned long));
    fio_map_init(g_interested_tbl, MAX_CALLBACK_FUN+1, MAX_CALLBACK_FUN+1, "g_interested_tbl");

    g_def_handles = fio_map_create(sizeof(unsigned long));
    fio_map_init(g_def_handles, MAX_CALLBACK_FUN+1, MAX_CALLBACK_FUN+1, "g_def_handles");

    fun_proc_def _fun_proc_def = fio_icmp_handle_req;
    g_defcbf_tbl = fio_map_create(sizeof(fun_proc_def));
    fio_map_init(g_defcbf_tbl, MAX_CALLBACK_FUN+1, MAX_CALLBACK_FUN+1, "g_defcbf_tbl");
    fio_map_set_cpy(g_defcbf_tbl, IPPROTO_ICMP, &_fun_proc_def);
}

void fio_destroy_cbf_tbl()
{
    if (g_cbf_tbl)
        fio_map_destroy(g_cbf_tbl);

    if (g_eth_handles)
        fio_map_destroy(g_eth_handles);

    if (g_interested_tbl)
        fio_map_destroy(g_interested_tbl);

    if (g_def_handles)
        fio_map_destroy(g_def_handles);

    if (g_defcbf_tbl)
        fio_map_destroy(g_defcbf_tbl);
}

