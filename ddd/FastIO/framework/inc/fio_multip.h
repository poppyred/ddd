#ifndef __FIO_MULTIP_H__
#define __FIO_MULTIP_H__

#include <netinet/in.h>
#include <net/if.h>
#include "fio_list.h"

#define FIO_MAX_MAC_NIC             256
#define FIO_MAX_IP_NIC              20
#define FIO_MAX_MASK_NIC            FIO_MAX_IP_NIC
#define FIO_MAX_NAME_LEN        IFNAMSIZ
#define FIO_MAX_NICS            10

struct fio_map_t;
struct fio_mapstr_t;

//子网拥有IP
struct fio_mp_mask
{
    struct fio_list_head link;
    uint32_t begin;
    uint32_t cnt;
    char flag;
#define fio_mp_mask_off ((char)0)
#define fio_mp_mask_on ((char)1)
    struct fio_map_t *my_ips;
    struct fio_map_t *buddy_ips;
};

struct fio_mp_ipinfo
{
    char nic_name[FIO_MAX_NAME_LEN];
    struct fio_map_t *if_localaddrs;
    struct fio_mapstr_t *map_arpa;
    struct fio_list_head mask_sort;
    struct fio_mp_mask **arr_mask_idx;
    int mask_cnt;
};

int fio_mp_fetchips();
uint16_t fio_mp_iphash(uint32_t u_ip);
int fio_mp_init(char nic_name[][FIO_MAX_NAME_LEN], int nic_cnt);
inline int fio_mp_update_ctiphashtbl(int8_t *ip_hashtbl);
int fio_mp_update_nic(struct fio_mp_ipinfo *mp_ipinfo, struct fio_map_t *buddy_macs);
struct fio_mp_mask * fio_mp_mask_finddomain(struct fio_mp_mask **const arr_mask_idx, int mask_cnt, uint32_t myip);
int fio_mp_mask_add_domainip(struct fio_mp_mask **const arr_mask_idx, int mask_cnt, uint32_t ip);
int fio_mp_nic_has_localip(struct fio_mp_ipinfo *ipinfo, uint32_t ip);
void fio_mp_destroy();
void fio_mp_nic_destroy(struct fio_mp_ipinfo *mp_ipinfo);

#endif

