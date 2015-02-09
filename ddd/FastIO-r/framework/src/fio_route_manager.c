//	$Id: fio_route_manager.c 2013-05-23 likunxiang$
//

#if (defined _TEST_ROUTE_)
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <arpa/inet.h>

#define ND(format, ...)	do {} while(0)
#define D(format, ...)				\
    fprintf(stderr, "%s [%d] " format "\n", 	\
            __FUNCTION__, __LINE__, ##__VA_ARGS__)

int splite_str(char *str, int str_size, char *buf, int buf_size, 
        char **pp, int pp_size, char delimit)
{
    char **p = NULL;
    char *q = NULL;
    char *q_pre = NULL;
    int ppcount = 0, i;

    buf_size = (str_size > buf_size ? buf_size : str_size);
    memcpy(buf, str, buf_size);

    p = pp;
    q = buf;
    q_pre = q;
    while(isspace(*q) || *q == '\t')
        q++;
    for (ppcount = 0; ppcount < pp_size; )
    { 
        if (*q =='\0')
        {
            *p = q_pre;
            ppcount++;
            break;
        }
        if (*q == delimit)
        {
            *q++ = '\0';
            for (i = 2; q-i > q_pre; i++)
            {
                if (isspace(*(q-i)) || *(q-i) == '\t')
                    *(q-i) = '\0';
                else
                    break;
            }
            while(isspace(*q) || *q == '\t')
                q++;
            *p++ = q_pre;
            q_pre = q;
            ppcount++;
            continue;
        }
        q++;

    }
    return ppcount;
}
#else
#include "fio_route_manager.h"
#include "fio_timer.h"
#include "fio_sysconfig.h"
#include "fio_nic.h"
#include "fio_map32.h"
//#include "log_log.h"
#endif

#define FIO_SYS_ROUTE_PATH "/proc/net/route"  

struct fio_route_link
{
    struct fio_list_head link;
    struct fio_route_data rd;
};  
struct fio_route_info
{
    struct fio_list_head route_list;
    int route_cnt;
};
struct fio_route_mgr
{
    uint64_t elapse;
    uint64_t interval;
    struct ether_addr defgw_mac_cache;
    struct fio_route_info route_info;
    struct fio_route_mask def_route;
};
struct fio_route_mgr g_route_mgr;

#if !(defined _TEST_ROUTE_)

uint32_t fio_route_read_defroute(uint32_t vlan_prefix);

inline struct ether_addr *fio_route_defgw_mac()
{
    return &g_route_mgr.defgw_mac_cache;
}

inline int32_t fio_route_update_defgw_mac(uint32_t hip, struct ether_addr *mac)
{
    if (hip != fio_route_defgw())
        return -1;

    memcpy(fio_route_defgw_mac(), mac, sizeof(struct ether_addr));
    return 0;
}

inline uint32_t fio_route_defgw_stat()
{
    return g_route_mgr.def_route.flag;
}

inline uint32_t fio_route_defgw()
{
    return g_route_mgr.def_route.rd.gateway.s_addr;
    }

int fio_route_resolve_mac(char *ifname, struct in_addr *dst_ip, struct in_addr *src_ip, struct ether_addr *src_mac)
{
    struct arp_packet pkt;
    char str_dst[16], recv_buf[120];
    int sfd, len, rfd, try;
    socklen_t slen;
    struct sockaddr_ll sl, repsa;
    struct pollfd fds[1];

    OD( "fio_mac_gen_pack");
    if (fio_mac_gen_pack(&pkt, src_ip, src_mac, dst_ip, NULL, ARPOP_REQUEST))
        OD( "generate arp packet error");
    sfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_RARP));
    if(-1 == sfd)
    {
		OD( "create socket errors");
        return -1;
    }
    memset(&sl, 0, sizeof(sl));
    sl.sll_family = AF_PACKET;
    //sl.sll_addr = MAC_SOURCE;
    if (!ifname || strlen(ifname) < 1)
        sl.sll_ifindex = IFF_BROADCAST;//非常重要
    else
    {
        sl.sll_halen = ETH_ALEN;
        sl.sll_ifindex = if_nametoindex(ifname);
    }

    len = sendto(sfd, &pkt, sizeof(pkt), 0, (struct sockaddr*)&sl, sizeof(sl));
    if(-1 == len)
    {
        OD( "<ERROR> Broadcast arp request of %s via if %s", inet_ntop(AF_INET, dst_ip, str_dst, 16), ifname);
        close(sfd);
        return -1;
    }

    OD( "Broadcast arp request of %s via if %s", inet_ntop(AF_INET, dst_ip, str_dst, 16), ifname);

    rfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    bzero(recv_buf, sizeof(recv_buf));
    bzero(&repsa, sizeof(repsa));
    slen = sizeof(struct sockaddr_ll);
    memset(fds, 0, sizeof(fds));
    fds[0].fd = rfd;
    fds[0].events = POLLIN;
    try = 0;
    int n;
    do {
        if ((n = poll(fds, 1, 2000)) > 0)
        {
            if((n = recvfrom(rfd, recv_buf, sizeof(struct arp_packet), 0, (struct sockaddr *)&repsa, &slen)) <= 0)
            {
                OD( "Recvfrom error");
                continue;
            }
            if( ntohs(*(uint16_t*)(recv_buf + 20))==ARPOP_REPLY && !memcmp(dst_ip, recv_buf + 28, 4) ) 
            {
                memcpy(fio_route_defgw_mac(), recv_buf+22, sizeof(struct ether_addr));
                OD( " IP : %s", str_dst);
                prmac( (uint8_t*)(fio_route_defgw_mac())); 
                break;
            }
        }
    }while (++try < 20);

    close(rfd);
    close(sfd);

    if (try >= 20)
    {
        OD( "fail to fetch ip %s mac via if %s", str_dst, ifname);
        return -1;
    }

    return 0;
}

int32_t fio_route_tbl_init()
{
    OD( "***********TEST****\n sizeof(struct fio_route_link) = %"SZF"!!", sizeof(struct fio_route_link));
    g_route_mgr.interval = sysconfig.route_chk_interval*1000000;
    //read_defroute(g_route_mgr.vlan_prefix);
    FIO_INIT_LIST_HEAD(&g_route_mgr.route_info.route_list);
    g_route_mgr.route_info.route_cnt = 0;
    fio_route_defroute_init(&g_route_mgr.def_route);
    fio_route_read();
    return 0;
    }
int fio_route_resolve_defgw_mac(struct fio_nic *nic, struct ether_addr *ptr_mac, char *str_ifname)
{
    if (fio_route_defgw_stat() != fio_route_on)
        goto err;
    uint32_t dip = fio_route_defgw(), sip, nip;
    int dev_idx;
    struct fio_nic *ptr_nic = nic;
    if (0 != fio_route_find(ptr_nic, dip, &sip, &dev_idx, &nip))
        goto err;
    if (dev_idx != ptr_nic->type_id)
    {
        OD( "don't resolve from me!!!!!!!!!!!!");
        goto err;
    }
    dip = htonl(dip);
    sip = htonl(sip);
    if (fio_route_resolve_mac(str_ifname?str_ifname:ptr_nic->nmr.nr_name, (struct in_addr*)&dip, 
                (struct in_addr*)&sip, ptr_mac?ptr_mac:&ptr_nic->if_mac))
        {
        if (sysconfig.prot_defgw_mac)
            {
            memcpy(fio_route_defgw_mac(), &sysconfig.defgw_mac.start, sizeof(struct ether_addr));
            OD( "can't find defgw mac, use mac %02x:%02x:%02x:%02x:%02x:%02x",
                    fio_route_defgw_mac()->ether_addr_octet[0], fio_route_defgw_mac()->ether_addr_octet[1], 
                    fio_route_defgw_mac()->ether_addr_octet[2], fio_route_defgw_mac()->ether_addr_octet[3],
                    fio_route_defgw_mac()->ether_addr_octet[4], fio_route_defgw_mac()->ether_addr_octet[5]);

            }
            else
        {
            OD( "can't find default gateway's mac!!!!!!!!!!!!");
            goto err;
        }
        }
    return 0;
err:
    return -1;
}

#endif

int fio_route_nicstr2idx(const char *iface)
{
    if (!iface)
        return -1;
    int i;
    for (i = 0; i < sysconfig.num_nic; i++)
        if (!strncmp(sysconfig.nic_names[i], iface, FIO_MAX_NAME_LEN))
            return i;
    return -1;
}

int fio_route_info_add_tail(struct fio_route_info *route_info, struct fio_route_data *rd)
{
    struct fio_route_link *rl = malloc(sizeof(struct fio_route_link));
    if(rl) 
    {
        memset(rl, 0, sizeof(struct fio_route_link)); 
        rl->rd = *rd;
        fio_list_add_tail(&rl->link, &route_info->route_list);
        route_info->route_cnt++;
    }
    else
    {
        OD( "error malloc fio_route_link!!!");
        return -1;
    }
    return 0;
}
void fio_route_info_clean(struct fio_route_info *route_info)
{
    struct fio_route_link *rl = NULL;
    struct fio_list_head *pos, *_head = &route_info->route_list;
    while(!fio_list_empty(_head))
    {
        pos = _head->next;
        rl = fio_list_entry(pos, struct fio_route_link, link);
        fio_list_del(&rl->link);
        free(rl);
    }
    FIO_INIT_LIST_HEAD(_head);
    route_info->route_cnt = 0;
}
int fio_route_get_entrys()  
{  
    FILE *fp = NULL;  
    fp = fopen (FIO_SYS_ROUTE_PATH, "r");  
    if (!fp) {  
        OD( "can't open file!");  
        return -1;
    }  
    char buf[BUFSIZ] = {'\0'};  
    int firsttime = 1;  
    int firstline = 1;  
    int nouse;  
    struct fio_route_data rd;

    while (!feof (fp) && fgets(buf, BUFSIZ, fp) != NULL) 
    {
        if (firstline) 
        {
            firstline = 0;  
            continue;  
        }  

        sscanf (buf, "%s%x%x%x%x%x%x%x", rd.iface, &rd.dest.s_addr,
                &rd.gateway.s_addr, &nouse, &nouse, &nouse, &nouse,
                &rd.mask.s_addr);  
        rd.dest.s_addr = ntohl(rd.dest.s_addr);
        rd.gateway.s_addr = ntohl(rd.gateway.s_addr);
        rd.mask.s_addr = ntohl(rd.mask.s_addr);
        rd.prefix.s_addr = (rd.dest.s_addr&rd.mask.s_addr);
        rd.nic_idx = fio_route_nicstr2idx(rd.iface);
        rd.srcips = NULL;
        if (firsttime)
            firsttime = 0;  
        if (0 != fio_route_info_add_tail(&g_route_mgr.route_info, &rd))
            return -1;
    }  

    fclose (fp);  
    return 0;  
}  

int fio_route_read()
{  
    char buf1[20] = {'\0'};  
    char buf2[20] = {'\0'};  
    char buf3[20] = {'\0'};  
    fio_route_info_clean(&g_route_mgr.route_info);
    fio_route_defroute_clean(&g_route_mgr.def_route);
    if (0 != fio_route_get_entrys()) {  
        OD( "can not get route table");  
        return -1;
    }  
    OD( "Destination\tGenmask\t\tGateway\t\tUse Iface");  
    struct in_addr gateway, netmask, destion;  
    struct fio_list_head *pos;
    struct fio_route_link *rl, *defrl = NULL;
    fio_list_for_each_safe(pos, &g_route_mgr.route_info.route_list)
    {
        rl = fio_list_entry(pos, struct fio_route_link, link);
        gateway.s_addr = htonl(rl->rd.gateway.s_addr);  
        netmask.s_addr = htonl(rl->rd.mask.s_addr);  
        destion.s_addr = htonl(rl->rd.dest.s_addr);  
        OD( "%s\t%s\t%s\t%s",  
                inet_ntop (AF_INET, &destion, buf1, 20),  
                inet_ntop (AF_INET, &netmask, buf2, 20),  
                inet_ntop (AF_INET, &gateway, buf3, 20),  
                rl->rd.iface);  
        if (destion.s_addr == 0 && netmask.s_addr == 0 && rl->rd.nic_idx >= 0)
        {
            defrl = rl;
        }
    }  
    if (defrl)
    {
        g_route_mgr.def_route.rd = defrl->rd;
        g_route_mgr.def_route.flag = fio_route_on;
        fio_list_del(&defrl->link);
        free(defrl);
        g_route_mgr.route_info.route_cnt--;
        OD( "useful route rule num is %d except default", g_route_mgr.route_info.route_cnt);
        uint32_t ndst = htonl(fio_route_defgw());
        OD( "my defgw is %s", inet_ntop(AF_INET, (struct in_addr*)&ndst,buf3, 16));
    }
    else
    {
        g_route_mgr.def_route.flag = fio_route_off;
        OD( "config default gateway error!!!!!!!!!!!!");
    }

    return 0;  
} 

void fio_route_tbl_destroy()
{
    OD("");
    fio_route_info_clean(&g_route_mgr.route_info);
    fio_route_defroute_clean(&g_route_mgr.def_route);
}

inline void fio_route_mask_cleansub(struct fio_route_mask *src)
{
    struct fio_route_mask *ptr_mask;
    while(!fio_list_empty(&src->sub))
    {
        ptr_mask = fio_list_entry(src->sub.next, struct fio_route_mask, link);
        fio_list_del(&ptr_mask->link);
        free(ptr_mask);
    }
}
inline void fio_route_mask_cpysub(struct fio_route_mask *src, struct fio_route_mask *dst)
{
    struct fio_list_head *pos;
    struct fio_route_mask *ptr_mask, *ptr_src_mask;
    fio_route_mask_cleansub(dst);
    fio_list_for_each(pos, &src->sub) 
    {
        ptr_src_mask = fio_list_entry(pos, struct fio_route_mask, link);
        ptr_mask = malloc(sizeof(struct fio_route_mask));
        memcpy(ptr_mask, ptr_src_mask, sizeof(struct fio_route_mask));
        FIO_INIT_LIST_HEAD(&ptr_mask->sub);
        ptr_mask->flag = dst->flag;
        ptr_mask->begin = dst->begin;
        ptr_mask->cnt = dst->cnt;
        ptr_mask->rd = dst->rd;
        fio_list_add_tail(&ptr_mask->link, &dst->sub);
    }
}
void fio_route_defroute_clean(struct fio_route_mask *def_route)
{
    fio_route_mask_cleansub(def_route);
    def_route->rd.srcips = NULL;
}
void fio_route_defroute_init(struct fio_route_mask *def_route)
{
    memset(def_route, 0, sizeof(struct fio_route_mask));
    FIO_INIT_LIST_HEAD(&def_route->link);
    FIO_INIT_LIST_HEAD(&def_route->sub);
    def_route->flag = fio_route_off;
}
void fio_route_context_init(struct fio_route_context *rc)
{
    FIO_INIT_LIST_HEAD(&rc->route_mask_list);
    rc->route_mask_cnt = 0;
    rc->route_cnt_before_optimize = 0;
    rc->arr_route_idx = NULL;
}
void fio_route_context_del_mask(struct fio_route_mask *m)
{
    fio_route_mask_cleansub(m);
    fio_list_del(&m->link);
    free(m);
}
void fio_route_context_clean(struct fio_route_context *rc)
{
    struct fio_route_mask *route_mask = NULL;
    struct fio_list_head *pos, *_head = &rc->route_mask_list;
    while(!fio_list_empty(_head))
    {
        pos = _head->next;
        route_mask = fio_list_entry(pos, struct fio_route_mask, link);
        fio_route_context_del_mask(route_mask);
    }
    FIO_INIT_LIST_HEAD(_head);
    if (rc->arr_route_idx)
    {
        free(rc->arr_route_idx);
        rc->arr_route_idx = NULL;
    }
    rc->route_mask_cnt = 0;
    rc->route_cnt_before_optimize = 0;
}
int fio_route_context_add(struct fio_list_head *_head, struct fio_route_mask *route_mask)
{
    struct fio_route_mask *rm = malloc(sizeof(struct fio_route_mask));
    if(rm) 
    {
        memcpy(rm, route_mask, sizeof(struct fio_route_mask));
        FIO_INIT_LIST_HEAD(&rm->sub);
        fio_route_mask_cpysub(route_mask, rm);
        fio_list_add(&rm->link, _head);
    }
    else
    {
        OD( "error malloc fio_route_mask!!!");
        return -1;
    }
    return 0;
}
inline int fio_route_context_add_tail(struct fio_list_head *_head, struct fio_route_mask *route_mask)
{
    return fio_route_context_add(_head->prev, route_mask);
}
inline void fio_route_mask_cpy(struct fio_route_mask *src, struct fio_route_mask *dst)
{
    dst->begin = src->begin;
    dst->cnt = src->cnt;
    dst->flag = src->flag;
    dst->rd = src->rd;
    fio_route_mask_cpysub(src, dst);
}
void fio_route_context_makeup(struct fio_list_head *root, struct fio_route_mask *m, struct fio_route_mask *n)
{
    struct fio_route_mask input_mask;
    memset(&input_mask, 0, sizeof(struct fio_route_mask));
    FIO_INIT_LIST_HEAD(&input_mask.sub);
    if (m->begin + m->cnt < n->begin)
    {
        input_mask.begin = m->begin + m->cnt;
        input_mask.cnt = n->begin - (m->begin + m->cnt);
        input_mask.flag = fio_route_off;
        input_mask.rd.srcips = NULL;
        fio_route_context_add(&m->link, &input_mask); 
        goto ret_;
    }
    if (m->begin + m->cnt > n->begin)
    {
        if (m->cnt == n->cnt)
        {
            fio_route_mask_cpy(n, m);
            fio_route_context_del_mask(n); 
            goto ret_;
        }
        if (m->begin == n->begin)
        {
            input_mask.flag = m->flag;
            input_mask.begin = n->begin + n->cnt;
            input_mask.cnt = m->cnt - n->cnt;
            input_mask.rd = m->rd;
            fio_route_mask_cpysub(m, &input_mask);
            fio_route_mask_cpy(n, m);
            fio_route_context_del_mask(n); 
            struct fio_list_head *pos = m->link.next;
            while (pos != root)
            {
                n = fio_list_entry(pos, struct fio_route_mask, link);
                if (input_mask.begin + input_mask.cnt <= n->begin)
                {
                    fio_route_context_add_tail(pos, &input_mask); 
                    break;
                }
                if (input_mask.begin == n->begin && input_mask.cnt == n->cnt)
                    break;
                if (input_mask.begin <= n->begin)
                {
                    fio_route_context_add_tail(pos, &input_mask); 
                    break;
                }
                pos = pos->next;
            }
            if (pos == root)
                fio_route_context_add_tail(root, &input_mask); 
        }
        else
        {
            input_mask.flag = m->flag;
            input_mask.begin = n->begin;
            input_mask.cnt = m->cnt - (n->begin - m->begin);
            input_mask.rd = m->rd;
            fio_route_mask_cpysub(m, &input_mask);
            m->cnt = n->begin - m->begin;
            fio_route_context_add(&m->link, &input_mask); 
        }
    }
ret_:
    fio_route_mask_cleansub(&input_mask);
}
void fio_route_construct_srcips(struct fio_nic *nic, struct fio_route_mask *aroute)
{
    struct fio_mp_mask *mp_mask;
    struct fio_mp_ipinfo *mp_ipinfo = &nic->mp_ipinfo;
    uint32_t key_ip;
    if (aroute->rd.gateway.s_addr == 0)
        key_ip = aroute->rd.prefix.s_addr;
    else
        key_ip = aroute->rd.gateway.s_addr;
    if (!(mp_mask=fio_mp_mask_finddomain(mp_ipinfo->arr_mask_idx, mp_ipinfo->mask_cnt, key_ip))
            || mp_mask->flag != fio_mp_mask_on)
        return;
    aroute->rd.srcips = mp_mask->my_ips;
}
int fio_route_context_update(struct fio_route_context *rc, struct fio_nic *nics, struct fio_route_mask *defroute)
{
    fio_route_context_clean(rc);
    struct fio_route_info *route_info = &g_route_mgr.route_info;
    struct fio_route_link *route_link;
    struct fio_route_mask input_mask, *route_mask;
    struct fio_list_head *pos, *mask_pos;
    fio_list_for_each(pos, &route_info->route_list) 
    {
        route_link = fio_list_entry(pos, struct fio_route_link, link);
        input_mask.begin = route_link->rd.prefix.s_addr;
        input_mask.cnt = ~0U - route_link->rd.mask.s_addr + 1;
        input_mask.flag = fio_route_on;
        input_mask.rd = route_link->rd;
        FIO_INIT_LIST_HEAD(&input_mask.sub);
        fio_list_for_each(mask_pos, &rc->route_mask_list) 
        {
            route_mask = fio_list_entry(mask_pos, struct fio_route_mask, link);
            if(route_mask->begin > input_mask.begin) 
            {
                fio_route_context_add_tail(mask_pos, &input_mask); 
                break;
            }
            else if (route_mask->begin == input_mask.begin)
            {
                if (route_mask->cnt > input_mask.cnt) 
                    fio_route_context_add(mask_pos, &input_mask); 
                else if (route_mask->cnt < input_mask.cnt) 
                    fio_route_context_add_tail(mask_pos, &input_mask); 
                else
                    fio_route_context_add_tail(&route_mask->sub, &input_mask); 
                break;
            }
        } 
        if (mask_pos == &rc->route_mask_list)
            fio_route_context_add_tail(mask_pos, &input_mask); 
    }
    fio_list_for_each(mask_pos, &rc->route_mask_list) 
    {
        if (mask_pos->next == &rc->route_mask_list) //end
            break;
        fio_route_context_makeup(&rc->route_mask_list, fio_list_entry(mask_pos, struct fio_route_mask, link),
                fio_list_entry(mask_pos->next, struct fio_route_mask, link));
    }
    rc->route_cnt_before_optimize = route_info->route_cnt;
    rc->route_mask_cnt = fio_list_count(&rc->route_mask_list);
    rc->arr_route_idx = calloc(rc->route_mask_cnt+1, sizeof(struct fio_route_mask*));
    int i = 0;
    fio_list_for_each(mask_pos, &rc->route_mask_list) 
    {
        rc->arr_route_idx[i] = route_mask =  fio_list_entry(mask_pos, struct fio_route_mask, link);
        if (route_mask->flag == fio_route_on && route_mask->rd.nic_idx >= 0)
            fio_route_construct_srcips(&nics[route_mask->rd.nic_idx], route_mask);
        fio_list_for_each(pos, &rc->arr_route_idx[i]->sub) 
        {
            route_mask = fio_list_entry(pos, struct fio_route_mask, link);
            if (route_mask->flag == fio_route_on && route_mask->rd.nic_idx >= 0)
                fio_route_construct_srcips(&nics[route_mask->rd.nic_idx], route_mask);
        }
        i++;
    }
    fio_route_defroute_clean(defroute);
    fio_route_mask_cpy(&g_route_mgr.def_route, defroute);
    if (defroute->flag == fio_route_on && defroute->rd.nic_idx >= 0)
        fio_route_construct_srcips(&nics[defroute->rd.nic_idx], defroute);
    return 0;
}
inline struct fio_route_mask * fio_route_findmask(struct fio_route_mask **arr_route_idx, int mask_cnt, uint32_t dip)
{
    int mid, start = 0, end = mask_cnt - 1;
    while (start <= end) 
    {
        mid = (start + end) / 2;
        if (arr_route_idx[mid]->begin+arr_route_idx[mid]->cnt-1 < dip)
            start = mid + 1;
        else if (arr_route_idx[mid]->begin > dip)
            end = mid - 1;
        else
        {
            //return ((arr_route_idx[mid]->flag == fio_route_on && arr_route_idx[mid]->rd.nic_idx >= 0)
            return arr_route_idx[mid];
        }
    }
    return NULL;
}
inline int fio_route_find(struct fio_nic *const ptr_nic, const uint32_t dip, uint32_t *const ptr_srcip,
        int *const ptr_dev, uint32_t *const ptr_nip)
{
    struct fio_node_t *ptr_sip_node;
    struct fio_route_context *ptr_route = ptr_nic->route_context;
    struct fio_route_mask *ptr_dstroute =
        fio_route_findmask(ptr_route->arr_route_idx, ptr_route->route_mask_cnt, dip);
    if (!ptr_dstroute || ptr_dstroute->flag != fio_route_on || ptr_dstroute->rd.nic_idx < 0)
        ptr_dstroute = NULL;
    if (!ptr_dstroute && ptr_nic->defroute_context->flag != fio_route_on)
        goto notexist;
    else if (!ptr_dstroute)
        ptr_dstroute = ptr_nic->defroute_context;
    if (!ptr_dstroute->rd.srcips || !(ptr_sip_node=fio_map_iterator_put(ptr_dstroute->rd.srcips)))
        goto notexist;
    *ptr_srcip = (uint32_t)ptr_sip_node->key;
    *ptr_dev = ptr_dstroute->rd.nic_idx;
    *ptr_nip = ((ptr_dstroute->rd.gateway.s_addr == 0) ? dip : ptr_dstroute->rd.gateway.s_addr);
    return 0;
notexist:
    return -1;
}
static inline int fio_route_mask_link_cmp(struct fio_route_mask *m, struct fio_route_link *l)
{
    if (       m->rd.prefix.s_addr != l->rd.prefix.s_addr
            || m->rd.gateway.s_addr != l->rd.gateway.s_addr
            || m->rd.nic_idx != l->rd.nic_idx   )
    {
        struct fio_list_head *pos;
        struct fio_route_mask *ptr_mask = NULL;
        fio_list_for_each(pos, &m->sub) 
        {
            ptr_mask = fio_list_entry(pos, struct fio_route_mask, link); 
            if (       ptr_mask->rd.prefix.s_addr == l->rd.prefix.s_addr
                    && ptr_mask->rd.gateway.s_addr == l->rd.gateway.s_addr
                    && ptr_mask->rd.nic_idx == l->rd.nic_idx   )
                return 0;
        }
        return -1;
    }
    return 0;
}
static inline int fio_route_mask_cmp(struct fio_route_mask *a, struct fio_route_mask *b)
{
    if (       a->flag != b->flag
            || a->rd.prefix.s_addr != b->rd.prefix.s_addr
            || a->rd.gateway.s_addr != b->rd.gateway.s_addr
            || a->rd.nic_idx != b->rd.nic_idx   )
        return -1;
    return 0;
}
int fio_route_context_cmp(struct fio_route_context *rc, struct fio_route_mask *defrc)
{
    //goto unequal; 
    if (fio_route_mask_cmp(defrc, &g_route_mgr.def_route))
        goto unequal;
    struct fio_route_info *myroute_info = &g_route_mgr.route_info;
    if (myroute_info->route_cnt != rc->route_cnt_before_optimize)
        goto unequal;
    struct fio_route_link *ptr_rl;
    struct fio_route_mask *ptr_rm;
    struct fio_list_head *pos;
    fio_list_for_each_safe(pos, &myroute_info->route_list)
    {
        ptr_rl = fio_list_entry(pos, struct fio_route_link, link);
        if (!(ptr_rm=fio_route_findmask(rc->arr_route_idx, rc->route_mask_cnt, ptr_rl->rd.prefix.s_addr))
                || ptr_rm->flag != fio_route_on)
            goto unequal;
        if ( fio_route_mask_link_cmp(ptr_rm, ptr_rl))
            goto unequal;
    }
    return 0;
unequal:
    return -1;
}
uint32_t fio_route_read_defroute(uint32_t vlan_prefix)
{
    static char file_name[] = FIO_SYS_ROUTE_PATH;
    FILE *fp;
    if ((fp = fopen(file_name, "r")) == NULL)
    {
        OD( "fopen() failed");
        return -1;
    }

    char line_buf[1024] = {0}, *p = NULL;
    char temp_arr[1024];
    char *temp_strs[1024];
    int pos_dst = 0, pos_gw = 0, pos_mask = 0, ret_splite, i;
    uint32_t nmask, ndst, ngw, ndefgw = 0, nmygw = 0, myvlan_prefix = 0;
    char temp_addr[16];

    if (fgets(line_buf, sizeof(line_buf), fp) == NULL)
        goto _failed;

    p = line_buf;
    if (*p == '\0')
        goto _failed;

    ret_splite = splite_str(p, strlen(p)+1, temp_arr, 1024, temp_strs, 1024, '\t');
    for (i = 0; i < ret_splite; i++)
    {
        if (!strcasecmp("Destination", temp_strs[i]))
            pos_dst = i;
        else if (!strcasecmp("Mask", temp_strs[i]))
            pos_mask = i;
        else if (!strcasecmp("Gateway", temp_strs[i]))
            pos_gw = i;
    }
    if (pos_gw == pos_dst && pos_dst == pos_mask)
        goto _failed;

    while (fgets(line_buf, sizeof(line_buf), fp) != NULL)
    {
        p = line_buf;
        if (*p == '\0')
            continue;
        ret_splite = splite_str(p, strlen(p)+1, temp_arr, 1024, temp_strs, 1024, '\t');
        if (ret_splite <= pos_gw || ret_splite <= pos_mask || ret_splite <= pos_dst)
            continue;

        sscanf(temp_strs[pos_dst], "%x", &ndst);
        sscanf(temp_strs[pos_mask], "%x", &nmask);
        sscanf(temp_strs[pos_gw], "%x", &ngw);
        ndst = ntohl(ndst);
        nmask = ntohl(nmask);
        ngw = ntohl(ngw);

        if ((myvlan_prefix=(ndst&nmask)) == 0)
            ndefgw = ngw;
        else if(myvlan_prefix == vlan_prefix)
        {
            nmygw = ngw;
        }
    }

    if (nmygw == 0)
        nmygw = ndefgw; 
    if (nmygw == 0)
    {
        ndst = htonl(vlan_prefix);
        OD( "<ERROR> can not found my %s vlan's default gateway!", 
                inet_ntop(AF_INET, (struct in_addr*)&ndst, temp_addr, 16));
        goto _failed;
    }
    g_route_mgr.def_route.rd.gateway.s_addr = nmygw;
    ndst = htonl(fio_route_defgw());
    OD( "<info> my defgw is %s", inet_ntop(AF_INET, (struct in_addr*)&ndst, temp_addr, 16));

    fclose(fp);
    return 0;

_failed:
    fclose(fp);
    return -1;
}

#if (defined _TEST_ROUTE_)
int main()
{
    struct in_addr sin;
    inet_aton("192.168.22.0", &sin);
    fio_route_read_defroute(ntohl(sin.s_addr));

    char temp_addr[16];
    uint32_t ngw = htonl(g_route_mgr.defgw);
    D("defgw %s!", inet_ntop(AF_INET, (struct in_addr*)&ngw, temp_addr, 16));
    return 0;
}
#endif
