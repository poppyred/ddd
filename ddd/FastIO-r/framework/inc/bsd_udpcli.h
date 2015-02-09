//	$Id: bsd_udpcli.h 2014-12-18 likunxiang$

#ifndef _BSD_UDPCLI_H_
#define _BSD_UDPCLI_H_

#include <stdint.h>

int buc_write(int sockfd, char *data, int len);
int buc_open(char *svr_ip, uint16_t svr_port);


#endif
