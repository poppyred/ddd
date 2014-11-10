#ifndef _FIO_SOCKPAIR_H__
#define _FIO_SOCKPAIR_H__

#include <netinet/in.h>
#include <stdint.h>

int fio_socketpair(int af, struct in_addr *if_ip, uint16_t sport, uint16_t delta, int type, int protocol, int fd[2]);
int fio_usocketpair(struct in_addr *if_ip, uint16_t sport, int fd[2]);

#endif
