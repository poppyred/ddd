//	$Id: fio_conf.h 2013-05-23 likunxiang$
//
#ifndef __FIO_CONF_HEAD__
#define __FIO_CONF_HEAD__

#define MAX_IPMAC 2000

#include <netinet/if_ether.h>
#include <netinet/in.h>

struct IPDST
{
    char ip[20];
    char mac[30];
};

struct IPMAC
{
    char ip[20];
    char mac[30];
    char via[30];
    struct in_addr _ip;
    struct ether_addr _mac;
};
extern struct IPMAC * g_ip_macs[MAX_IPMAC];
extern int g_mac_num;

int parse_config(const char *file_name);
void translate_port_clear();

#endif

