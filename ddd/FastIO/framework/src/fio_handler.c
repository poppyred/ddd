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
#include "log_log.h"

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

void fio_handle_ctmsg(struct fio_context *mycontext, struct fio_notify_msg *msg)
{
    if (sysconfig.prsys_stack)
        OD( "tid %d read msg_id = %u, nic_id = %d, abstract = %p", mycontext->me,
            msg->msg_id, msg->nic_id, (void*)msg->abstract);
    switch (msg->msg_id)
    {
        case FIO_MSG_SYSSTACK:
            fio_nic_send_sys_rxbuf(&mycontext->nics[msg->nic_id]);
            break;
        case FIO_MSG_UPSNDBUF:
            fio_nic_send_upsndbuf(&mycontext->nics[msg->nic_id]);
            break;
        case FIO_MSG_UPMAC:
            {
                struct fio_nic *from = &mycontext->nics[msg->nic_id];
                struct fio_mac_info *ptr_mac_info = (struct fio_mac_info*)msg->abstract;
                if (0 == fio_mp_mask_add_domainip(from->mp_ipinfo.arr_mask_idx, 
                            from->mp_ipinfo.mask_cnt, ptr_mac_info->ip))
                {
                    struct fio_mac_info *cur = NULL;
                    fio_mac_set_buddy_mac(from, ptr_mac_info->ip, &cur, &ptr_mac_info->mac);
                    fio_route_update_defgw_mac(ptr_mac_info->ip, &ptr_mac_info->mac);
                }
                free(ptr_mac_info);
            }
            break;
        case FIO_MSG_DEL_BUDDYIP:
            {
                struct fio_nic *nic = &mycontext->nics[msg->nic_id];
                struct fio_mp_ipinfo *mp_ipinfo = &nic->mp_ipinfo;
                struct fio_mp_mask *mp_mask;
                struct in_addr sin_dip_debug;
                char str_dip_debug[16];
                sin_dip_debug.s_addr = htonl((uint32_t)msg->abstract);
                inet_ntop(AF_INET, &sin_dip_debug, str_dip_debug, 16);
                if ((mp_mask=fio_mp_mask_finddomain(mp_ipinfo->arr_mask_idx, mp_ipinfo->mask_cnt,
                                (uint32_t)msg->abstract)) && mp_mask->buddy_ips)
                {
                    if (sysconfig.prsys_stack)
                        OD( "tid %d del buddy ip %s nic_id = %d", mycontext->me, str_dip_debug, msg->nic_id);
                    fio_map_del(mp_mask->buddy_ips, msg->abstract);
                }
                if (sysconfig.prsys_stack)
                    OD( "tid %d del buddy ip %s's mac nic_id = %d", mycontext->me, str_dip_debug, msg->nic_id);
                fio_mac_del_buddy_mac(nic, (uint32_t)msg->abstract);
            }
            break;
        case FIO_MSG_CLEAR_MACPOOL:
            {
                struct fio_nic *nic = NULL;
                int nic_num = mycontext->num_nic, i, notfound = 1;
                struct fio_mp_ipinfo *mp_ipinfo;
                struct fio_mp_mask *mp_mask;
                struct fio_node_t *nd;
                uint32_t buddyip;
                struct in_addr sin_dip_debug;
                char str_dip_debug[16];
                for (nd = fio_map_first(mycontext->buddy_macs); nd;)
                {
                    notfound = 1;
                    buddyip = (uint32_t)nd->key;
                    for (i = 0; i < nic_num; i++)
                    {
                        nic = &mycontext->nics[i];
                        mp_ipinfo = &nic->mp_ipinfo;
                        if ((mp_mask=fio_mp_mask_finddomain(mp_ipinfo->arr_mask_idx, mp_ipinfo->mask_cnt, buddyip)) 
                                && mp_mask->buddy_ips
                                && fio_map_exist(mp_mask->buddy_ips, buddyip))
                        {
                            notfound = 0;
                            break;
                        }
                    }
                    if (notfound)
                    {
                        sin_dip_debug.s_addr = htonl(buddyip);
                        inet_ntop(AF_INET, &sin_dip_debug, str_dip_debug, 16);
                        if (sysconfig.prsys_stack)
                            OD( "tid %d del buddy ip %s's mac", mycontext->me, str_dip_debug);
                        nd = fio_map_next(mycontext->buddy_macs, nd);
                        fio_mac_del_buddy_mac(&mycontext->nics[0], buddyip);
                        continue;
                    }
                    nd = fio_map_next(mycontext->buddy_macs, nd);
                }
            }
            break;
        default:
            break;
    }
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

