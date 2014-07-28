#ifndef __DNS_ERRDST_H__
#define __DNS_ERRDST_H__


int dns_errdst_init();

//int dns_errdst_set(struct in_addr ip);

void dns_errdst_destroy();

int dns_errdst_answer_get(char *domain, int domainlen ,int view_id,char *result);

int dns_errdst_answer_cache_get(char *domain, int domainlen ,int view_id,char *result);

void dns_errdst_check();

void dns_errdst_clear();

int dns_errdst_count();

#endif