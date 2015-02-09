//	$Id: fio_statistics.c 2013-05-23 likunxiang$
//
#include "fio_statistics.h"
#include "fio_sysconfig.h"
#include "fio_context.h"
//#include "log_log.h"
#include "bsd_udpcli.h"
 #include <syslog.h>
#include <stdarg.h>

#define MAX_SYSLOG_DATA 10240
#define SYSLOG_FACILITIES 24
struct timeval fio_now;
uint32_t fio_gnow = 0;
time_t g_cur_ts;

static char g_logmsg[MAX_SYSLOG_DATA];
static char *g_ptr_logdata = NULL;
static long g_log_tvsec = -1;
static char *g_log_host_name;
static char g_str_logfac[] = "local1";
static int g_logfac;
static int g_loglevel = LOG_INFO;
static const char * syslog_facilities[SYSLOG_FACILITIES] = {
	"kern", "user", "mail", "daemon",
	"auth", "syslog", "lpr", "news",
	"uucp", "cron", "auth2", "ftp",
	"ntp", "audit", "alert", "cron2",
	"local0", "local1", "local2", "local3",
	"local4", "local5", "local6", "local7"
};
static const char *syslog_monthname[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};   
static int syslog_get_facility(const char *fac)
{
	int facility;
	facility = SYSLOG_FACILITIES - 1;
	while (facility >= 0 && strcmp(syslog_facilities[facility], fac))
		facility--;
	return facility;
}
inline void tv_update_date()
{
    gettimeofday(&fio_now, NULL);
}

void syslog_format(int bucfd, const char *message, ...)
{
	int hdr_len, data_len;
	va_list argp;
	int fac_level;
	char *log_ptr;
	if (!message)
        goto err;
	if (nm_unlikely(fio_now.tv_sec != g_log_tvsec || !g_ptr_logdata)) {
		struct tm tm;
		g_log_tvsec = fio_now.tv_sec;
        localtime_r(&g_log_tvsec, &tm); 
		hdr_len = snprintf(g_logmsg, MAX_SYSLOG_DATA,
				   "<<<<>%s %2d %02d:%02d:%02d %s:",
				   syslog_monthname[tm.tm_mon],
				   tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
				   g_log_host_name);
		if (hdr_len < 0 || hdr_len > MAX_SYSLOG_DATA)
			hdr_len = MAX_SYSLOG_DATA;
		g_ptr_logdata = g_logmsg + hdr_len;
	}
	va_start(argp, message);
	data_len = vsnprintf(g_ptr_logdata, g_logmsg + MAX_SYSLOG_DATA -1 - g_ptr_logdata, message, argp);
	if (data_len < 0 || data_len > (g_logmsg + MAX_SYSLOG_DATA -1 - g_ptr_logdata))
		data_len = g_logmsg + MAX_SYSLOG_DATA -1 - g_ptr_logdata;
	va_end(argp);
	g_ptr_logdata[data_len] = '\n'; /* force a break on ultra-long lines */
    fac_level = (g_logfac << 3) + g_loglevel;
    log_ptr = g_logmsg + 3; /* last digit of the log level */
    do {
        *log_ptr = '0' + fac_level % 10;
        fac_level /= 10;
        log_ptr--;
    } while (fac_level && log_ptr > g_logmsg);
    *log_ptr = '<';
    buc_write(bucfd, log_ptr, g_ptr_logdata + data_len - log_ptr);
err:
    return;
}
void * fio_count(void *data)
{
    int i;
    uint64_t my_rxcount = 0, my_txcount = 0, unuse_count = 0, prev_rx = 0, prev_tx = 0;
    struct timeval tic, toc;
    int report_interval = 1*1000;	/* report interval */
    char dump_buf_rx[2000] = {0};
    char dump_buf_tx[2000] = {0};
    char dump_buf[1000] = {0};
	
	g_logfac = syslog_get_facility(g_str_logfac);
    g_log_host_name = sysconfig.src_ip.name;

    int bucfd = buc_open(sysconfig.dst_ip[0].name, sysconfig.src_port);
    if (bucfd == -1)
        return NULL;

    gettimeofday(&toc, NULL);
    while (sysconfig.working)
    {
        struct timeval delta;
        uint64_t pps_rx, pps_tx;
        int done = 0;
        dump_buf_rx[0] = '\0';
        dump_buf_tx[0] = '\0';

        delta.tv_sec = report_interval/1000;
        delta.tv_usec = (report_interval%1000)*1000;

        //计时== 
        select(0, NULL, NULL, NULL, &delta);

        tv_update_date();
        timersub(&fio_now, &toc, &toc);
        my_rxcount = 0;
        my_txcount = 0;
        unuse_count = 0;
        for (i = 0; i < sysconfig.nthreads; i++) {
            my_rxcount += g_contexts[i].nics[0].rxcount;
            my_rxcount += g_contexts[i].nics[1].rxcount;
            my_txcount += g_contexts[i].nics[0].txcount;
            my_txcount += g_contexts[i].nics[1].txcount;
            unuse_count += g_contexts[i].nics[0].unuse_count;
            unuse_count += g_contexts[i].nics[1].unuse_count;
            sprintf(dump_buf, "%"LU64" ", g_contexts[i].nics[0].rxcount 
                    + g_contexts[i].nics[1].rxcount);
            strcat(dump_buf_rx, dump_buf);
            sprintf(dump_buf, "%"LU64" ", g_contexts[i].nics[0].txcount
                    + g_contexts[i].nics[1].txcount);
            strcat(dump_buf_tx, dump_buf);
            if (g_contexts[i].used == 0)
                done++;
        }
        pps_tx = pps_rx = toc.tv_sec* 1000000 + toc.tv_usec;
        if (pps_rx < 10000)
            continue;
        pps_rx = (my_rxcount - prev_rx)*1000000 / pps_rx;
        OD( "rx %"LU64" ppsrx %s", pps_rx, dump_buf_rx);
        pps_tx = (my_txcount - prev_tx)*1000000 / pps_tx;
        OD( "tx %"LU64" ppstx %s", pps_tx, dump_buf_tx);
        OD( "rx useless pkt %"LU64"\n", unuse_count);
        syslog_format(bucfd, "rx %"LU64" ppsrx %s\ntx %"LU64" ppstx %s\nrx useless pkt %"LU64,
                pps_rx, dump_buf_rx, pps_tx, dump_buf_tx, unuse_count);
        prev_rx = my_rxcount;
        prev_tx = my_txcount;
        toc = fio_now;
        fio_gnow++;
        if (done == sysconfig.nthreads)
            break;
    }

    timerclear(&tic);
    timerclear(&toc);
    OD( "I'll shutdown");

    return NULL;
}

