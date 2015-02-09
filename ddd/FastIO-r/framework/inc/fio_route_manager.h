//	$Id: fio_route_manager.h 2013-05-23 likunxiang$
//
#ifndef __FIO_ROUTE_MANAGER_H__
#define __FIO_ROUTE_MANAGER_H__

#include "fio_nm_util.h"

struct fio_nic;
struct fio_map_t;

struct fio_route_data
{
    struct in_addr dest;  
    struct in_addr mask;  
    struct in_addr gateway;  
    struct in_addr prefix;  
    char iface[FIO_MAX_NAME_LEN];  
    int nic_idx;  
    struct fio_map_t *srcips;
};
struct fio_route_mask
{
    struct fio_list_head link;
    struct fio_list_head sub;
    uint32_t begin;
    uint32_t cnt;
    char flag;
#define fio_route_off ((char)0)
#define fio_route_on ((char)1)
    struct fio_route_data rd;
};
struct fio_route_context
{
    struct fio_list_head route_mask_list;
    struct fio_route_mask **arr_route_idx;
    int route_mask_cnt;
    int route_cnt_before_optimize;
};

int fio_route_resolve_defgw_mac(struct fio_nic *nic, struct ether_addr *ptr_mac, char *str_ifname);
inline uint32_t fio_route_defgw();
inline uint32_t fio_route_defgw_stat();
inline struct ether_addr *fio_route_defgw_mac();
inline int32_t fio_route_update_defgw_mac(uint32_t hip, struct ether_addr *mac);
void fio_route_context_init(struct fio_route_context *rc);
void fio_route_defroute_init(struct fio_route_mask *def_route);
void fio_route_defroute_clean(struct fio_route_mask *def_route);
void fio_route_context_clean(struct fio_route_context *rc);
int fio_route_context_update(struct fio_route_context *rc, struct fio_nic *nics, struct fio_route_mask *defroute);
int fio_route_read();
inline int fio_route_find(struct fio_nic *const ptr_nic, const uint32_t dip, uint32_t *const ptr_srcip,
        int *const ptr_dev, uint32_t *const ptr_nip);
int fio_route_context_cmp(struct fio_route_context *rc, struct fio_route_mask *defrc);
int32_t fio_route_tbl_init();
void fio_route_tbl_destroy();

#endif
