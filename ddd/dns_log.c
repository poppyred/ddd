#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>

#include "libbase.h"
#include "dns_log.h"
#include "dns_comdef.h"

#define MAX_PTHREAD_NUM (24)
#define MAX_HASH_SPCES  (1<<16)
#define MAX_DOMAIN_LEN  (255)
#define MAX_MSG_LEN     (512)
#define MAX_IP_LEN  (16)

#define CACHE_LOG   (1)
#define RECORD_LOG  (2)
#define ANSWER_LOG  (3)

extern time_t global_now;

extern struct tm *g_tm;


h_ring_st *g_log_ring= NULL;

h_mem_pool_st *g_log_pool[MAX_PTHREAD_NUM];


typedef struct st_dns_log
{
    char domain[MAX_DOMAIN_LEN];
    int src_ip;
    int svr_ip;
    int view;
    int type;
    int pid;
    int msg_len;
    char msg[0];
    
}st_dns_log;


void log_table_destroy(void *data)
{
    st_dns_log *temp = (st_dns_log *)data;
    h_free(temp);
    
}


void dns_log_uninit()
{
    
    h_ring_destroy(g_log_ring);
    
}


int dns_log_init(int pthread_num, int size)
{
    int i = 0;
    if (g_log_ring)
    {
        goto FAILED;
    }

    g_log_ring = h_ring_create("dns_log",pthread_num*size,WNS_RING_F_SC_EN);
    if (!g_log_ring)
    {
        goto FAILED;
    }

    for (i = 0; i<MAX_PTHREAD_NUM; i++)
    {
        char name[5];
        sprintf_n(name,5,"%d",i);
        g_log_pool[i] = h_mem_pool_create(name, 65536*16, sizeof(st_dns_log), 0);
        if (!g_log_pool[i])
        {
            hyb_debug("h_mem_pool_create failed!\n");
            goto FAILED;
        }
        memset(name,0,5);
    }

    return 0;
    

FAILED:
    return -1;
    
}

/*
static int getkey(char *domain ,int ip,char *key)
{
    int len = strlen(domain);
    memcpy(key,domain,len);
    key = key+len;
    memcpy(key,ip,4);

    return len+4;
    
}
*/



int dns_log_cache(char *domain,int domain_len,int sip,int view_id,int pthread_id)
{
    if (!domain || !g_log_ring)
    {
        hyb_debug("dns_log_cache failed!\n");
        return -1;
    }

    //st_dns_log *temp = NULL;
    
   // st_dns_log *temp = (st_dns_log *)h_malloc(sizeof(st_dns_log));
    st_dns_log *temp = (st_dns_log *)h_mem_pool_get(g_log_pool[pthread_id]);
    if (!temp)
    {
        //hyb_debug("h_mem_pool_get failed!\n");
        return -1;
    }

    memset(temp,0,sizeof(st_dns_log));
    temp->src_ip = sip;
    temp->type = CACHE_LOG;
    temp->view = view_id;
    temp->pid = pthread_id;
    memcpy(temp->domain,domain,domain_len);

    
    if (h_ring_enqueue(g_log_ring,temp))
    {
        hyb_debug("h_ring_enqueue failed!\n");
        goto FAILED;
    }
    

    return 0;

FAILED:

    if (temp)
    {
        hyb_debug("failed here!\n");
        h_mem_pool_put(g_log_pool[pthread_id],temp,0);
    }

    return -1;
        
}


int dns_log_record(char *domain,int domain_len,int sip,int view_id)
{
    if (!domain || !g_log_ring)
    {
        hyb_debug("dns_log_cache failed!\n");
        return -1;
    }

    st_dns_log *temp = (st_dns_log *)h_malloc(sizeof(st_dns_log));
    if (!temp)
    {
        return -1;
    }
    
    temp->src_ip = sip;
    temp->type = RECORD_LOG;
    temp->view = view_id;
    memcpy(temp->domain,domain,domain_len);

    if (h_ring_enqueue(g_log_ring,temp))
    {
        hyb_debug("h_ring_enqueue failed!\n");
        goto FAILED;
    }


    return 0;

FAILED:

    if (temp)
        h_free(temp);

    return -1;
    
        
}


#define RR_LOG_FORMAT "%d-%d-%s|"


int dns_log_rrmake(int type,int ttl,int ip,int offset,char *result)
{
    if (!result)
    {
        return -1;
    }

    char *temp = result;

    temp = temp+offset;
    int len = sprintf_n(temp,50,RR_LOG_FORMAT,type,ttl,inet_ntoa(*(struct in_addr *)&ip));

    return len+offset;
}


int dns_log_answer(char *domain,int domain_len,char *msg, int msg_len,int sip, int svr, int view_id)
{
    if (!domain || !g_log_ring)
    {
        hyb_debug("dns_log_cache failed!\n");
        return -1;
    }

    st_dns_log *temp = (st_dns_log *)h_malloc(sizeof(st_dns_log) + msg_len);
    if (!temp)
    {
        return -1;
    }
    
    temp->svr_ip = svr;
    temp->src_ip = sip;
    temp->type = ANSWER_LOG;
    temp->view = view_id;
    temp->msg_len = msg_len;
   
    memcpy(temp->domain,domain,domain_len);
    memcpy(temp->msg,msg,msg_len);

    if (h_ring_enqueue(g_log_ring,temp))
    {
        hyb_debug("h_ring_enqueue failed!\n");
        goto FAILED;
    }


    return 0;

FAILED:

    if (temp)
        h_free(temp);

    return -1;
    
        
}


#define CACHE_LOG_FORMAT "%04d-%02d-%02d-%02d-%02d-%02d\tCA\t%s\t%d\t%s\tpid:%d\n"
#define RECORD_LOG_FORMAT "%04d-%02d-%02d-%02d-%02d-%02d\tJC\t%s\t%d\t%s\n"
#define ANSWER_LOG_FORMAT "%04d-%02d-%02d-%02d-%02d-%02d\tAN\t%s\t%d\t%s\t%s\t%s\n"



static void log_file_cache(st_dns_log *data, FILE *fp, char *msg,int pid)
{

    sprintf_n(msg, MAX_MSG_LEN, CACHE_LOG_FORMAT , g_tm->tm_year+1900, g_tm->tm_mon+1,g_tm->tm_mday,
                g_tm->tm_hour,g_tm->tm_min, g_tm->tm_sec,inet_ntoa(*(struct in_addr *)&data->src_ip),
                data->view,data->domain,pid);

    fwrite(msg,strlen(msg),1,fp);
}

static void log_file_record(st_dns_log *data, FILE *fp, char *msg)
{

    sprintf_n(msg, MAX_MSG_LEN, RECORD_LOG_FORMAT , g_tm->tm_year+1900, g_tm->tm_mon+1,g_tm->tm_mday,
                g_tm->tm_hour,g_tm->tm_min, g_tm->tm_sec,inet_ntoa(*(struct in_addr *)&data->src_ip),
                data->view,data->domain);

    fwrite(msg,strlen(msg),1,fp);
}


static void log_file_answer(st_dns_log *data, FILE *fp, char *msg)
{

    sprintf_n(msg, MAX_MSG_LEN, ANSWER_LOG_FORMAT , g_tm->tm_year+1900, g_tm->tm_mon+1,g_tm->tm_mday,
                g_tm->tm_hour,g_tm->tm_min, g_tm->tm_sec,inet_ntoa(*(struct in_addr *)&data->src_ip),
                data->view,data->domain,inet_ntoa(*(struct in_addr *)&data->svr_ip),data->msg);

    fwrite(msg,strlen(msg),1,fp);
}



void dns_log_to_file(int num)
{
    st_dns_log *temp = NULL;
    FILE *fp =NULL;
    char msg[MAX_MSG_LEN] = {0};

    fp = fopen("./log/dnsproxy.log","a+");
    if (!fp)
    {
        hyb_debug("log file open failed!\n");
    }

    while(num > 0 && (h_ring_dequeue(g_log_ring,(void **)&temp) == 0))
    {
        memset(msg,0,MAX_MSG_LEN);
        switch(temp->type)
        {
        case CACHE_LOG:
            log_file_cache(temp, fp, msg,temp->pid);
            break;
        case RECORD_LOG:
            log_file_record(temp, fp, msg);
            break;
        case ANSWER_LOG:
            log_file_answer(temp, fp, msg);
            break;
        default:
            break;
        }
        //hyb_debug("h_mem_pool_put:%d\n",temp->pid);
        h_mem_pool_put(g_log_pool[temp->pid],temp,temp->pid);

        temp = NULL;
        num--;
    }

    fclose(fp);

    //hyb_debug("log file closed!\n");
}



/*---------------------------------------------------------------------------*/


#define REQUSET_LOG_FORMAT "%s\t%s\t%s\t%d\t%d\t%s\n"
#define ILLREQ_LOG_FORMAT "%s\t%s\t%s\n"


void dns_test_log_requset(int pid, int sip,int dip,unsigned short qid,int qtype,char *domain)
{
    /*
    if (qtype == 0x001c)
    {
        return;
    }
    */
    
    if (sip == -652826432 || dip == -652826432)
    {
        return;
    }
    
    FILE *fp = NULL;
    char path[64] = {0};
    char date[64] = {0};
    char s_ip[20] = {0};
    char d_ip[20] = {0};
    char data[512] = {0};

    sprintf_n(path,64,"./test/requesrt_%02d",pid);
    fp = fopen(path,"a+");


    sprintf_n(date, 64, "%04d-%02d-%02d %02d:%02d:%02d", g_tm->tm_year+1900, g_tm->tm_mon+1,g_tm->tm_mday,
                g_tm->tm_hour,g_tm->tm_min, g_tm->tm_sec);

    
    strcpy_n(s_ip,20,inet_ntoa(*(struct in_addr *)&sip));
    strcpy_n(d_ip,20,inet_ntoa(*(struct in_addr *)&dip));
    
    sprintf_n(data,512,REQUSET_LOG_FORMAT,date,s_ip,
        d_ip,qid,qtype,domain);

    fwrite(data,strlen(data),1,fp);

    fclose(fp);

    
}


void dns_test_log_answer(int pid, int sip,int dip,unsigned short qid,int qtype,char *domain)
{
    /*
    if (qtype == 0x001c)
    {
        return;
    }*/

    if (sip == -652826432 || dip == -652826432)
    {
        return;
    }
    
    FILE *fp = NULL;
    char path[64] = {0};
    char date[64] = {0};
    char s_ip[20] = {0};
    char d_ip[20] = {0};
    char data[512] = {0};

    sprintf_n(path,64,"./test/answer_%02d",pid);
    fp = fopen(path,"a+");
    
    sprintf_n(date, 64, "%04d-%02d-%02d %02d:%02d:%02d", g_tm->tm_year+1900, g_tm->tm_mon+1,g_tm->tm_mday,
                g_tm->tm_hour,g_tm->tm_min, g_tm->tm_sec);

    strcpy_n(s_ip,20,inet_ntoa(*(struct in_addr *)&sip));
    strcpy_n(d_ip,20,inet_ntoa(*(struct in_addr *)&dip));

    
    sprintf_n(data,512,REQUSET_LOG_FORMAT,date,s_ip,
        d_ip ,qid,qtype,domain);

    fwrite(data,strlen(data),1,fp);

    fclose(fp);

    
}


void dns_test_log_illreq(int pid, int sip,int dip)
{
    /*
    if (qtype == 0x001c)
    {
        return;
    }*/

    FILE *fp = NULL;
    char path[64] = {0};
    char date[64] = {0};
    char s_ip[20] = {0};
    char d_ip[20] = {0};
    char data[512] = {0};

    sprintf_n(path,64,"./test/illreq_%02d",pid);
    fp = fopen(path,"a+");
    
    sprintf_n(date, 64, "%04d-%02d-%02d %02d:%02d:%02d", g_tm->tm_year+1900, g_tm->tm_mon+1,g_tm->tm_mday,
                g_tm->tm_hour,g_tm->tm_min, g_tm->tm_sec);

    strcpy_n(s_ip,20,inet_ntoa(*(struct in_addr *)&sip));
    strcpy_n(d_ip,20,inet_ntoa(*(struct in_addr *)&dip));

    
    sprintf_n(data,512,ILLREQ_LOG_FORMAT,date,s_ip,d_ip);

    fwrite(data,strlen(data),1,fp);

    fclose(fp);

    
}



