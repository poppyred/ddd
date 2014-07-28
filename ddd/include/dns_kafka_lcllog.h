#include "dns_kafka_log.h"

void dns_kafka_lcllog_init();
void dns_kafka_lcllog_errdst(int pid,int view,char *domain,int sip);
void dns_kafka_lcllog_answer(int pid,int view,char *domain,int server);
void dns_kafka_lcllog_request(int pid,int view,char *domain,int number);
void dns_kafka_lcllog_query_A(int pid,char*type,int view,char *domain,int number,int sip,int dip);