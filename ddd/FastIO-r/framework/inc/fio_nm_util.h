//	$Id: fio_nm_util.h 2013-05-23 likunxiang$
//

#ifndef _NM_UTIL_H
#define _NM_UTIL_H
#include <errno.h>
#include <signal.h>	/* signal */
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>	/* PRI* macros */
#include <string.h>	/* strcmp */
#include <fcntl.h>	/* open */
#include <unistd.h>	/* close */
#include <ifaddrs.h>	/* getifaddrs */

#include <sys/mman.h>	/* PROT_* */
#include <sys/ioctl.h>	/* ioctl */
#include <sys/poll.h>
#include <sys/socket.h>	/* sockaddr.. */
#include <arpa/inet.h>	/* ntohs */
#include <sys/param.h>
#include <sys/sysctl.h>	/* sysctl */
#include <sys/time.h>	/* timersub */

#include <net/ethernet.h>
#include <net/if.h>	/* ifreq */

#ifdef linux
#define ifr_flagshigh  ifr_flags
#define ifr_curcap     ifr_flags
#define ifr_reqcap     ifr_flags
#define IFF_PPROMISC   IFF_PROMISC
#include <linux/ethtool.h>
#include <linux/sockios.h>
#endif /* linux */

#ifdef __APPLE__
#define ifr_flagshigh  ifr_flags	// XXX
#define IFF_PPROMISC   IFF_PROMISC
#endif /* __APPLE__ */

#ifdef __FreeBSD__
#include <sys/endian.h> /* le64toh */
#include <machine/param.h>
#endif /* __FreeBSD__ */

//#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>

#include <net/netmap.h>
#include <net/netmap_user.h>
//#ifndef MY_PCAP
//#include <pcap/pcap.h> // XXX do we need it ?
//#endif // XXX hack

#include <pthread.h>	/* pthread_* */

#if defined(__FreeBSD__)
#include <pthread_np.h> /* pthread w/ affinity */
#include <sys/cpuset.h> /* cpu_set */
#include <net/if_dl.h>  /* LLADDR */
#endif

#if defined(__APPLE__)
#include <net/if_dl.h>  /* LLADDR */
#define clock_gettime(a,b)	\
	do {struct timespec t0 = {0,0};	*(b) = t0; } while (0)
#endif

#ifdef linux
#define CLOCK_REALTIME_PRECISE CLOCK_REALTIME
#include <netinet/ether.h>      /* ether_aton */
#include <linux/if_packet.h>    /* sockaddr_ll */
#endif /* linux */

#ifdef __linux__
#define sockaddr_dl    sockaddr_ll
#define sdl_family     sll_family
#define AF_LINK        AF_PACKET
#define LLADDR(s)      s->sll_addr;
#endif /* linux */

#ifdef __linux__
#define uh_sport source
#define uh_dport dest
#define uh_ulen len
#define uh_sum check
#endif /* linux */

#include "fio_multip.h"

#if (__SIZEOF_POINTER__ == 4)
#define LU64 "llu"
#define SZF "u"
#else
#define LU64 "lu"
#define SZF "lu"
#endif

#define MAX_FIO_PATH 256
extern int16_t g_arp_type;
extern int16_t g_ip_type;
extern const int g_ip_head_offset;
extern const int g_sip_offset;
extern const int g_dip_offset;
extern const int g_ip_check_offset;
extern const int g_payload_offset;
extern const int g_sport_offset;
extern const int g_recvport_offset;
extern const int g_prototype_offset;
extern const int g_eth_type_offset;
extern const int g_eth_dmac;
extern const int g_eth_smac;

static inline int min(int a, int b) { return a < b ? a : b; }

/* debug support */
#define NOD(format, ...)	do {} while(0)

#ifdef __FIO_RELEASE__
#define OD(format, ...)				\
    if (sysconfig.prdebug)  \
	fprintf(stderr, "%s [%d] " format "\n", 	\
	__FUNCTION__, __LINE__, ##__VA_ARGS__)
//#define D(format, ...) do {} while(0)
#else
#define OD(format, ...)				\
	fprintf(stderr, "%s [%d] " format "\n", 	\
	__FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#ifndef EXPERIMENTAL
#define EXPERIMENTAL 0
#endif

#define nm_likely(x)       __builtin_expect(!!(x), 1)
#define nm_unlikely(x)       __builtin_expect(!!(x), 0)

// XXX does it work on 32-bit machines ?
inline void nm_prefetch (const void *x);

// XXX only for multiples of 64 bytes, non overlapped.
inline void nm_pkt_copy(const void *_src, void *_dst, int l);

#if EXPERIMENTAL
/* Wrapper around `rdtsc' to take reliable timestamps flushing the pipeline */ 
#define netmap_rdtsc(t) \
	do { \
		u_int __regs[4];					\
		do_cpuid(0, __regs);					\
		(t) = rdtsc();						\
	} while (0)

static __inline void do_cpuid(u_int ax, u_int *p)
{
	__asm __volatile("cpuid"
			 : "=a" (p[0]), "=b" (p[1]), "=c" (p[2]), "=d" (p[3])
			 :  "0" (ax));
}

static __inline uint64_t rdtsc(void)
{
	uint64_t rv;

	__asm __volatile("rdtsc" : "=A" (rv));
	return (rv);
}
#endif /* EXPERIMENTAL */

struct ip_range {
    char name[256];
    struct in_addr start, end;
};

struct mac_range {
    char name[256];
    struct ether_addr start, end;
};
void extract_ip_range(struct ip_range *r);
void extract_mac_range(struct mac_range *r);

struct pkttcp {
    struct ether_header eh;
    struct ip ip;
    struct tcphdr tcp;
    uint8_t body[0];
} __attribute__((__packed__));

struct pktudp {
    struct ether_header eh;
    struct ip ip;
    struct udphdr udp;
    uint8_t body[1500];	// XXX hardwired
} __attribute__((__packed__));

inline uint16_t wrapsum(uint32_t sum);
inline uint16_t checksum2(const void *data, uint16_t len, uint32_t sum);
inline uint16_t checksum(uint16_t *buf, int32_t nword);
inline void fixCheckSumUDP(struct udphdr * const udp, struct iphdr const * const ip, void const * const data);
uint16_t fixCheckSumTCP(const struct pkttcp *pkttcp);
inline uint16_t ip_fast_csum(const void *iph, unsigned int ihl);
inline uint16_t in_cksum(uint16_t *buffer, int len);
int source_hwaddr(const char *ifname, char *buf);
int get_if_ip(const char * const eth, struct in_addr *ipaddr);
void init_nm_util();

//struct my_ring;
/*
 * info on a ring we handle
 */
struct my_ring {
        const char *ifname;
        int fd;
        char *mem;                      /* userspace mmap address */
        u_int memsize;
        u_int queueid;
        u_int the_begin, the_end;               /* first..last+1 rings to check */
        uint16_t nr_rx_rings;
        uint16_t nr_tx_rings;
        uint32_t nr_rx_slots, nr_tx_slots;
        struct netmap_if *nifp;
        struct netmap_ring *tx, *rx;    /* shortcuts */

        uint32_t if_flags;
        uint32_t if_reqcap;
        uint32_t if_curcap;
        int port;
};
int netmap_open(struct my_ring *me, int ringid, int promisc);
int netmap_close(struct my_ring *me);
int nm_do_ioctl(struct my_ring *me, int what, int subcmd);
int splite_str(char *str, int str_size, char *buf, int buf_size, char **pp, int pp_size, char delimit);
int ip2net_mask(const char *str, uint32_t *src_raw, uint32_t *mask, uint32_t *range);
int fio_create_dir(const char *sPathName);
int fio_get_exec_path( char* path );
#endif /* _NM_UTIL_H */
