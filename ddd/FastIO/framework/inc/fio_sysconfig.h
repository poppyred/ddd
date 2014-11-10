//	$Id: fio_sysconfig.h 2013-05-23 likunxiang$
//

#ifndef _FIO_GLOBAL_H_
#define _FIO_GLOBAL_H_

#include "fio_nm_util.h"
#include "proto_map.h"
#include "fio_log.h"

#define TXBUF_SIZE (MAX_PKT_COPY_ONCE*4)
//#define TXBUF_SIZE_SINGLE_MODE (MAX_PKT_COPY_ONCE*24)

#define MAX_CALLBACK_FUN MAX_PROTO

#define FIO_SPECTSK_COPYRX 0x1
#define FIO_SPECTSK_SNDMAC 0x2
#ifdef use_old_drive
#define FIO_SPECTSK_POLLOUT 0x4
#endif
#define FIO_SPECTSK_UPDATEMP 0x8
#define FIO_SPECTSK_CLEARMAC 0x10

#define FIO_MAX_DST_TEST 10

struct port_range
{
    uint16_t pr_st;
    uint16_t pr_ed;
};

struct sysconfig {
    int working;
    int needcp;
    int single_thread;
    struct port_range *interested_ports_range[FIO_MAX_NAME_LEN];
    char str_interested_port[FIO_MAX_NAME_LEN];
    uint8_t interested_protos[FIO_MAX_NAME_LEN];
    char str_interested_proto[FIO_MAX_NAME_LEN];
    uint8_t def_protos[FIO_MAX_NAME_LEN];
    char str_def_proto[FIO_MAX_NAME_LEN];
    char if_in[FIO_MAX_NAME_LEN];
    char if_out[FIO_MAX_NAME_LEN];
#ifdef __old_mac_mgr__
    char vlans[FIO_MAX_NICS][FIO_MAX_NAME_LEN];
#endif
    char nic_names[FIO_MAX_NICS][FIO_MAX_NAME_LEN];
    int num_nic;
    struct ether_addr if_macs[FIO_MAX_NICS];
    struct in_addr if_ips[FIO_MAX_NICS];
    uint64_t mac_lifetime;
    uint64_t mac_buddy_lifetime;
    //uint64_t multip_checktime;
    int route_chk_interval, mac_try;
    int burst;
    int nthreads;
    int promisc, prot_defgw_mac, enable_slog, enable_sysstack;
    
    int is_test, dst_count;
    struct ip_range src_ip;
    struct ip_range dst_ip[FIO_MAX_DST_TEST];
    struct mac_range src_mac, defgw_mac;
    struct mac_range dst_mac[FIO_MAX_DST_TEST];
    uint16_t src_port;
    uint16_t dst_port;
    int pkt_size;
    int snd_pkts_per_sec;
    int prdebug, prsys_stack;
    int prmac_info;
    char str_logpath[MAX_FIO_PATH];
    char str_logfname[MAX_FIO_PATH];
    char str_spec_ip[MAX_FIO_PATH];
    struct fio_log maclog;
    int buf_num_;
    char log_folder_[MAX_FIO_PATH+1];
    char error_path_[MAX_FIO_PATH+1];
    char trace_path_[MAX_FIO_PATH+1];
    int daemon_mode_;
};
extern struct sysconfig sysconfig;

#endif
