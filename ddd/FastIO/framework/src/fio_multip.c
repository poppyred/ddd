//	$Id: fio_multip.c 2013-05-23 likunxiang$
//

//#define _test_multip_
#ifndef _test_multip_

#include "fio_multip.h"
#include "fio_sysconfig.h"
#include "fio_timer.h"
#include "fio_context.h"
#include "fio_map32.h"
#include "log_log.h"
#include "fio_mapstr.h"
#include "fio_route_manager.h"

#define FIO_FORMAT_ARPA "%s.in-addr.arpa"
struct fio_mp_ifa
{
  struct in_addr ifa_addr;	/* Network address of this interface.  */
  struct in_addr ifa_prefix; /* net work prefix of this interface.  */
  struct in_addr ifa_mask; /* netmask of this interface.  */
};
struct fio_mp_nic_aip
{
    struct fio_list_head link;
    struct fio_mp_ifa ifa;
};
struct fio_mp_nicinfo
{
    char nic_name[FIO_MAX_NAME_LEN];
    struct fio_list_head ip_list;
    int ip_cnt;
};
struct fio_mp_mgr
{
    int8_t g_fio_mp_hashtbl[0xffff];
    struct fio_mp_nicinfo mp_nicinfos[FIO_MAX_NICS];
    int nic_cnt;
    uint64_t interval;
    uint64_t elapse;
};
struct fio_mp_mgr g_mp_mgr;
int fio_mp_nic_has_localip(struct fio_mp_ipinfo *ipinfo, uint32_t ip)
{
    if (!fio_map_exist(ipinfo->if_localaddrs, (unsigned long)ip))
        return 0;
    return 1;
}
int fio_mp_nic_cmp_addrs(struct fio_mp_nicinfo *nicinfo, struct fio_mp_ipinfo *ipinfo)
{
    if ((int)fio_map_count(ipinfo->if_localaddrs) != nicinfo->ip_cnt)
        return -1;
    struct fio_mp_nic_aip *_aip;
    struct fio_list_head *pos;
    fio_list_for_each_safe(pos, &nicinfo->ip_list) 
    {
        _aip = fio_list_entry(pos, struct fio_mp_nic_aip, link);
        if (!fio_map_exist(ipinfo->if_localaddrs, (unsigned long)_aip->ifa.ifa_addr.s_addr))
            return -1;
    }
    return 0;
}

int fio_mp_timeout(uint64_t interval, struct fio_mp_mgr *mgr, uint64_t elapse) 
{
    //FLOG2_TRACE(g_log2, "mgr->elapse %"LU64" elapse %"LU64" interval %"LU64, mgr->elapse, elapse, interval);
    mgr->elapse += elapse; 
    if (mgr->elapse < interval)
        return 0; 
    mgr->elapse -= interval; 
    return 1;
}

static void fio_chk_mp(uint64_t elapse)
{
    if (!fio_mp_timeout(g_mp_mgr.interval, &g_mp_mgr, elapse))
        return;

    int i, j;
    fio_mp_fetchips();
    fio_route_read();
    
    for (j = 0; j < sysconfig.nthreads; j++)
    {
        struct fio_context *ct = &g_contexts[j];
        if (fio_route_context_cmp(&ct->route_context, &ct->defroute_context))
        {
            OD( "tid %d need to update mp cause route change", ct->me);
            ct->nics[0].spec_tasks |= FIO_SPECTSK_UPDATEMP;
            continue;
        }
        for (i = 0; i < g_mp_mgr.nic_cnt; i++)
            if ( fio_mp_nic_cmp_addrs(&g_mp_mgr.mp_nicinfos[i], &ct->nics[i].mp_ipinfo) )
            {
                OD( "tid %d need to update mp cause mp change", ct->me);
                ct->nics[0].spec_tasks |= FIO_SPECTSK_UPDATEMP;
                break;
        }
    }
}

int fio_mp_init(char nic_names[][FIO_MAX_NAME_LEN], int nic_cnt)
{
    int i;
    static int inited = 0;
    if (inited)
        return 0;

    inited = 1;
    g_mp_mgr.interval = sysconfig.route_chk_interval*1000000;
    g_mp_mgr.nic_cnt = nic_cnt;
    for (i = 0; i < FIO_MAX_NICS && i < nic_cnt; i++)
    {
        strncpy(g_mp_mgr.mp_nicinfos[i].nic_name, nic_names[i], FIO_MAX_NAME_LEN);
        FIO_INIT_LIST_HEAD(&g_mp_mgr.mp_nicinfos[i].ip_list);
        g_mp_mgr.mp_nicinfos[i].ip_cnt = 0;
    }
    fio_mp_fetchips();
    fio_register_timer_handler(FIO_T_CHECK_MULTIP, fio_chk_mp);
    return 0;
}

uint16_t fio_mp_iphash(uint32_t u_ip)
{ 
    uint32_t big = ((u_ip&0xffff0000)>>16);
    return (uint16_t)(big&(u_ip&0xffff));  
}  

inline int fio_mp_update_ctiphashtbl(int8_t *ip_hashtbl)
{
    memcpy(ip_hashtbl, g_mp_mgr.g_fio_mp_hashtbl, 0xffff);
    return 0;
}

void fio_mp_mask_sort_del(struct fio_mp_mask *n)
{
	if (!n)
		return;
    fio_list_del(&n->link);
    if (n->my_ips)
        fio_map_destroy(n->my_ips);
    if (n->buddy_ips)
        fio_map_destroy(n->buddy_ips);
    free(n);
}

bool fio_mp_mask_sort_add(struct fio_list_head *_head, struct fio_mp_mask *_mask)
{
    struct fio_mp_mask *node = malloc(sizeof(struct fio_mp_mask));
    if( node ) 
    {
        memset(node, 0, sizeof(struct fio_mp_mask)); 
        node->begin = _mask->begin;
        node->cnt = _mask->cnt;
        node->buddy_ips = _mask->buddy_ips;
        node->my_ips = _mask->my_ips;
        node->flag = _mask->flag;
        fio_list_add(&node->link, _head);
    }
    else
        return false;
    return true;
}

inline bool fio_mp_mask_sort_add_tail(struct fio_list_head *_head, struct fio_mp_mask *_mask)
{
    return fio_mp_mask_sort_add(_head->prev, _mask);
}

void fio_mp_mask_sort_clean(struct fio_list_head *_head)
{
    struct fio_mp_mask *node = NULL;
    struct fio_list_head *pos;
    while(!fio_list_empty(_head))
    {
        pos = _head->next;
        node = fio_list_entry(pos, struct fio_mp_mask, link);
        fio_mp_mask_sort_del(node);
    }
    FIO_INIT_LIST_HEAD(_head);
}

inline void fio_mp_mask_cpy(struct fio_mp_mask *src, struct fio_mp_mask *dst)
{
    dst->begin = src->begin;
    dst->cnt = src->cnt;
    dst->flag = src->flag;
    dst->my_ips = src->my_ips;
    dst->buddy_ips = src->buddy_ips;
}
void fio_mp_mask_makeup(struct fio_list_head *root, struct fio_mp_mask *m, struct fio_mp_mask *n)
{
    struct fio_mp_mask input_mask;
    input_mask.buddy_ips = NULL;
    input_mask.my_ips = NULL;
    if (m->begin + m->cnt < n->begin)
    {
        input_mask.begin = m->begin + m->cnt;
        input_mask.cnt = n->begin - (m->begin + m->cnt);
        input_mask.flag = fio_mp_mask_off;
        fio_mp_mask_sort_add(&m->link, &input_mask); 
        return;
    }
    if (m->begin + m->cnt > n->begin)
    {
        if (m->cnt == n->cnt)
        {
            fio_mp_mask_sort_del(n); 
            return;
        }
        if (m->begin != n->begin)
        {
            input_mask.flag = m->flag;
            input_mask.begin = n->begin;
            input_mask.cnt = m->cnt - (n->begin - m->begin);
            m->cnt = n->begin - m->begin;
            fio_mp_mask_sort_add(&m->link, &input_mask); 
        }
        else
        {
            input_mask.flag = m->flag;
            input_mask.begin = n->begin + n->cnt;
            input_mask.cnt = m->cnt - n->cnt;
            fio_mp_mask_cpy(n, m);
            fio_mp_mask_sort_del(n); 
            struct fio_list_head *pos = m->link.next;
            while (pos != root)
            {
                n = fio_list_entry(pos, struct fio_mp_mask, link);
                if (input_mask.begin + input_mask.cnt <= n->begin)
                {
                    fio_mp_mask_sort_add_tail(pos, &input_mask); 
                    break;
                }
                if (input_mask.begin == n->begin && input_mask.cnt == n->cnt)
                    break;
                if (input_mask.begin <= n->begin)
                {
                    fio_mp_mask_sort_add_tail(pos, &input_mask); 
                    break;
                }
                pos = pos->next;
            }
            if (pos == root)
                fio_mp_mask_sort_add_tail(root, &input_mask); 
        }
    }
}

void fio_mp_nic_iplist_clean(struct fio_mp_nicinfo *mp_nicinfo)
{
    struct fio_mp_nic_aip *aip = NULL;
    struct fio_list_head *pos, *_head = &mp_nicinfo->ip_list;
    while(!fio_list_empty(_head))
    {
        pos = _head->next;
        aip = fio_list_entry(pos, struct fio_mp_nic_aip, link);
        fio_list_del(&aip->link);
        free(aip);
    }
    FIO_INIT_LIST_HEAD(_head);
    mp_nicinfo->ip_cnt = 0;
}
bool fio_mp_nic_iplist_add(struct fio_mp_nicinfo *mp_nicinfo, struct fio_mp_nic_aip *_aip)
{
    struct fio_mp_nic_aip *node = malloc(sizeof(struct fio_mp_nic_aip));
    if( node ) 
    {
        memset(node, 0, sizeof(struct fio_mp_nic_aip)); 
        node->ifa = _aip->ifa;
        fio_list_add(&node->link, &mp_nicinfo->ip_list);
        mp_nicinfo->ip_cnt++;
    }
    else
    {
        OD( "error malloc fio_mp_nic_aip!!!");
        return false;
    }
    return true;
}
struct fio_mp_mask * fio_mp_mask_finddomain(struct fio_mp_mask **const arr_mask_idx, int mask_cnt, uint32_t myip)
{
    if (mask_cnt < 1)
        return NULL;
    int mid, start = 0, end = mask_cnt - 1;
    while (start <= end) {
        mid = (start + end) / 2;
        if (arr_mask_idx[mid]->begin+arr_mask_idx[mid]->cnt-1 < myip)
            start = mid + 1;
        else if (arr_mask_idx[mid]->begin > myip)
            end = mid - 1;
        else
        {
            return arr_mask_idx[mid];
        }
    }
    return NULL;
}
int fio_mp_mask_add_domainip(struct fio_mp_mask **const arr_mask_idx, int mask_cnt, uint32_t ip)
{
    struct fio_mp_mask *domain;
    if (!(domain=fio_mp_mask_finddomain(arr_mask_idx, mask_cnt, ip)) || domain->flag != fio_mp_mask_on)
        return -1;
    char cval = 1;
    if (!domain->buddy_ips)
    {
        domain->buddy_ips = fio_map_create(sizeof(char));
        fio_map_init(domain->buddy_ips, FIO_MAX_MAC_NIC, FIO_MAX_MAC_NIC, "map_otherips_nic");
    }
    fio_map_set_cpy(domain->buddy_ips, ip, &cval);
    return 0;
}
void fio_mp_mask_fill_myips(struct fio_mp_ipinfo *mp_ipinfo)
{
    struct fio_node_t *nd;
    char ins_val = 1;
    uint32_t ip;
    struct fio_mp_mask *mp_mask;
    for (nd = fio_map_first(mp_ipinfo->if_localaddrs); nd; nd = fio_map_next(mp_ipinfo->if_localaddrs, nd))
    {
        ip = (uint32_t)nd->key;
        if (!(mp_mask=fio_mp_mask_finddomain(mp_ipinfo->arr_mask_idx, mp_ipinfo->mask_cnt, ip)) 
                || mp_mask->flag != fio_mp_mask_on)
            continue;
        if (!mp_mask->my_ips)
        {
            mp_mask->my_ips = fio_map_create(sizeof(char));
            fio_map_init(mp_mask->my_ips, FIO_MAX_MAC_NIC, FIO_MAX_MAC_NIC, "map_myips_nic");
        }
        fio_map_set_cpy(mp_mask->my_ips, ip, &ins_val);
    }
}
void fio_mp_mask_fill_domainip(struct fio_mp_mask **const arr_mask_idx, int mask_cnt, struct fio_map_t *buddy_macs)
{
    struct fio_node_t *nd;
    uint32_t ip;
    struct fio_mp_mask *mp_mask;
    char cval = 1;
    for (nd = fio_map_first(buddy_macs); nd; nd = fio_map_next(buddy_macs, nd))
    {
        ip = (uint32_t)nd->key;
        if (!(mp_mask=fio_mp_mask_finddomain(arr_mask_idx, mask_cnt, ip)) || mp_mask->flag != fio_mp_mask_on)
            continue;
        if (!mp_mask->buddy_ips)
        {
            mp_mask->buddy_ips = fio_map_create(sizeof(char));
            fio_map_init(mp_mask->buddy_ips, FIO_MAX_MAC_NIC, FIO_MAX_MAC_NIC, "map_otherips_nic");
        }
        fio_map_set_cpy(mp_mask->buddy_ips, ip, &cval);
    }
}

int fio_mp_update_nic(struct fio_mp_ipinfo *mp_ipinfo, struct fio_map_t *buddy_macs)  
{
    int j, i;
    for (j = 0; j < g_mp_mgr.nic_cnt; j++)
    {
        struct fio_mp_nicinfo *mp_nicinfo = &g_mp_mgr.mp_nicinfos[j];
        if (!strncmp(mp_ipinfo->nic_name, mp_nicinfo->nic_name, FIO_MAX_NAME_LEN))
        {
            struct fio_mp_mask input_mask;// = {.begin = 0, .cnt = ~0U + 1;};
            struct fio_list_head *pos, *mask_pos;
            struct fio_mp_nic_aip *_aip;
            char str_aip[16];
            char str_arpa[MAX_NAME];
            fio_mp_mask_sort_clean(&mp_ipinfo->mask_sort);
            mp_ipinfo->mask_cnt = 0;
            free(mp_ipinfo->arr_mask_idx);
            mp_ipinfo->arr_mask_idx = NULL;
            fio_map_clean(mp_ipinfo->if_localaddrs);
            fio_mapstr_clean(mp_ipinfo->map_arpa);
            fio_list_for_each(pos, &mp_nicinfo->ip_list) 
            {
                struct fio_mp_mask *mp_mask = NULL;
                uint32_t prefix, if_addr;
                char ins_val = 1;
                _aip = fio_list_entry(pos, struct fio_mp_nic_aip, link);
                prefix = _aip->ifa.ifa_prefix.s_addr;
                if_addr = _aip->ifa.ifa_addr.s_addr;
                fio_map_set_cpy(mp_ipinfo->if_localaddrs, (unsigned long)if_addr, &ins_val);
                if (inet_ntop(AF_INET, &_aip->ifa.ifa_addr, str_aip, 16))
                {
                    snprintf(str_arpa, MAX_NAME, FIO_FORMAT_ARPA, str_aip);
                    fio_mapstr_set_cpy(mp_ipinfo->map_arpa, str_arpa, &ins_val);
                }
                input_mask.begin = prefix;
                input_mask.cnt = ~0U - _aip->ifa.ifa_mask.s_addr + 1;
                input_mask.buddy_ips = NULL;
                input_mask.my_ips = NULL;
                input_mask.flag = fio_mp_mask_on;
                fio_list_for_each(mask_pos, &mp_ipinfo->mask_sort) 
                {
                    mp_mask = fio_list_entry(mask_pos, struct fio_mp_mask, link);
                    if(mp_mask->begin > input_mask.begin) 
                    {
                        fio_mp_mask_sort_add_tail(mask_pos, &input_mask); 
                        break;
                    }
                    else if (mp_mask->begin == input_mask.begin)
                    {
                        if (mp_mask->cnt > input_mask.cnt) 
                            fio_mp_mask_sort_add(mask_pos, &input_mask); 
                        else if (mp_mask->cnt < input_mask.cnt) 
                            fio_mp_mask_sort_add_tail(mask_pos, &input_mask); 
                        break;
                    }
                } 
                if (mask_pos == &mp_ipinfo->mask_sort)
                    fio_mp_mask_sort_add_tail(mask_pos, &input_mask); 
            }
            fio_list_for_each(mask_pos, &mp_ipinfo->mask_sort) 
            {
                if (mask_pos->next == &mp_ipinfo->mask_sort) //end
                    break;
                fio_mp_mask_makeup(&mp_ipinfo->mask_sort, fio_list_entry(mask_pos, struct fio_mp_mask, link),
                        fio_list_entry(mask_pos->next, struct fio_mp_mask, link));
            }
            mp_ipinfo->mask_cnt = fio_list_count(&mp_ipinfo->mask_sort);
            mp_ipinfo->arr_mask_idx = calloc(mp_ipinfo->mask_cnt+1, sizeof(struct fio_mp_mask*));
            i = 0;
            fio_list_for_each(mask_pos, &mp_ipinfo->mask_sort) 
            {
                mp_ipinfo->arr_mask_idx[i] = fio_list_entry(mask_pos, struct fio_mp_mask, link);
                i++;
            }
            fio_mp_mask_fill_domainip(mp_ipinfo->arr_mask_idx, mp_ipinfo->mask_cnt, buddy_macs);
            fio_mp_mask_fill_myips(mp_ipinfo);
            break;
        }
    }
    return 0;
}

int fio_mp_fetchips()  
{
    int j;
    struct ifaddrs *ifa = NULL, *ifList;  

    if (getifaddrs(&ifList) < 0)
    {
        OD( "can't getifaddrs!");  
        return -1;  
    }

    memset(g_mp_mgr.g_fio_mp_hashtbl, 0, 0xffff);
    for (j = 0; j < g_mp_mgr.nic_cnt; j++)
        fio_mp_nic_iplist_clean(&g_mp_mgr.mp_nicinfos[j]);

    for (ifa = ifList; ifa != NULL; ifa = ifa->ifa_next)  
    {  
        if(ifa->ifa_addr->sa_family != AF_INET)  
            continue;
        else
        {  
            for (j = 0; j < g_mp_mgr.nic_cnt; j++)
            {
                struct fio_mp_nicinfo *mp_nicinfo = &g_mp_mgr.mp_nicinfos[j];
                if (/*mp_nicinfo->ip_cnt < FIO_MAX_IP_NIC &&*/ !strncmp(ifa->ifa_name, mp_nicinfo->nic_name, FIO_MAX_NAME_LEN))
                {
                    uint32_t addr = ntohl(((struct sockaddr_in*)ifa->ifa_addr)->sin_addr.s_addr), 
                             netmask = ntohl(((struct sockaddr_in*)ifa->ifa_netmask)->sin_addr.s_addr);
                    struct fio_mp_nic_aip _aip;
                    _aip.ifa.ifa_addr.s_addr = addr;
                    _aip.ifa.ifa_prefix.s_addr = (addr & netmask);
                    _aip.ifa.ifa_mask.s_addr = netmask;
                    fio_mp_nic_iplist_add(mp_nicinfo, &_aip);
                    g_mp_mgr.g_fio_mp_hashtbl[fio_mp_iphash(addr)] = 1;

                    OD( "\n>>> interfaceName: %s", ifa->ifa_name);  
                    OD( ">>> ipAddress: %s", inet_ntoa(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr));  
                    OD( ">>> broadcast: %s", inet_ntoa(((struct sockaddr_in *)ifa->ifa_dstaddr)->sin_addr));  
                    OD( ">>> subnetMask: %s", inet_ntoa(((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr));  
                    break;
                }
            }
        }  
    }  

    freeifaddrs(ifList);  
    return 0;  
}

void fio_mp_destroy()
{
    int j;
    for (j = 0; j < g_mp_mgr.nic_cnt; j++)
        fio_mp_nic_iplist_clean(&g_mp_mgr.mp_nicinfos[j]);
}

void fio_mp_nic_destroy(struct fio_mp_ipinfo *mp_ipinfo) 
{
	if (!mp_ipinfo)
		return;
    fio_map_destroy(mp_ipinfo->if_localaddrs);
    fio_mapstr_destroy(mp_ipinfo->map_arpa);
    fio_mp_mask_sort_clean(&mp_ipinfo->mask_sort);
    mp_ipinfo->mask_cnt = 0;
    free(mp_ipinfo->arr_mask_idx);
    mp_ipinfo->arr_mask_idx = NULL;
}

#else

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/sockios.h>

int main()  
{  
    int i=0;  
    int sockfd;  
    struct ifconf ifconf;  
    unsigned char buf[512];  
    struct ifreq *ifreq;  

    //初始化ifconf  
    ifconf.ifc_len = 512;  
    ifconf.ifc_buf = buf;  

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0)  
    {  
        perror("socket" );  
        exit(1);  
    }  
    ioctl(sockfd, SIOCGIFCONF, &ifconf); //获取所有接口信息  

    //接下来一个一个的获取IP地址  
    ifreq = (struct ifreq*)buf;  
    for (i=(ifconf.ifc_len/sizeof (struct ifreq)); i>0; i--)  
    {  
        // if(ifreq->ifr_flags == AF_INET){ //for ipv4  
        printf("name = [%s]\n" , ifreq->ifr_name);  
        printf("local addr = [%s]\t\n" , inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr));
        ifreq++;  
        // }  
    }  

    return 0;  
}  
#else
#include <stdio.h>  
#include <ifaddrs.h>  
#include <arpa/inet.h>  

int getSubnetMask()  
{  
    struct sockaddr_in *sin = NULL;  
    struct ifaddrs *ifa = NULL, *ifList;  

    if (getifaddrs(&ifList) < 0) return -1;  

    for (ifa = ifList; ifa != NULL; ifa = ifa->ifa_next)  
    {  
        if(ifa->ifa_addr->sa_family == AF_INET)  
        {  
            OD( "\n>>> interfaceName: %s", ifa->ifa_name);  

            sin = (struct sockaddr_in *)ifa->ifa_addr;  
            OD( ">>> ipAddress: %s", inet_ntoa(sin->sin_addr));  

            sin = (struct sockaddr_in *)ifa->ifa_dstaddr;  
            OD( ">>> broadcast: %s", inet_ntoa(sin->sin_addr));  

            sin = (struct sockaddr_in *)ifa->ifa_netmask;  
            OD( ">>> subnetMask: %s", inet_ntoa(sin->sin_addr));  
        }  
    }  

    freeifaddrs(ifList);  

    return 0;  
}  

int main(void)  
{  
    getSubnetMask();  

    return 0;  
}  
#endif

#endif


