#ifndef __DNS_LOG_H__
#define __DNS_LOG_H__

int dns_log_init(int pthread_num, int size);

void dns_log_uninit();

void dns_log_to_file(int num);

int dns_log_cache(char *domain,int domain_len,int sip,int view_id,int pthread_id);

int dns_log_record(char *domain,int domain_len,int sip,int view_id);

int dns_log_rrmake(int type,int ttl,int ip,int offset,char *result);

int dns_log_answer(char *domain,int domain_len,char *msg, int msg_len,int sip, int svr, int view_id);


/*local test*/
void dns_test_log_answer(int pid, int sip,int dip,unsigned short qid,int qtype,char *domain);

void dns_test_log_requset(int pid, int sip,int dip,unsigned short qid,int qtype,char *domain);

void dns_test_log_illreq(int pid, int sip,int dip);



#endif
