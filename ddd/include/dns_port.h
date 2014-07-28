#ifndef __DNS_PORT_H__
#define __DNS_PORT_H__



int dns_port_init();


unsigned short dns_get_port();

int dns_port_to_socket(unsigned short port);








#endif