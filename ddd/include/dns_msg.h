#ifndef __DNS_MSG_H__
#define __DNS_MSG_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DOMAIN_TYPE 1
#define EXTEND_TYPE 2
#define VIEW_TYPE 3
#define MASK_TYPE 4

//domain&extend
#define CREATE_OPT 1
#define INSERT_OPT 2
#define REMOVE_OPT 3
#define MODIFY_OPT 4
#define RESTART_OPT 5
#define STOP_OPT 6
#define DROPALL_OPT 7
#define DROP_OPT 8
#define STOPDOMAIN_OPT 9

//view
#define VIEW_CREATE_OPT 1
#define VIEW_DELETE_OPT 2
#define VIEW_STOP_OPT 3
#define VIEW_RESTART_OPT 4
#define IP_INSERT_OPT 5
#define IP_REMOVE_OPT 6
#define IP_MODIFY_OPT 7
#define IP_RESTART_OPT 8
#define IP_STOP_OPT 9

//mask
#define MASK_CREATE_OPT 1
#define MASK_STOP_OPT 2
#define MASK_RESTART_OPT 3
#define MASK_DELETE_OPT 4


//errdst
#define ERRDST_ADD 1
#define ERRDST_DEL 2
#define ERRDST_MOD 3


//ILLREQ
#define ILLREQ_ADD 1
#define ILLREQ_DEL 2
#define ILLREQ_MOD 3



#define MGR_ANSWER_SUCCESS (0)
#define MGR_ANSWER_FAILED  (1)
#define MGR_ANSWER_NOEXIST  (2)



int get_type_from_msg(char *msg);

int get_vid_from_msg(char *msg);

int get_opt_from_msg(char *msg);

int get_domain_from_msg(char *msg, char *domain, int *len);

int get_ip_from_msg(char *msg, struct in_addr *ips, int *num);

void answer_display(void *buf);

void deal_domain_type(void* buf, char* answer);

void deal_extend_type(void* buf, char* answer);

void deal_view_type(void* buf, char* answer);

void deal_mask_type(void* buf, char* answer);

void answer_sucess(char *buf);

void answer_failure(char *buf);

int dns_msg_cache_analyze(char *msg);

int dns_msg_view_analyze(char *msg);

int dns_msg_pack_answer(char *nameclass,int opt,unsigned int msg_id,int type,int view,char *data, int result,char *msg);

int dns_msg_pack_init(char *msg);

int dns_msg_pack_register(char *msg);



#endif
