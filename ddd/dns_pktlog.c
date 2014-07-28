#include <stdio.h>
#include <stdlib.h>

#include "libbase.h"
#include "dns_pktlog.h"
#include "dns_comdef.h"


#define MAX_MSG_SIZE 1450
#define MAX_PATH_LEN 256
#define MAX_FILE_NAME 128


typedef struct st_pktlog
{
    char msg[MAX_MSG_SIZE];
    int count;
    int offset;

}st_pktlog;


int g_pthread_num = 0;
int g_limit = 0;
uint32_t g_dst_addr = 0;
uint16_t g_dst_port = 0;

st_pktlog *g_cache_pktlog = NULL;
st_pktlog *g_record_pktlog = NULL;
st_pktlog *g_answer_pktlog = NULL;
st_pktlog *g_errdst_pktlog = NULL;
st_pktlog *g_rarp_pktlog = NULL;



extern time_t global_now;


int dns_pktlog_init(char *dstip, uint16_t port,int pnum,int limit)
{
    g_pthread_num = pnum;
    g_limit = limit;
    g_dst_addr = inet_addr(dstip);
    g_dst_port = htons(port);

    g_cache_pktlog = (st_pktlog *)h_malloc(sizeof(st_pktlog) * pnum);
    if (!g_cache_pktlog)
    {
        return -1;
    }
    
    g_record_pktlog = (st_pktlog *)h_malloc(sizeof(st_pktlog) * pnum);
    if (!g_record_pktlog)
    {
        return -1;
    }
    
    g_answer_pktlog = (st_pktlog *)h_malloc(sizeof(st_pktlog) * pnum);
    if (!g_answer_pktlog)
    {
        return -1;
    }

    g_errdst_pktlog = (st_pktlog *)h_malloc(sizeof(st_pktlog) * pnum);
    if (!g_errdst_pktlog)
    {
        return -1;
    }

    g_rarp_pktlog = (st_pktlog *)h_malloc(sizeof(st_pktlog) * pnum);
    if (!g_rarp_pktlog)
    {
        return -1;
    }
    
    return 0;
}


void dns_pktlog_uninit()
{
    if (g_answer_pktlog)
    {
        h_free(g_answer_pktlog);
        g_answer_pktlog = NULL;
    }

    if (g_record_pktlog)
    {
        h_free(g_record_pktlog);
        g_record_pktlog = NULL;
    }

    
    if (g_cache_pktlog)
    {
        h_free(g_cache_pktlog);
        g_cache_pktlog = NULL;
    }

    if (g_errdst_pktlog)
    {
        h_free(g_errdst_pktlog);
        g_errdst_pktlog = NULL;
    }

    
    if (g_rarp_pktlog)
    {
        h_free(g_rarp_pktlog);
        g_rarp_pktlog = NULL;
    }
}

void dns_pktlog_set(int new_ip)
{
    g_dst_addr = new_ip;

    hyb_debug("set pktlog ip : %s\n",inet_ntoa(*(struct in_addr *)&g_dst_addr));

}


int dns_pktlog_errdst(struct fio_nic *dst, int sip, unsigned short view, 
    char *domain,unsigned short domainlen,unsigned short pid,unsigned short sport)
{
    struct fio_txdata *txdata = NULL;


    if (g_dst_addr == 0)
    {
        return -1;
    }
    
    if (g_errdst_pktlog[pid].count == g_limit && g_errdst_pktlog[pid].offset < MAX_MSG_SIZE)
    {
        //TODO SENDPACKET
        int txcount = fio_nic_reserve_tx(dst,&txdata,NULL); 
        if (!txcount)
        {
            hyb_debug("pktlog send error!\n");
            return -1;
        }


	    //hyb_debug("[-------- send a ERRDST packet to %s dport:%d sport:%d pid:%d len:%d-------------]\n",inet_ntoa(*(struct in_addr *)&g_dst_addr),ntohs(g_dst_port),ntohs(CACHE_PORT),pid,g_cache_pktlog[pid].offset);
        memcpy(txdata->pdata, g_errdst_pktlog[pid].msg,g_errdst_pktlog[pid].offset);
	    txdata->dstip = g_dst_addr;
	    txdata->dstport = g_dst_port;
	    txdata->srcport = ERRDST_PORT;
        fio_send(dst, htons(g_errdst_pktlog[pid].offset), txdata, 0);

        g_errdst_pktlog[pid].offset = 0;
        g_errdst_pktlog[pid].count = 0;
        memset(g_errdst_pktlog[pid].msg,0,MAX_MSG_SIZE);

        
    }
    else if(g_errdst_pktlog[pid].count > g_limit)
    {
        hyb_debug("pktlog limit error!\n");
        exit(0);
        return -1;
    }

    if (g_errdst_pktlog[pid].offset >= MAX_MSG_SIZE)
    {
            hyb_debug("pktlog size error!\n");
            return -1;
    }
        
    int mark = 0;
            
    char *head = g_errdst_pktlog[pid].msg + g_errdst_pktlog[pid].offset;
    char *temp = head;
            
    memcpy(temp,&global_now,sizeof(time_t));
    mark += sizeof(time_t);
            
    temp = head + mark;
    memcpy(temp,&sip,sizeof(int));
    mark += 4;

    temp = head + mark;
    memcpy(temp,&sport,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,&view,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,&domainlen,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,domain,domainlen);
    mark += domainlen;

    head[mark] = '\0';
    mark ++;
            
    g_errdst_pktlog[pid].offset += mark;
    g_errdst_pktlog[pid].count++;
            
        

    return 0;
}







int dns_pktlog_cache(struct fio_nic *dst, int sip, unsigned short view, 
    char *domain,unsigned short domainlen,unsigned short pid,unsigned short sport)
{
    struct fio_txdata *txdata = NULL;

    if (g_dst_addr == 0)
    {
        return -1;
    }

    if (g_cache_pktlog[pid].count == g_limit && g_cache_pktlog[pid].offset < MAX_MSG_SIZE)
    {
        //TODO SENDPACKET
        int txcount = fio_nic_reserve_tx(dst,&txdata,NULL); 
        if (!txcount)
        {
            hyb_debug("pktlog send error!\n");
            return -1;
        }

	
	//    hyb_debug("[-------- send a CACHE packet to %s dport:%d sport:%d pid:%d len:%d-------------]\n",inet_ntoa(*(struct in_addr *)&g_dst_addr),ntohs(g_dst_port),ntohs(CACHE_PORT),pid,g_cache_pktlog[pid].offset);
        memcpy(txdata->pdata, g_cache_pktlog[pid].msg,g_cache_pktlog[pid].offset);
	    txdata->dstip = g_dst_addr;
	    txdata->dstport = g_dst_port;
	    txdata->srcport = CACHE_PORT;
        fio_send(dst, htons(g_cache_pktlog[pid].offset), txdata, 0);
        //fio_nic_sendmsg(dst, g_dst_addr, g_dst_port,NULL, 0,CACHE_PORT, htons(g_cache_pktlog[pid].offset), txdata, 0);
	
        g_cache_pktlog[pid].offset = 0;
        g_cache_pktlog[pid].count = 0;
        memset(g_cache_pktlog[pid].msg,0,MAX_MSG_SIZE);

        
    }
    else if(g_cache_pktlog[pid].count > g_limit)
    {
        hyb_debug("pktlog limit error!\n");
        exit(0);
        return -1;
    }

    if (g_cache_pktlog[pid].offset >= MAX_MSG_SIZE)
    {
            hyb_debug("pktlog size error!\n");
            return -1;
    }
        
    int mark = 0;
            
    char *head = g_cache_pktlog[pid].msg + g_cache_pktlog[pid].offset;
    char *temp = head;
            
    memcpy(temp,&global_now,sizeof(time_t));
    mark += sizeof(time_t);
            
    temp = head + mark;
    memcpy(temp,&sip,sizeof(int));
    mark += 4;

    temp = head + mark;
    memcpy(temp,&sport,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,&view,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,&domainlen,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,domain,domainlen);
    mark += domainlen;

    head[mark] = '\0';
    mark ++;
            
    g_cache_pktlog[pid].offset += mark;
    g_cache_pktlog[pid].count++;
            
        

    return 0;
}


int dns_pktlog_record(struct fio_nic *dst, int sip,unsigned short view, 
    char *domain,unsigned short domainlen,unsigned short pid,unsigned short sport)
{
    struct fio_txdata *txdata = NULL;

    
    if (g_dst_addr == 0)
    {
        return -1;
    }
    
    if (g_record_pktlog[pid].count == g_limit && g_record_pktlog[pid].offset < MAX_MSG_SIZE)
    {
        //TODO SENDPACKET

        //hyb_debug("[-------- send a RECORD packet to %s dport:%d sport:%d pid:%d len:%d-------------]\n",inet_ntoa(*(struct in_addr *)&g_dst_addr),ntohs(g_dst_port),ntohs(RECORD_PORT),pid,g_record_pktlog[pid].offset);
        int txcount = fio_nic_reserve_tx(dst, &txdata,NULL); 
        if (!txcount)
        {
            hyb_debug("pktlog send error!\n");
            return -1;
        }
	

        memcpy(txdata->pdata, g_record_pktlog[pid].msg,g_record_pktlog[pid].offset);
	    txdata->dstip = g_dst_addr;
        
	    txdata->dstport = g_dst_port;
	    txdata->srcport = RECORD_PORT;
        fio_send(dst, htons(g_record_pktlog[pid].offset), txdata, 0);
        //fio_nic_sendmsg(dst, g_dst_addr, g_dst_port,NULL, 0,RECORD_PORT, htons(g_record_pktlog[pid].offset), txdata, 0);
        g_record_pktlog[pid].offset = 0;
        g_record_pktlog[pid].count = 0;

        
    }
    else if(g_record_pktlog[pid].count > g_limit)
    {
        hyb_debug("pktlog limit error!\n");
        exit(0);
        return -1;
    }

    if (g_record_pktlog[pid].offset >= MAX_MSG_SIZE)
    {
        hyb_debug("pktlog size error!\n");
        return -1;
    }

    int mark = 0;
            
    char *head = g_record_pktlog[pid].msg + g_record_pktlog[pid].offset;
    char *temp = head;
            
    memcpy(temp,&global_now,sizeof(time_t));
    mark += sizeof(time_t);
            
    temp = head + mark;
    memcpy(temp,&sip,sizeof(int));
    mark += 4;

    temp = head + mark;
    memcpy(temp,&sport,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,&view,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,&domainlen,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,domain,domainlen);
    mark += domainlen;

    head[mark] = '\0';
    mark ++;
            
    g_record_pktlog[pid].offset += mark;
    g_record_pktlog[pid].count++;
            
        

    return 0;
}


int dns_pktlog_answer(struct fio_nic *dst, int sip,int svrip,unsigned short view, 
    char *domain,unsigned short domainlen,unsigned short pid,unsigned short sport,unsigned short rs_num)
{
    struct fio_txdata *txdata = NULL;

    
    if (g_dst_addr == 0)
    {
        return -1;
    }
    
    if (g_answer_pktlog[pid].count == g_limit && g_answer_pktlog[pid].offset < MAX_MSG_SIZE)
    {
        //TODO SENDPACKET
        int txcount = fio_nic_reserve_tx(dst, &txdata,NULL); 
        if (!txcount)
        {
            hyb_debug("pktlog send error!\n");
            return -1;
        }
        //hyb_debug("[-------- send a ANSWER packet to %s dport:%d sport:%d pid:%d len:%d-------------]\n",inet_ntoa(*(struct in_addr *)&g_dst_addr),ntohs(g_dst_port),ntohs(ANSWER_PORT),pid,g_answer_pktlog[pid].offset);
        memcpy(txdata->pdata, g_answer_pktlog[pid].msg,g_answer_pktlog[pid].offset);
	    txdata->dstip = g_dst_addr;
	    txdata->dstport = g_dst_port;
	    txdata->srcport = ANSWER_PORT;
        fio_send(dst, htons(g_answer_pktlog[pid].offset), txdata, 0);
        //fio_nic_sendmsg(dst, g_dst_addr, g_dst_port,NULL, 0,ANSWER_PORT, htons(g_answer_pktlog[pid].offset), txdata, 0);
        
        g_answer_pktlog[pid].offset = 0;
        g_answer_pktlog[pid].count = 0;
        memset(g_answer_pktlog[pid].msg,0,MAX_MSG_SIZE);

        
    }

    else if(g_answer_pktlog[pid].count > g_limit || g_answer_pktlog[pid].offset >= MAX_MSG_SIZE)
    {
        hyb_debug("pktlog limit error!\n");

        g_answer_pktlog[pid].offset = 0;
        g_answer_pktlog[pid].count = 0;
        memset(g_answer_pktlog[pid].msg,0,MAX_MSG_SIZE);
        return -1;
    }

    int mark = 0;
            
    char *head = g_answer_pktlog[pid].msg + g_answer_pktlog[pid].offset;
    char *temp = head;
            
    memcpy(temp,&global_now,sizeof(time_t));
    mark += sizeof(time_t);
            
    temp = head + mark;
    memcpy(temp,&sip,4);
    mark += 4;

    temp = head + mark;
    memcpy(temp,&sport,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,&svrip,4);
    mark += 4;

    temp = head + mark;
    memcpy(temp,&view,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,&domainlen,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,domain,domainlen);
    mark += domainlen;

    temp = head + mark;
    memcpy(temp,"\0",1);
    mark += 1;

    temp = head + mark;
    memcpy(temp,&rs_num,2);
    mark += 2;

    g_answer_pktlog[pid].offset += mark;
    g_answer_pktlog[pid].count++;
            
    return 0;
}



int dns_pktlog_answer_rsadd(unsigned short type,unsigned int ttl,int ip,int pid)
{
    int mark = 0;
    
    if (g_dst_addr == 0)
    {
        return -1;
    }

    if (g_answer_pktlog[pid].offset >= MAX_MSG_SIZE)
    {
        hyb_debug("pktlog size error!\n");
        g_answer_pktlog[pid].offset = 0;
        g_answer_pktlog[pid].count = 0;
        memset(g_answer_pktlog[pid].msg,0,MAX_MSG_SIZE);
        return -1;
    }
                
    char *head = g_answer_pktlog[pid].msg + g_answer_pktlog[pid].offset;
    char *temp = head;

    memcpy(temp,&type,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,&ttl,4);
    mark += 4;

    temp = head + mark;
    memcpy(temp,&ip,4);
    mark += 4;


    g_answer_pktlog[pid].offset += mark;
    return 0;
            
}


int dns_pktlog_answer_cnameadd(unsigned short type,unsigned int ttl,char *domain,unsigned short domainlen,int pid)
{
    int mark = 0;

    
    if (g_dst_addr == 0)
    {
        return -1;
    }

    if (g_answer_pktlog[pid].offset >= MAX_MSG_SIZE)
    {
        hyb_debug("pktlog size error!\n");
        return -1;
    }
                
    char *head = g_answer_pktlog[pid].msg + g_answer_pktlog[pid].offset;
    char *temp = head;

    memcpy(temp,&type,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,&ttl,4);
    mark += 4;

    temp = head + mark;
    memcpy(temp,&domainlen,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,domain,domainlen);
    mark += domainlen;

    temp = head + mark;
    memcpy(temp,"\0",1);
    mark += 1;
    
    g_answer_pktlog[pid].offset += mark;

    return 0;
            
}



int dns_pktlog_rarp(struct fio_nic *dst, int sip, unsigned short view, 
    char *domain,unsigned short domainlen,unsigned short pid,unsigned short sport)
{
    struct fio_txdata *txdata = NULL;

    
    if (g_dst_addr == 0)
    {
        return -1;
    }

    if (g_rarp_pktlog[pid].count == g_limit && g_rarp_pktlog[pid].offset < MAX_MSG_SIZE)
    {
        //TODO SENDPACKET
        int txcount = fio_nic_reserve_tx(dst,&txdata,NULL); 
        if (!txcount)
        {
            hyb_debug("pktlog send error!\n");
            return -1;
        }
        
	    //hyb_debug("[-------- send a REVERSE packet to %s dport:%d sport:%d pid:%d len:%d-------------]\n",inet_ntoa(*(struct in_addr *)&g_dst_addr),ntohs(g_dst_port),ntohs(CACHE_PORT),pid,g_cache_pktlog[pid].offset);
        memcpy(txdata->pdata, g_rarp_pktlog[pid].msg,g_rarp_pktlog[pid].offset);
	    txdata->dstip = g_dst_addr;
	    txdata->dstport = g_dst_port;
	    txdata->srcport = RARP_PORT;
        fio_send(dst, htons(g_rarp_pktlog[pid].offset), txdata, 0);
        //fio_nic_sendmsg(dst, g_dst_addr, g_dst_port, NULL, 0,RARP_PORT, htons(g_rarp_pktlog[pid].offset), txdata, 0);
        
        g_rarp_pktlog[pid].offset = 0;
        g_rarp_pktlog[pid].count = 0;
        memset(g_rarp_pktlog[pid].msg,0,MAX_MSG_SIZE);

        
    }
    else if(g_rarp_pktlog[pid].count > g_limit)
    {
        hyb_debug("pktlog limit error!\n");
        exit(0);
        return -1;
    }

    if (g_rarp_pktlog[pid].offset >= MAX_MSG_SIZE)
    {
            hyb_debug("pktlog size error!\n");
            return -1;
    }
        
    int mark = 0;
            
    char *head = g_rarp_pktlog[pid].msg + g_rarp_pktlog[pid].offset;
    char *temp = head;
            
    memcpy(temp,&global_now,sizeof(time_t));
    mark += sizeof(time_t);
            
    temp = head + mark;
    memcpy(temp,&sip,sizeof(int));
    mark += 4;

    temp = head + mark;
    memcpy(temp,&sport,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,&view,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,&domainlen,2);
    mark += 2;

    temp = head + mark;
    memcpy(temp,domain,domainlen);
    mark += domainlen;

    head[mark] = '\0';
    mark ++;
            
    g_rarp_pktlog[pid].offset += mark;
    g_rarp_pktlog[pid].count++;
            
        

    return 0;
}


/*----------------------------------------------------------------------------*/

