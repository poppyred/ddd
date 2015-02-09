//	$Id: fio_pack_misc.h 2013-05-23 likunxiang$
//
#ifndef _FIO_PACK_MISC_H_
#define _FIO_PACK_MISC_H_

#include "fio_nm_util.h"

extern struct pktudp g_pkt4test;
extern struct pktudp g_pkt4test2;

struct fio_nic;
void __initialize_dns_packet(char *str_payload, int len);

#endif
