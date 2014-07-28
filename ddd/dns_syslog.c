#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "dns_syslog.h"


void dns_syslog_init()
{
    openlog("dnsproxy", LOG_CONS | LOG_PID, LOG_LOCAL3);
}

void dns_syslog_uninit()
{
    closelog();
}

