#ifndef __DNS_LCLLOG_H__
#define __DNS_LCLLOG_H__

int dns_lcllog_init(int pnum);

void dns_lcllog_remake_path();

void dns_lcllog_general();

void dns_lcllog_answer(int pid,int view,char *domain,int server);

void dns_lcllog_errdst(int pid,int view,char *domain,int sip);

void dns_lcllog_cache_count();

void dns_lcllog_record_count();

void dns_lcllog_answer_count();

void dns_lcllog_errdst_count();

void dns_lcllog_allreq_count();

void dns_lcllog_illreq_count();

void dns_lcllog_dnsreq_count();

void dns_lcllog_view();

void dns_lcllog_cache_viewcount(int view_id);

void dns_lcllog_record_viewcount(int view_id);

void dns_lcllog_answer_viewcount(int view_id);

void dns_lcllog_errdst_viewcount(int view_id);

void dns_lcllog_dnsreq_viewcount(int view_id);

void dns_lcllog_allreq_viewcount(int view_id);

void dns_lcllog_illreq_viewcount(int view_id);

void dns_lcllog_reqcnt_log(char *domain,int dlen,int view_id,int pid);

void dns_lcllog_reqcnt();


#endif
