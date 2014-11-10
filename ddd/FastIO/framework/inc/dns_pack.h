//	$Id: dns_pack.h 2013-05-23 likunxiang$
//
#ifndef __DNS_PACK_H__
#define __DNS_PACK_H__

int dns_pack_head(char*result,char*domain,int domain_len);

int dns_pack_answer(char*result,int total_len,void *ip,unsigned short ttl);

void dns_pack_destroy(char *msg);

char *dns_pack_init();

void dns_edit_ttl(char* result, int total_len, unsigned short ttl);

void dns_edit_anum(char*result, unsigned short awnser_num);

#endif
