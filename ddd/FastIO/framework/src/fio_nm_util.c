//	$Id: fio_nm_util.c 2013-05-23 likunxiang$
//

#include "fio_nm_util.h"
#ifdef __FIO_RELEASE__
#include "fio_sysconfig.h"
#endif
#include <ctype.h>
#include <netdb.h>
#include <sys/stat.h>
#include <assert.h>
//#include <log_log.h>

int verbose = 0;

int16_t g_arp_type;
int16_t g_ip_type;
const int g_ip_head_offset = sizeof(struct ethhdr);
const int g_sip_offset = sizeof(struct ethhdr) + (u_long)&((struct iphdr*)0)->saddr;
const int g_dip_offset = sizeof(struct ethhdr) + (u_long)&((struct iphdr*)0)->daddr;
const int g_ip_check_offset = sizeof(struct ethhdr) + (u_long)&((struct iphdr*)0)->check;
const int g_payload_offset = sizeof(struct ether_header)+sizeof(struct ip)+sizeof(struct udphdr);
const int g_sport_offset = sizeof(struct ethhdr) + sizeof(struct iphdr);
const int g_recvport_offset = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(uint16_t);
const int g_prototype_offset = sizeof(struct ethhdr) + (u_long)&((struct iphdr*)0)->protocol;
const int g_eth_type_offset = (u_long)&((struct ethhdr*)0)->h_proto;
const int g_eth_dmac = (u_long)&((struct ethhdr*)0)->h_dest;
const int g_eth_smac = (u_long)&((struct ethhdr*)0)->h_source;

int nm_do_ioctl(struct my_ring *me, int what, int subcmd)
{
    struct ifreq ifr;
    int error;
#if defined( __FreeBSD__ ) || defined (__APPLE__)
    int fd = me->fd;
#endif
#ifdef linux 
    struct ethtool_value eval;
    int fd;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        printf("Error: cannot get device control socket.\n");
        return -1;
    }
#endif /* linux */

    (void)subcmd;	// unused
    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, me->ifname, sizeof(ifr.ifr_name));
    switch (what) {
        case SIOCSIFFLAGS:
#ifndef __APPLE__
            ifr.ifr_flagshigh = me->if_flags >> 16;
#endif
            ifr.ifr_flags = me->if_flags & 0xffff;
            break;

#if defined( __FreeBSD__ )
        case SIOCSIFCAP:
            ifr.ifr_reqcap = me->if_reqcap;
            ifr.ifr_curcap = me->if_curcap;
            break;
#endif
#ifdef linux
        case SIOCETHTOOL:
            eval.cmd = subcmd;
            eval.data = 0;
            ifr.ifr_data = (caddr_t)&eval;
            break;
#endif /* linux */
    }
    error = ioctl(fd, what, &ifr);
    if (error)
        goto done;
    switch (what) {
        case SIOCGIFFLAGS:
#ifndef __APPLE__
            me->if_flags = (ifr.ifr_flagshigh << 16) |
                (0xffff & ifr.ifr_flags);
#endif
            if (verbose)
                OD("flags are 0x%x", me->if_flags);
            break;

#if defined( __FreeBSD__ )
        case SIOCGIFCAP:
            me->if_reqcap = ifr.ifr_reqcap;
            me->if_curcap = ifr.ifr_curcap;
            if (verbose)
                OD("curcap are 0x%x", me->if_curcap);
            break;
#endif /* __FreeBSD__ */
    }
done:
#ifdef linux
    close(fd);
#endif
    if (error)
        OD("ioctl error %d %d", error, what);
    return error;
}

/*
 * open a device. if me->mem is null then do an mmap.
 * Returns the file descriptor.
 * The extra flag checks configures promisc mode.
 */
int netmap_open(struct my_ring *me, int ringid, int promisc)
{
    int fd, err, l;
    struct nmreq req;

    me->fd = fd = open("/dev/netmap", O_RDWR);
    if (fd < 0) {
        OD("Unable to open /dev/netmap");
        return (-1);
    }
    bzero(&req, sizeof(req));
    req.nr_version = NETMAP_API;
    strncpy(req.nr_name, me->ifname, sizeof(req.nr_name));
    req.nr_ringid = ringid;
    err = ioctl(fd, NIOCGINFO, &req);
    if (err) {
        OD("cannot get info on %s, errno %d ver %d",
                me->ifname, errno, req.nr_version);
        goto error;
    }
    OD("ringid is %d", req.nr_ringid);
    me->memsize = l = req.nr_memsize;
    if (verbose)
        OD("memsize is %d MB", l>>20);
    err = ioctl(fd, NIOCREGIF, &req);
    if (err) {
        OD("Unable to register %s", me->ifname);
        goto error;
    }

    if (me->mem == NULL) {
        me->mem = mmap(0, l, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
        if (me->mem == MAP_FAILED) {
            OD("Unable to mmap");
            me->mem = NULL;
            goto error;
        }
    }


    /* Set the operating mode. */
    if (ringid != NETMAP_SW_RING) {
        nm_do_ioctl(me, SIOCGIFFLAGS, 0);
        if ((me[0].if_flags & IFF_UP) == 0) {
            OD("%s is down, bringing up...", me[0].ifname);
            me[0].if_flags |= IFF_UP;
        }
        if (promisc) {
            me[0].if_flags |= IFF_PPROMISC;
            nm_do_ioctl(me, SIOCSIFFLAGS, 0);

            //nm_do_ioctl(me+1, SIOCGIFFLAGS, 0);
            //me[1].if_flags |= IFF_PPROMISC;
            //nm_do_ioctl(me+1, SIOCSIFFLAGS, 0);
        }

#ifdef __FreeBSD__
        /* also disable checksums etc. */
        nm_do_ioctl(me, SIOCGIFCAP, 0);
        me[0].if_reqcap = me[0].if_curcap;
        me[0].if_reqcap &= ~(IFCAP_HWCSUM | IFCAP_TSO | IFCAP_TOE);
        nm_do_ioctl(me+0, SIOCSIFCAP, 0);
#endif
#ifdef linux
        /* disable:
         * - generic-segmentation-offload
         * - tcp-segmentation-offload
         * - rx-checksumming
         * - tx-checksumming
         * XXX check how to set back the caps.
         */
        nm_do_ioctl(me, SIOCETHTOOL, ETHTOOL_SGSO);
        nm_do_ioctl(me, SIOCETHTOOL, ETHTOOL_STSO);
        nm_do_ioctl(me, SIOCETHTOOL, ETHTOOL_SRXCSUM);
        nm_do_ioctl(me, SIOCETHTOOL, ETHTOOL_STXCSUM);
#endif /* linux */
    }

    me->nifp = NETMAP_IF(me->mem, req.nr_offset);
    me->queueid = ringid;
    me->nr_rx_rings = req.nr_rx_rings;
    me->nr_tx_rings = req.nr_tx_rings;
    OD("ringid is %d", ringid);
    if (ringid & NETMAP_SW_RING) {
        me->the_begin = req.nr_rx_rings;
        me->the_end = me->the_begin + 1;
        me->nr_tx_slots = req.nr_tx_slots;
        me->nr_rx_slots = req.nr_rx_slots;
        me->tx = NETMAP_TXRING(me->nifp, req.nr_tx_rings);
        me->rx = NETMAP_RXRING(me->nifp, req.nr_rx_rings);
        OD("begin is %d", me->the_begin);
        OD("end is %d", me->the_end);
        OD("tx slot is %d", me->nr_tx_slots);
        OD("rx slot is %d", me->nr_rx_slots);
    } else if (ringid & NETMAP_HW_RING) {
        OD("XXX check multiple threads");
        me->the_begin = ringid & NETMAP_RING_MASK;
        me->the_end = me->the_begin + 1;
        me->nr_tx_slots = req.nr_tx_slots;
        me->nr_rx_slots = req.nr_rx_slots;
        me->tx = NETMAP_TXRING(me->nifp, me->the_begin);
        me->rx = NETMAP_RXRING(me->nifp, me->the_begin);
        OD("begin is %d", me->the_begin);
        OD("end is %d", me->the_end);
        OD("tx slot is %d", me->nr_tx_slots);
        OD("rx slot is %d", me->nr_rx_slots);
    } else {
        me->the_begin = 0;
        me->the_end = req.nr_rx_rings; // XXX max of the two
        me->nr_tx_slots = req.nr_tx_slots;
        me->nr_rx_slots = req.nr_rx_slots;
        me->tx = NETMAP_TXRING(me->nifp, 0);
        me->rx = NETMAP_RXRING(me->nifp, 0);
        OD( "begin is %d", me->the_begin);
        OD( "end is %d", me->the_end);
        OD( "tx slot is %d", me->nr_tx_slots);
        OD("rx slot is %d", me->nr_rx_slots);
    }
    return (0);
error:
    close(me->fd);
    return -1;
}



/*
 * how many packets on this set of queues ?
 */


void extract_ip_range(struct ip_range *r)
{
    OD("extract IP range from %s", r->name);
    inet_aton(r->name, &r->start);
    inet_aton(r->name, &r->end);
    OD("%s starts at %s", r->name, inet_ntoa(r->start));
}

void extract_mac_range(struct mac_range *r)
{
    OD("extract MAC range from %s", r->name);
    if (strlen(r->name))
    { 
        bcopy(ether_aton(r->name), &r->start, ETH_ALEN);
        bcopy(ether_aton(r->name), &r->end, ETH_ALEN);
        OD("%s starts at %s", r->name, ether_ntoa(&r->start));
    }
}

inline uint16_t wrapsum(uint32_t sum)
{
    sum = ~sum & 0xFFFF;
    return (htons(sum));
}

struct pseudo_header_udp //udp伪头部
{   
    struct in_addr src;
    struct in_addr dst;
    uint8_t                mbz;
    uint8_t                proto;
    uint16_t                len;
} __attribute__((__packed__));
struct pseudo_header_tcp //tcp伪头部
{
    struct in_addr src_ip; /* source ip */
    struct in_addr dest_ip; /* destination ip */
    uint8_t zeroes; /* = 0 */
    uint8_t protocol; /* = 6 */
    uint16_t len; /* length of TCPHeader */
} __attribute__ ((__packed__));
inline uint16_t in_cksum(uint16_t *buffer, int len)
{
    uint32_t sum = 0;
    while (len > 1) 
    {
        sum += *buffer++;
        len -= sizeof(uint16_t);
    }

    if (len) 
    {
        sum += *(uint8_t*)buffer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (uint16_t)(~sum);
}

inline void fixCheckSumUDP(struct udphdr * const udp, struct iphdr const * const ip, void const * const data) 
{
    uint32_t                sum;
    struct pseudo_header_udp pseudo_hdr;
    pseudo_hdr.src.s_addr = (ip->saddr);
    pseudo_hdr.dst.s_addr = (ip->daddr);
    pseudo_hdr.mbz   = 0;
    pseudo_hdr.proto = ip->protocol;
    pseudo_hdr.len   = udp->len;
    udp->uh_sum = 0;
    sum = checksum2((uint16_t*)&pseudo_hdr, sizeof(pseudo_hdr), 0); 
    sum = checksum2((uint16_t*)udp, sizeof(*udp), sum);
    sum = checksum2((uint16_t*)data, ntohs(udp->len)-sizeof(*udp), sum);

    sum = ~ntohs(sum);
    if (sum==0) 
        sum=~sum;

    udp->uh_sum = sum;
}
static uint16_t in_cksum_tcp (const void * addr, unsigned len, const uint8_t *buffer, int blen, uint16_t init) 
{
    uint32_t sum;
    const uint16_t * word;
    sum = init;
    word = addr;
    while (len >= 2) {
        sum += *(word++);
        len -= 2;
    }
    word = (const uint16_t*)buffer;
    while (blen >= 2) {
        sum += *(word++);
        blen -= 2;
    }
    if (blen > 0) {
        sum += *(uint8_t*)word;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ((uint16_t)~sum);
}
uint16_t fixCheckSumTCP(const struct pkttcp *pkttcp) 
{
    unsigned ipHdrLen;
    unsigned ipPacketLen;
    unsigned ipPayloadLen;
    struct pseudo_header_tcp pseudo;
    const struct iphdr * ip = (const struct iphdr*)&pkttcp->ip;
    ipHdrLen = ip->ihl * 4;
    ipPacketLen = ntohs(ip->tot_len);
    ipPayloadLen = ipPacketLen - ipHdrLen;
    pseudo.src_ip.s_addr = ip->saddr;
    pseudo.dest_ip.s_addr = ip->daddr;
    pseudo.zeroes = 0;
    pseudo.protocol = ip->protocol;
    pseudo.len = htons(ipPayloadLen);
    return in_cksum_tcp(&pseudo, (unsigned)sizeof(pseudo), (const uint8_t*)&pkttcp->tcp, ipPayloadLen, 0);
}

inline uint16_t checksum2(const void *data, uint16_t len, uint32_t sum)
{
    const uint8_t *addr = data;
    uint32_t i;

    /* Checksum all the pairs of bytes first... */
    for (i = 0; i < (len & ~1U); i += 2) {
        sum += (u_int16_t)ntohs(*((u_int16_t *)(addr + i)));
        if (sum > 0xFFFF)
            sum -= 0xFFFF;
    }
    /*
     *      * If there's a single byte left over, checksum it, too.
     *           * Network byte order is big-endian, so the remaining byte is
     *                * the high byte.
     *                     */
    if (i < len) {
        sum += addr[i] << 8;
        if (sum > 0xFFFF)
            sum -= 0xFFFF;
    }
    return sum;
}

inline uint16_t checksum(uint16_t *buf, int32_t nword)
{
    unsigned long sum;

    for(sum = 0; nword > 0; nword--)
        sum += *buf++;

    sum = (sum>>16) + (sum&0xffff);
    sum += (sum>>16);

    return ~sum;
}

inline uint16_t ip_fast_csum(const void *iph, unsigned int ihl)
{
    unsigned int sum;

    __asm__ __volatile__(
            "movl (%1), %0 ;\n"
            "subl $4, %2 ;\n"
            "jbe 2f ;\n"
            "addl 4(%1), %0 ;\n"
            "adcl 8(%1), %0 ;\n"
            "adcl 12(%1), %0 ;\n"
            "1:     adcl 16(%1), %0 ;\n"
            "lea 4(%1), %1 ;\n"
            "decl %2 ;\n"
            "jne 1b ;\n"
            "adcl $0, %0 ;\n"
            "movl %0, %2 ;\n"
            "shrl $16, %0 ;\n"
            "addw %w2, %w0 ;\n"
            "adcl $0, %0 ;\n"
            "notl %0 ;\n"
            "2: ;\n"
            /* Since the input registers which are loaded with iph and ihl
             *   are modified, we must also specify them as outputs, or gcc
             *     will assume they contain their original values. */
            : "=r" (sum), "=r" (iph), "=r" (ihl)
            : "1" (iph), "2" (ihl)
               : "memory");
    return(sum);
}

/*
 * locate the src mac address for our interface, put it
 * into the user-supplied buffer. return 0 if ok, -1 on error.
 */
int source_hwaddr(const char *ifname, char *buf)
{
    struct ifaddrs *ifaphead, *ifap;
    int l = sizeof(ifap->ifa_name);

    if (getifaddrs(&ifaphead) != 0) {
        OD("getifaddrs %s failed", ifname);
        return (-1);
    }

    for (ifap = ifaphead; ifap; ifap = ifap->ifa_next) {
        struct sockaddr_dl *sdl =
            (struct sockaddr_dl *)ifap->ifa_addr;
        uint8_t *mac;

        if (!sdl || sdl->sdl_family != AF_LINK)
            continue;
        if (strncmp(ifap->ifa_name, ifname, l) != 0)
            continue;
        mac = (uint8_t *)LLADDR(sdl);
        sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
                mac[0], mac[1], mac[2],
                mac[3], mac[4], mac[5]);
        if (verbose)
            OD("source hwaddr %s", buf);
        break;
    }
    freeifaddrs(ifaphead);
    return ifap ? 0 : 1;
}

int get_if_ip(const char * const eth, struct in_addr *ipaddr)
{
    int sock_fd;
    struct  sockaddr_in my_addr;
    struct ifreq ifr;

    /**//* Get socket file descriptor */
    if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
    {
        OD("open sock dgram, errno %s", strerror(errno));
        return -1;
    }

    /**//* Get IP Address */
    strncpy(ifr.ifr_name, eth, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ-1]='\0';

    if (ioctl(sock_fd, SIOCGIFADDR, &ifr) < 0)
    {
        OD("No Such Device %s",eth);
        return -1;
    }

    memcpy(&my_addr, &ifr.ifr_addr, sizeof(my_addr));
    *ipaddr = my_addr.sin_addr;
    OD("if %s, ip %s", eth, inet_ntoa(*ipaddr));
    close(sock_fd);
    return 0;
}

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

int ip2net_mask(const char *str, uint32_t *src_raw, uint32_t *mask,
        uint32_t *range)
{
    __label__ out_free, out_err;
    char *c, *s, *s_raw;
    int ret_val;
    struct sockaddr_in sa;

    s = strdup(str);
    s_raw = s;
    if (!s_raw)
        return 0;

    memset(&sa, 0, sizeof(sa));
    *src_raw = 0;
    *mask = ~0U;
    *range = 1;

    if ((c = strrchr(s, '/')) != NULL) {
        *c++ = '\0';
        /* c points to the mask */
        char *err;
        if(inet_pton(AF_INET, s, &sa.sin_addr))
        {
            *src_raw = ntohl((unsigned long)(sa.sin_addr.s_addr));
            *mask = (~0U << (32-strtol(c, &err, 10)));
            *range = (1U << (32-strtol(c, &err, 10)));
        }
        else
            goto out_err;
    }
    else
    {
        if (!inet_pton(AF_INET, s, &sa.sin_addr)) {
            struct hostent *he;

            if ((he = gethostbyname(s)) == NULL) {
                goto out_err;
            }
            else
            {
                sa.sin_addr = *(struct in_addr *) *(he->h_addr_list);
                *src_raw = ntohl((unsigned long)(sa.sin_addr.s_addr));
            }
        }
    }

    //sa.sin_family = AF_INET;
    ret_val = 1;
out_free:
    free(s_raw);
    return ret_val;
out_err:
    ret_val = 0;
    goto out_free;
}

void init_nm_util()
{
    g_arp_type = htons(ETHERTYPE_ARP);
    g_ip_type = htons(ETHERTYPE_IP);
}

inline void nm_prefetch (const void *x)
{
    __asm volatile("prefetcht0 %0" :: "m" (*(const unsigned long *)x));
}

inline void nm_pkt_copy(const void *_src, void *_dst, int l)
{
	const uint64_t *src = _src;
	uint64_t *dst = _dst;
	if (nm_unlikely(l >= 1024)) {
		bcopy(src, dst, l);
		return;
	}
	for (; l > 0; l-=64) {
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
	}
}

int fio_create_dir(const char *sPathName)
{  
    char DirName[MAX_FIO_PATH+1];  
    strncpy(DirName, sPathName, MAX_FIO_PATH);  
    int i, len = strlen(DirName);  
    if(DirName[len-1] != '/')  
        strcat(DirName, "/");  
    len = strlen(DirName);  
    for(i=1; i<len; i++)  
    {  
        if(DirName[i]=='/')  
        {  
            DirName[i] = 0;  
            if( access(DirName, 0) != 0 )  
            {  
                if(mkdir(DirName, 0755) == -1)  
                {   
                    perror("mkdir error");   
                    return -1;   
                }  
            }  
            DirName[i] = '/';  
        }  
    }  
    return 0;  
} 
int fio_get_real_path( char* path, const char* _exec_name )
{   
    char exec_name[MAX_FIO_PATH];
    memset( exec_name, 0, MAX_FIO_PATH );
    strcpy( exec_name, _exec_name );
    char *ptoken = NULL;
    char *psplit = "/";
    int offset =1;
    int lastlen = 0;
    path[0] = '/';
    ptoken = strtok( exec_name, psplit );
    while( ptoken )
    {
        if( offset >= MAX_FIO_PATH ){
            return -1;
        }
        strcpy( path+offset, ptoken );
        lastlen = strlen( ptoken );
        offset += lastlen;
        path[offset] = '/';
        offset++;
        ptoken = strtok( NULL , psplit );
    }
    path[offset-lastlen-2] = 0;
    return 0;
}
int fio_get_exec_path( char* path )
{   
    assert( path );
    char exec_name[MAX_FIO_PATH];
    memset( exec_name, 0, MAX_FIO_PATH );
    char file_name[MAX_FIO_PATH];
    memset( file_name, 0, MAX_FIO_PATH );
    sprintf( file_name, "/proc/%d/exe", getpid() );
    int len = readlink( file_name, exec_name, sizeof( exec_name ) );
    if( len < 0 ) {
        return len; 
    }
    exec_name[len] = 0;
    return fio_get_real_path( path, exec_name );
}   
