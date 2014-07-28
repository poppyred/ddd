#ifndef __DNS_RSYSLOG_H__
#define __DNS_RSYSLOG_H__



#include <syslog.h>


void dns_syslog_init();

void dns_syslog_uninit();

#define dns_rsyslog(fmt, args ...) \
    do {syslog(LOG_INFO,fmt,##args);}while(0)




#endif
