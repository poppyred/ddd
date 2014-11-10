//	$Id: fio_syslog.c 2013-05-23 likunxiang$
//

#include <stdarg.h>
#include "fio_syslog.h"
#include "fio_context.h"
#include "fio_sysconfig.h"
#include "fio_route_manager.h"
#include "fio_log.h"
#include "log_log.h"
#include "fio_statistics.h"

struct fio_syslog
{
    int logfac;
    int loglev;
    int minlvl;
    struct sockaddr_in toaddr; 
    uint16_t srcport;
    char host_name[FIO_MAX_NAME_LEN];
    char module_name[FIO_MAX_NAME_LEN];
    char **logmsg_bythread;
	char **dataptr_bythread;
	long *tvsec;	/* to force the string to be initialized */
};

struct fio_syslog g_fio_slog;

const char * fio_slog_facilities[FIO_SYSLOG_FACILITIES] = {
	"kern", "user", "mail", "daemon",
	"auth", "syslog", "lpr", "news",
	"uucp", "cron", "auth2", "ftp",
	"ntp", "audit", "alert", "cron2",
	"local0", "local1", "local2", "local3",
	"local4", "local5", "local6", "local7"
};

const char *fio_slog_monthname[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};   

int fio_slog_get_facility(const char *fac)
{
	int facility;

	facility = FIO_SYSLOG_FACILITIES - 1;
	while (facility >= 0 && strcmp(fio_slog_facilities[facility], fac))
		facility--;
	
	return facility;
}

static inline void get_localtime(const time_t now, struct tm *tm) 
{       
    localtime_r(&now, tm); 
}       

void fio_slog_close()
{
    int i;
    for (i = 0; i < sysconfig.nthreads; i++)
    {
        if (g_fio_slog.logmsg_bythread[i])
            free(g_fio_slog.logmsg_bythread[i]);
    }
    free(g_fio_slog.logmsg_bythread);
    free(g_fio_slog.dataptr_bythread);
    free(g_fio_slog.tvsec);
}

//level在/usr/include/sys/syslog.h定义
void fio_slog_open(uint32_t toip, const char *host_name, const char *module_name, 
        int level, const char *fac)
{
    int i;
    g_fio_slog.toaddr.sin_addr.s_addr = toip;
    g_fio_slog.toaddr.sin_port = htons(514);
    g_fio_slog.srcport = htons(59315);
    g_fio_slog.loglev = level;
    g_fio_slog.logfac = fio_slog_get_facility(fac);
    g_fio_slog.minlvl = LOG_EMERG;
    strncpy(g_fio_slog.host_name, host_name, FIO_MAX_NAME_LEN);
    strncpy(g_fio_slog.module_name, module_name, FIO_MAX_NAME_LEN);
    g_fio_slog.logmsg_bythread = calloc(sysconfig.nthreads, sizeof(char*));
    for (i = 0; i < sysconfig.nthreads; i++)
        g_fio_slog.logmsg_bythread[i] = calloc(1, MAX_SYSLOG_LEN*sizeof(char));
    g_fio_slog.dataptr_bythread = calloc(sysconfig.nthreads, sizeof(char*));

    g_fio_slog.tvsec = malloc(sysconfig.nthreads * sizeof(long));
    for (i = 0; i < sysconfig.nthreads; i++)
        g_fio_slog.tvsec[i] = -1;
}

//int fio_slog_sendmsg_(struct fio_nic *nic, uint16_t data_len, struct fio_txdata *txd, int need_csum)
//{ 
//    uint32_t nip, get_ret = 0;
//    int snd_ret = 0;
    
//    uint32_t bydefgw = next_hop(txd->dstip, &nip);
//    if (0 != bydefgw && 1 != bydefgw)
//        goto err;

//    get_ret = fio_mac_get_bynip(nic, nip, &txd->dmac, bydefgw);

//    struct pktudp *ppkt = (struct pktudp*)txd->pbuf;
//    struct ether_header *eh = &ppkt->eh;
	
//    struct ip *ip = &ppkt->ip;
//    struct udphdr *udp = &ppkt->udp;
//    int pkt_size = g_payload_offset+ntohs(data_len);
//    if (pkt_size > MAX_PKT_SIZE)
//    {
//        pkt_size = MAX_PKT_SIZE;
//        data_len = htons(MAX_PAYLOAD_SIZE);
//    }
//    ip->ip_len = ntohs(pkt_size - sizeof(*eh));
//    ip->ip_dst.s_addr = txd->dstip;
//    ip->ip_src.s_addr = nic->if_ipdef.s_addr;

//    ip->ip_sum = 0;
//    ip->ip_sum = ip_fast_csum((char*)ip, ip->ip_hl);

//    udp->uh_sport = txd->srcport;
//    udp->uh_dport = txd->dstport;
//    udp->uh_ulen = htons(pkt_size-g_sport_offset);
//    udp->uh_sum = 0;
//    if (need_csum)
//        fixCheckSumUDP(udp, (struct iphdr*)ip, ppkt->body);

//    bcopy(&nic->if_mac, eh->ether_shost, ETH_ALEN);
//    if (0 == get_ret)
//        bcopy(txd->dmac, eh->ether_dhost, ETH_ALEN);
//    else
//        memset(eh->ether_dhost, 0, ETH_ALEN);
//    txd->size = pkt_size;

//    if (0 == get_ret)
//        snd_ret = fio_nic_commit(nic, txd, 1);
//    else
//        snd_ret = fio_mac_cache_pkt(nic, nip, txd);

//go_out:
//   return snd_ret;
//err:
//   snd_ret = -1; 
//   goto go_out;
//}

void fio_slog_snd_format(struct fio_nic *nic, const char *message, ...)
{
	//static long tvsec = -1;	/* to force the string to be initialized */
	//static char logmsg[MAX_SYSLOG_LEN];
	//static char *dataptr = NULL;
    int tid = NIC_EXTRA_CONTEXT(nic)->me;
	int hdr_len, data_len;
	va_list argp;
	int fac_level;
    char *const logmsg = g_fio_slog.logmsg_bythread[tid];
    char *dataptr = g_fio_slog.dataptr_bythread[tid];
	char *log_ptr;
    struct fio_txdata *txd;

	if (!message)
        goto err;

	if (nm_unlikely(fio_now.tv_sec != g_fio_slog.tvsec[tid] || !dataptr)) {
		/* this string is rebuild only once a second */
		struct tm tm;

		g_fio_slog.tvsec[tid] = fio_now.tv_sec;
		get_localtime(g_fio_slog.tvsec[tid], &tm);

		hdr_len = snprintf(logmsg, MAX_SYSLOG_LEN,
				   "<<<<>%s %2d %02d:%02d:%02d %s_%s[%d]: ",
				   fio_slog_monthname[tm.tm_mon],
				   tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
				   g_fio_slog.host_name, g_fio_slog.module_name, tid);
		/* WARNING: depending upon implementations, snprintf may return
		 * either -1 or the number of bytes that would be needed to store
		 * the total message. In both cases, we must adjust it.
		 */
		if (hdr_len < 0 || hdr_len > MAX_SYSLOG_LEN)
			hdr_len = MAX_SYSLOG_LEN;

		dataptr = g_fio_slog.dataptr_bythread[tid] = logmsg + hdr_len;
	}

	va_start(argp, message);
	/*
	 * FIXME: we take a huge performance hit here. We might have to replace
	 * vsnprintf() for a hard-coded log writer.
	 */
	data_len = vsnprintf(dataptr, logmsg + MAX_SYSLOG_LEN -1 - dataptr, message, argp);
	if (data_len < 0 || data_len > (logmsg + MAX_SYSLOG_LEN -1 - dataptr))
		data_len = logmsg + MAX_SYSLOG_LEN -1 - dataptr;
	va_end(argp);
	dataptr[data_len] = '\n'; /* force a break on ultra-long lines */

    fac_level = (g_fio_slog.logfac << 3) + g_fio_slog.loglev;
    log_ptr = logmsg + 3; /* last digit of the log level */
    do {
        *log_ptr = '0' + fac_level % 10;
        fac_level /= 10;
        log_ptr--;
    } while (fac_level && log_ptr > logmsg);
    *log_ptr = '<';

    if (fio_nic_reserve_tx(nic, &txd, NULL) < 1)
        goto err;

    txd->dstip = g_fio_slog.toaddr.sin_addr.s_addr;
    txd->dstport = g_fio_slog.toaddr.sin_port;
    txd->srcport = g_fio_slog.srcport;
    memcpy(txd->pdata, log_ptr, dataptr + data_len - log_ptr);
    if (fio_send(nic, htons(dataptr + data_len - log_ptr), txd, 0) < 1)
        OD( "tid %d nic %s send syslog error", tid, nic->alise);

err:
    return;
}

void fio_slog_snd(struct fio_nic *nic, const char *message, int data_len)
{
	//static long tvsec = -1;	/* to force the string to be initialized */
	//static char logmsg[MAX_SYSLOG_LEN];
	//static char *dataptr = NULL;
    int tid = NIC_EXTRA_CONTEXT(nic)->me;
	int hdr_len;
	int fac_level;
    char *const logmsg = g_fio_slog.logmsg_bythread[tid];
    char *dataptr = g_fio_slog.dataptr_bythread[tid];
	char *log_ptr;
    struct fio_txdata *txd;

	if (!message)
        goto err;

	if (nm_unlikely(fio_now.tv_sec != g_fio_slog.tvsec[tid] || !dataptr)) {
		/* this string is rebuild only once a second */
		struct tm tm;

		g_fio_slog.tvsec[tid] = fio_now.tv_sec;
		get_localtime(g_fio_slog.tvsec[tid], &tm);

		hdr_len = snprintf(logmsg, MAX_SYSLOG_LEN,
				   "<<<<>%s %2d %02d:%02d:%02d %s_%s[%d]: ",
				   fio_slog_monthname[tm.tm_mon],
				   tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
				   g_fio_slog.host_name, g_fio_slog.module_name, tid);
		/* WARNING: depending upon implementations, snprintf may return
		 * either -1 or the number of bytes that would be needed to store
		 * the total message. In both cases, we must adjust it.
		 */
		if (hdr_len < 0 || hdr_len > MAX_SYSLOG_LEN)
			hdr_len = MAX_SYSLOG_LEN;

		dataptr = g_fio_slog.dataptr_bythread[tid] = logmsg + hdr_len;
	}

	if (data_len < 0 || data_len > (logmsg + MAX_SYSLOG_LEN -1 - dataptr))
		data_len = logmsg + MAX_SYSLOG_LEN -1 - dataptr;
	memcpy(dataptr, message, data_len);
	dataptr[data_len] = '\n'; /* force a break on ultra-long lines */

    fac_level = (g_fio_slog.logfac << 3) + g_fio_slog.loglev;
    log_ptr = logmsg + 3; /* last digit of the log level */
    do {
        *log_ptr = '0' + fac_level % 10;
        fac_level /= 10;
        log_ptr--;
    } while (fac_level && log_ptr > logmsg);
    *log_ptr = '<';

    if (fio_nic_reserve_tx(nic, &txd, NULL) < 1)
        goto err;

    txd->dstip = g_fio_slog.toaddr.sin_addr.s_addr;
    txd->dstport = g_fio_slog.toaddr.sin_port;
    txd->srcport = g_fio_slog.srcport;
    memcpy(txd->pdata, log_ptr, dataptr + data_len - log_ptr);
    if (fio_send(nic, htons(dataptr + data_len - log_ptr), txd, 0) < 1)
        OD( "tid %d nic %s send syslog error", tid, nic->alise);

err:
    return;
}

