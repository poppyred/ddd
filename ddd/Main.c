#define _GNU_SOURCE
#include "Main.h"

//#define	_DISPLAY_TABLE 1

#include <time.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>

#include "dns_cache.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "dns_port.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "dns_extche.h"
#include "dns_errdst.h"
#include "dns_msg.h"
#include "dns_domain.h"
#include "dns_extend.h"
#include "dns_view.h"
#include "dns_mask.h"
#include "dns_log.h"
#include "dns_pktlog.h"
#include "dns_comdef.h"
#include "dns_lcllog.h"
#include "efly_ev.h"
#include "fio.h"
#include "libbase.h"
#include "dns_ptr.h"
#include "dns_syslog.h"
#include "dns_verify.h"
#include "dns_pack.h"
#include "dns_pkt.h"



#define CORE_IP_ADDR "121.201.11.3"

static int dnsheadsize = sizeof(dnsheader);
time_t global_now = 0; //全局时间，每秒+1s
int g_mday = 0;
struct tm *g_tm = NULL;




char *dns_ip; //主DNS地址
char *bind_ip; //绑定公网地址
char *response_ip;

efly_ipc_svr *g_ipc_svr = NULL;

int g_answer_sockfd = 0;
struct sockaddr_in g_answer_addr;  //应答目标addr


static int dns_config_init();

static int dns_table_init();

static int dns_record_answer_check(struct fio_nic *t,struct fio_nic *log_t, struct fio_txdata *txdata,
		char *domain,int domainlen,int view_id,struct sockaddr_in *from, unsigned short qid,uint32_t dip,
		struct ether_addr smac);



static int answer_to_cache(char*domain, unsigned int view_id, char*answer, int answer_len,
        unsigned short type);



static int dns_cache_answer_check(struct fio_nic *src,struct fio_txdata *txdata, 
        char* domain, int len, int view_id,unsigned short type, 
        struct sockaddr_in *client,ushort qid, uint32_t src_dip, 
        struct ether_addr smac);


static int dns_errdst_answer_check(struct fio_nic *t,struct fio_nic *log_t, struct fio_txdata *txdata,
		char *domain,int domainlen,int view_id,struct sockaddr_in *from, unsigned short qid,uint32_t dip,
		struct ether_addr smac);


static void send_to_core(struct fio_nic *t, struct fio_txdata *txdata,
		ushort recvport, char*recvmsg, int recvlen, ushort qId, 
		struct sockaddr *ipaddr);


static int recv_request_analyze(char*recvmsg,int num,char*domain,int * len);

static void init_to_mgr();

static void register_to_mgr();


#ifdef _DISPLAY_TABLE
static void display_table();
#endif



/**
 * @brief   劫持记录查询
 * @param   domain       域名
 * @param   domain_len   域名长度
 * @param   view_id   
 *
 * @return 
 * @remark null
 * @see     
 * @author hyb      @date 2013/08/23
 **/
static int dns_record_get(char *packet,char *domain, int domain_len,ushort view_id)
{
	int packet_len;

	packet_len = dns_domain_get(packet,domain,domain_len,view_id);
	if(packet_len > 0)
	{
		return packet_len;
	}

	packet_len = dns_extend_get(packet,domain,domain_len,view_id);
	return packet_len;
}

int g_num = 0;



static void dns_deal_ptr_answer(struct fio_nic *src,struct fio_rxdata *rxdata,
        struct fio_txdata *txdata,int pktlen,char *domain,int domainlen,
        struct sockaddr_in *client)
{   
    if (fio_nic_is_myarpa(src, domain))
    {
		int len = make_ptr_answer(txdata->pdata, pktlen, domain, domainlen);
		txdata->dmac = &rxdata->smac;
		txdata->dstip = client->sin_addr.s_addr;
		txdata->dstport = client->sin_port;
		txdata->srcip = rxdata->dip;
		txdata->srcport = htons(53);
		fio_send(src, htons(len), txdata, 0);

        hyb_debug("Rarp Answer =>[%s] to user[%s:%d]\n",domain,inet_ntoa(*(struct in_addr *)&client->sin_addr.s_addr),ntohs(client->sin_port));
        dns_rsyslog("Rarp Answer =>[%s] to user[%s:%d]",domain,inet_ntoa(*(struct in_addr *)&client->sin_addr.s_addr),ntohs(client->sin_port));

    }
    
}



#define EDNS_PROTO  (41)

int dns_edns_unpack(char *msg,int msglen)
{
    int mask_ip = 0;
    st_dns_info pkt = {0};
    int ret = dns_unpack(msg,msglen,&pkt);
    if (ret)
    {
        return -1;
    }


    
    int locate = pkt.head->add_count;
    unsigned short type = pkt.additional[locate -1].Type;
    int rdlen = pkt.additional[locate -1].Length;
    char *rdata = pkt.additional[locate -1].data;

   // hyb_debug("loca")
    if (type != EDNS_PROTO || rdlen <= 8)
    {
        return -1;
           
    }

    rdata += 8;

    memcpy(&mask_ip,rdata,rdlen-8);

    
    hyb_debug("Edns ip:%s\n",inet_ntoa(*(struct in_addr *)&mask_ip));

    return mask_ip;
}



static void 
answer_to_client(struct fio_nic *src, struct fio_rxdata *rxdata, int recvlen,
        int q_type,ushort qurey_id,struct sockaddr_in *client, char *domain, 
        int domainlen,int view_id)
{
    struct fio_txdata *txdata = NULL;

    /*避免多次拷贝，先直接申请包内存空间*/
	int txcount = fio_nic_reserve_tx(src,&txdata,rxdata);
    if (unlikely(txcount == 0))
	{
		return;
	}

    /*反向解析处理*/
    if(q_type == 0x000c)
	{
        dns_deal_ptr_answer(src,rxdata,txdata,recvlen,domain,domainlen,client);
        return;
    }
		
    /*view_id= 1 为默认视图*/
    if(likely(view_id > 0 && view_id < MAX_VIEM_NUM)) //在用户ip视图中找到
    {
        #if 0 
        /*检查是否被劫持*/
        if (unlikely(dns_record_answer_check(src,out,txdata,domain,domainlen,view_id,&client,qurey_id,rxdata->dip,rxdata->smac) == 0))
        {
            return;
	    }

           
        /*检查是否在错误记录*/       
		if (unlikely(dns_errdst_answer_check(src,out,txdata,domain,domainlen,view_id,&client,qurey_id,rxdata->dip,rxdata->smac) == 0))
		{
			return;
		}
        #endif



        /*检查是否已缓存*/
        int ret = dns_cache_answer_check(src, txdata, domain, domainlen, view_id, 
                q_type,client,qurey_id,rxdata->dip,rxdata->smac);
        if (ret)
        {
            /*找不到*/
            if (q_type == 0x001c)
            {
                txdata->dmac = &rxdata->smac;
			    txdata->dstip = rxdata->sip;
			    txdata->dstport = rxdata->sport;
		        txdata->srcip = rxdata->dip;
		        txdata->srcport = htons(53);
                dns_edit_logo(txdata->pdata,0x8500);
			    fio_send(src, htons(recvlen), txdata, 1);
            
                return;
            }
        }

    }
}



/**
 * @brief   处理DNS请求
 * @param   t        上下文
 * @param   rxdata   请求包
 * @param   sip      请求ip 
 * @param   sport    请求端口
 *
 * @return 
 * @remark null
 * @see     
 * @author hyb      @date 2013/08/12
 **/
void handle_query_msg(struct fio_nic *src, struct fio_nic *in, struct fio_nic *out, struct fio_rxdata *rxdata)
{

	struct sockaddr_in client = {0};
	char domain[MAX_DOMAIN_LEN] = {0};
    int domainlen = 0;

	char *recvmsg = rxdata->pbuf + UDP_HEAD_LEN;
	int recvlen = rxdata->size - UDP_HEAD_LEN;

	client.sin_addr.s_addr = rxdata->sip;
	client.sin_port = rxdata->sport;
	client.sin_family = AF_INET;
    
    struct MAIN_DNS_HEADER*head = (struct MAIN_DNS_HEADER*)recvmsg; 
    int qcount = ntohs(head->q_count);
	int addcount = ntohs(head->add_count);

	int q_type = recv_request_analyze(recvmsg, recvlen, domain, &domainlen);
	if(unlikely(q_type <= 0))
	{
        
        /*local count*/
        dns_lcllog_illreq_count();
        //dns_lcllog_illreq_viewcount(view_id);
		return;
	}

    ushort q_id = *((ushort *)recvmsg);

    if (addcount == 0 && qcount == 1)
    {     
        /*正常DNS请求处理*/
        
        /*查找用户对应视图*/
        int view_id = dns_mask_get_view(&client.sin_addr); 
        
        view_id = 2;//***********test!!!!**************//

	    /*local count*/
	    dns_lcllog_allreq_count();
	    dns_lcllog_dnsreq_count();
        dns_lcllog_allreq_viewcount(view_id); 
        dns_lcllog_dnsreq_viewcount(view_id);


        answer_to_client(src, rxdata, recvlen, q_type, q_id, &client, domain, domainlen,view_id);

    }
    
    else if (addcount== 1 && qcount == 1)
    {    
        /*EDNS请求处理*/

        int client_ip = dns_edns_unpack(recvmsg,recvlen);
        if (client_ip != 0)
        {                 
             /*查找用户对应视图*/
            int view_id = dns_mask_get_view((struct in_addr *)&client_ip); 
                
	        /*local count*/
	        dns_lcllog_allreq_count();
  	        dns_lcllog_dnsreq_count();
            dns_lcllog_allreq_viewcount(view_id); 
            dns_lcllog_dnsreq_viewcount(view_id);

            answer_to_client(src, rxdata, recvlen, q_type, q_id, &client, domain, domainlen,view_id);

        }
        
    }

	return;
}



/*----------------------------------------------------------------------------*/




/**
 * @brief   检查是否通过缓存应答
 * @param   t            上下文
 * @param   txdata       发包句柄
 * @param   domain       域名
 * @param   len          域名长度
 * @param   view_id      视图id
 * @param   client       客户IP信息
 * @param   qid          请求包qid
 *
 * @return 
 * @remark null
 * @see     
 * @author hyb      @date 2013/08/23
 **/
static int dns_cache_answer_check(struct fio_nic *src,struct fio_txdata *txdata, 
        char* domain, int len, int view_id,unsigned short type, 
        struct sockaddr_in *client,ushort qid, uint32_t src_dip, struct ether_addr smac)
{

	int size = 0;


	struct sockaddr_in *dst = (struct sockaddr_in *)client;
	size = dns_cache_get(txdata->pdata,domain,len,view_id,type);
	if (likely(size > 0))
	{

		memcpy(txdata->pdata, &qid , 2);
		txdata->dmac = &smac;
		txdata->dstip = dst->sin_addr.s_addr;
		txdata->dstport = dst->sin_port;
		txdata->srcip = src_dip;
		txdata->srcport = htons(53);
		fio_send(src, htons(size), txdata, 0);


		/*local count*/
		dns_lcllog_cache_count();
		dns_lcllog_cache_viewcount(view_id);

		/*日志接口*/
		//dns_pktlog_cache(log_t,dst->sin_addr.s_addr,view_id,domain,len,NIC_EXTRA_CONTEXT(log_t)->me,dst->sin_port);

        hyb_debug("Cache Answer [view:%d]=>[%s] to user[%s:%d]\n",view_id,domain,inet_ntoa(*(struct in_addr *)&dst->sin_addr.s_addr),ntohs(dst->sin_port));
        dns_rsyslog("Cache Answer [view:%d]=>[%s] to user[%s:%d]",view_id,domain,inet_ntoa(*(struct in_addr *)&dst->sin_addr.s_addr),ntohs(dst->sin_port));

        dns_lcllog_reqcnt_log(domain,len,view_id,NIC_EXTRA_CONTEXT(src)->me);
		return 0;

	}

    
    size = dns_ext_cache_get(txdata->pdata,domain,len,view_id,type,NIC_EXTRA_CONTEXT(src)->me);
	if (likely(size > 0))
	{
		memcpy(txdata->pdata, &qid , 2);
		txdata->dmac = &smac;
		txdata->dstip = dst->sin_addr.s_addr;
		txdata->dstport = dst->sin_port;
		txdata->srcip = src_dip;
		txdata->srcport = htons(53);
		fio_send(src, htons(size), txdata, 0);


		/*local count*/
		dns_lcllog_cache_count();
		dns_lcllog_cache_viewcount(view_id);

		/*日志接口*/
		//dns_pktlog_cache(log_t,dst->sin_addr.s_addr,view_id,domain,len,NIC_EXTRA_CONTEXT(log_t)->me,dst->sin_port);

        hyb_debug("Extche Answer [view:%d]=>[%s] to user[%s:%d]\n",view_id,domain,inet_ntoa(*(struct in_addr *)&dst->sin_addr.s_addr),ntohs(dst->sin_port));
        dns_rsyslog("Extche Answer [view:%d]=>[%s] to user[%s:%d]",view_id,domain,inet_ntoa(*(struct in_addr *)&dst->sin_addr.s_addr),ntohs(dst->sin_port));

		return 0;

	}
    
	return -1;
	

}


#define CANME_TYPE 5
#define DNAME_TYPE 39
#define A_TYPE     1

static int answer_to_cache(char*domain, unsigned int view_id, char*answer, int answer_len,
        unsigned short type)
{
    int ret = 0;
	int domainlen = strlen(domain);

    
    if (*domain == '*')
    {
        //hyb_debug("dns_ext_cache_set:%s type:%d view:%d\n",domain,type,view_id);
        ret = dns_ext_cache_set(domain,domainlen,view_id,answer,answer_len,type);
    }
    else
    {
        //hyb_debug("dns_cache_set:%s type:%d view:%d\n ",domain,type,view_id);
        ret = dns_cache_set(domain,domainlen,view_id,answer,answer_len,type);
    }
	return ret;

 
    
}



/**
 * @brief   请求包解包
 * @param   recvmsg   
 * @param   num   
 * @param   domain   
 * @param   len   
 *
 * @return 
 * @remark null
 * @see     
 * @author hyb      @date 2013/08/23
 **/
static int recv_request_analyze(char*recvmsg, int num, char*domain, int* len)
{
	if (num > dnsheadsize && num < 268) 
	{

		recvmsg[num] = '\0';
		ushort q_type;  //请求类型

		char *ptr = recvmsg + dnsheadsize;
		char *cpy = domain;
		int c,count = dnsheadsize;
		int domainpath = 0;
		while(count < num)
		{
			c = *ptr;
			if(c < 64 && c > 0)
			{
				memcpy(cpy, ptr+1, c);
				memcpy(cpy+c, ".", 1);
				cpy = cpy+c+1;
				ptr += (c+1);
				count += (c+1);
				domainpath++;
			}
			else
			{
				break;
			}
		}
		if(c == '\0' && domainpath>0) //正确的DNS请求包
		{
			int size = count-dnsheadsize-1;
			domain[size] = 0;
			*len = size;
			memcpy(&q_type, ptr+1, 2);
			int temp = domain_toLowerCase(domain,size);
			if(temp < 0)
			{
				return -1;
			}
			else
			{
				return ntohs(q_type);
			}
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}

}




/**
 * @brief   应答包解包
 * @param   recvmsg   
 * @param   num   
 * @param   domain   
 *
 * @return 
 * @remark null
 * @see     
 * @author hyb      @date 2013/08/23
 **/
int recv_answer_analyze(char*recvmsg,int num, char*domain)
{
	int len = 0;
	if (num > dnsheadsize) 
	{    	
		recvmsg[num] = '\0';
		ushort q_type;  //请求类型

		char *ptr = recvmsg + dnsheadsize;
		char *cpy = domain;
		int c,count = dnsheadsize;
		int domainpath = 0;
		while(count < num)
		{
			c = *ptr;
			if(c < 64 && c > 0)
			{
				memcpy(cpy, ptr+1, c);
				memcpy(cpy+c, ".", 1);
				cpy = cpy+c+1;
				ptr += (c+1);
				count += (c+1);
				domainpath++;
			}
			else
			{
				break;
			}
		}
		if(c == '\0' && domainpath>0) //正确的DNS请求包
		{
			int size = count-dnsheadsize-1;
			domain[size] = 0;
			len = size;
			memcpy(&q_type, ptr+1, 2);
			domain_toLowerCase(domain,size);
			return len;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}

}




/**
 * @brief   发送到公网DNS服务器
 * @param   t            上下文
 * @param   txdata       发送句柄
 * @param   recvport     源端口
 * @param   recvmsg      包内容
 * @param   recvlen      包长度
 * @param   qId          新的QID
 * @param   ipaddr       服务器IP信息
 *
 * @return 
 * @remark null
 * @see     
 * @author hyb      @date 2013/08/23
 **/
static void send_to_core(struct fio_nic *t, struct fio_txdata *txdata,
		ushort recvport,char*recvmsg,int recvlen,ushort qId,
		struct sockaddr *ipaddr)
{
	assert(txdata);
	//struct sockaddr_in *ptr = (struct sockaddr_in *)ipaddr;
	//hyb_debug("[request send to ip:%s]\n",inet_ntoa(*(struct in_addr *)&ptr->sin_addr.s_addr));

	struct sockaddr_in *dst = NULL;
	dst = (struct sockaddr_in *)ipaddr;
	memcpy(txdata->pdata, recvmsg, recvlen);
	memcpy(txdata->pdata, &qId , 2);
	txdata->dstip = dst->sin_addr.s_addr;
	txdata->dstport = dst->sin_port;
	txdata->srcport = htons(recvport);
	fio_send(t, htons(recvlen), txdata, 1);
	//fio_nic_sendmsg(t, dst->sin_addr.s_addr, dst->sin_port, NULL,0, htons(recvport), htons(recvlen), txdata, 1);

	//g_num ++;
}


/**
 * @brief   非DNS请求发送到指定ip
 * @param   t            上下文
 * @param   txdata       发送句柄
 * @param   recvport     源端口
 * @param   recvmsg      包内容
 * @param   recvlen      包长度
 * @param   qId          新的QID
 * @param   ipaddr       服务器IP信息
 *
 * @return 
 * @remark null
 * @see     
 * @author hyb      @date 2013/08/23
 **/
#if 0
static void illreq_ref_to_server(struct fio_nic *src, struct fio_rxdata *rxdata,int recvlen)
{
    #if 0
    struct fio_txdata *txdata = NULL;
    struct fio_nic *tar = NULL;
    tar = fio_nic_fromip_nic(src, "eth4");
    if (!tar)
    {
        hyb_debug("fio_nic_fromip failed!\n");
        return;
    }
    
    int txcount = fio_nic_reserve_tx(tar,&txdata,rxdata);
    if (unlikely(txcount == 0) || !txdata)
    {
        return;
    }
    int nip = inet_addr("10.11.253.115");
    txdata->dmac = fio_mac_byip(tar, nip);
    if (txdata->dmac)
    {
        /*fill here*/
	    txdata->srcip = rxdata->sip;
        txdata->dstip = rxdata->dip;
	    txdata->dstport = rxdata->dport;
	    txdata->srcport = rxdata->sport;
	    fio_nic_sendmsg_(tar, htons(recvlen), txdata, 0);
    }
    #endif

    fio_stack_send_up(src,rxdata->pbuf,rxdata->size);

}

#endif




/**
 * @brief   处理DNS应答
 * @param   t        上下文
 * @param   rxdata   应答包
 * @param   recvport 接收端口
 *
 * @return 
 * @remark null
 * @see     
 * @author hyb      @date 2013/08/12
 **/

void handle_answer_msg(struct fio_nic *src, struct fio_nic *in, struct fio_nic *out,struct fio_rxdata *rxdata)
{

	char domain[MAX_DOMAIN_LEN] = {0};
	char *recvmsg = NULL;
	//struct fio_txdata *txdata = NULL;

	int recvlen = 0;
    
    unsigned short view_id = 0;
    unsigned short type;

	recvmsg = rxdata->pbuf + UDP_HEAD_LEN;
	recvlen = rxdata->size - UDP_HEAD_LEN;
    st_dns_info pkt = {0};

    if (recvlen > 12 && recvlen < MAX_DATA_SIZE) 
	{

        int ret = dns_unpack(recvmsg,recvlen,&pkt);
        if(ret)
        {
            hyb_debug("receive nondns packet!\n");
        }
        else
        {
            strcpy_n(domain,MAX_DOMAIN_LEN,pkt.question->Domain);       
            if (pkt.head->q_count && pkt.head->add_count)
            {
                type = pkt.question->Type;

                int locate = pkt.head->add_count;
                view_id = pkt.additional[locate -1].Class;

                /*处理A记录*/
                struct MAIN_DNS_HEADER*head = (struct MAIN_DNS_HEADER*)recvmsg; 
                head->add_count = htons(ntohs(head->add_count) -1);

                //hyb_debug("%s:%d\n",domain,ntohs(head->add_count));
                recvlen = recvlen - 11;
                if (head->rcode == 0)
                {
                    answer_to_cache(domain,view_id,recvmsg,recvlen,type);  
                    answer_to_mgr("dns_reply",CACHE_OPTION_REF,type,view_id,domain,MGR_ANSWER_SUCCESS);
                }
                else
                {
                    answer_to_mgr("dns_reply",CACHE_OPTION_REF,type,view_id,domain,MGR_ANSWER_NOEXIST);
                }

            }

            dns_unpack_clear(&pkt);
        }
    }
    
    #if 0

	if (recvlen > 12 && recvlen < MAX_DATA_SIZE) 
	{
		if (recv_answer_analyze(recvmsg, recvlen, domain) >= 0)
		{
			id = *((ushort *)recvmsg);


					struct MAIN_DNS_HEADER*head = (struct MAIN_DNS_HEADER*)recvmsg; 
					int qcount = ntohs(head->q_count);
					int acount = ntohs(head->ans_count);


					/*local log*/
					//dns_lcllog_answer(NIC_EXTRA_CONTEXT(src)->me,query->view_id,domain,rxdata->sip);
					//dns_kafka_lcllog_answer(NIC_EXTRA_CONTEXT(src)->me,query->view_id,domain,rxdata->sip);
					

					if(qcount == 1 && acount == 0)
					{
                        /*重定向处理*/
                        #if 0

						txcount = fio_nic_reserve_tx(tar, &txdata,rxdata);
						if (txcount)
						{
							int err_size = dns_errdst_answer_get(domain,strlen(domain),query->view_id,txdata->pdata);
							if (err_size > 0)
							{
								memcpy(txdata->pdata, &query->clientid, 2); //还原ID号

								//hyb_debug("errdst msg to %s port:%d id:%d\n",inet_ntoa(*(struct in_addr *)&query->srcip.sin_addr.s_addr),ntohs(query->srcip.sin_port),ntohs(query->clientid));

								txdata->dmac = &query->smac;
								txdata->dstip = query->srcip.sin_addr.s_addr;
								txdata->dstport = query->srcip.sin_port;
								txdata->srcip = query->dstip;
								txdata->srcport = htons(53);
								fio_send(tar, htons(err_size), txdata, 0);
								//fio_nic_sendmsg(tar, query->srcip.sin_addr.s_addr, query->srcip.sin_port,&query->smac,query->dstip, htons(53), htons(err_size), txdata, 0);


								/*local log*/
								dns_lcllog_errdst_count();
								dns_lcllog_errdst_viewcount(query->view_id);
								dns_lcllog_errdst(NIC_EXTRA_CONTEXT(src)->me,query->view_id,
										domain,query->srcip.sin_addr.s_addr);
								//dns_kafka_lcllog_errdst(NIC_EXTRA_CONTEXT(src)->me,query->view_id,domain,query->srcip.sin_addr.s_addr);

								//log errdst
								//dns_pktlog_errdst(out,query->srcip.sin_addr.s_addr,query->view_id,domain,strlen(domain),NIC_EXTRA_CONTEXT(src)->me,query->srcip.sin_port);
							}
							else
							{
								hyb_debug("errdst get pkt failed!\n");
							}
						}
                        #endif
					}
					else
					{
                        #if 0
						/*是否为后台req*/
                        
						if (query->bg_req == 0)
						{
							query->bg_req = 1;
							/*回复给客户*/
							/*local log*/
							dns_lcllog_answer_count();
							dns_lcllog_answer_viewcount(query->view_id);
							memcpy(recvmsg, &query->clientid, 2); //还原ID号

							//hyb_debug("errdst msg to %s port:%d id:%d\n",inet_ntoa(*(struct in_addr *)&query->srcip.sin_addr.s_addr),ntohs(query->srcip.sin_port),ntohs(query->clientid));
							// hyb_debug("Return msg to %s port:%d id:%d\n",inet_ntoa(*(struct in_addr *)&query->ip.sin_addr.s_addr),ntohs(query->ip.sin_port),ntohs(query->clientid));

							txcount = fio_nic_reserve_tx(tar, &txdata,rxdata);
							NOD("************pkt_id %d txdata slot_rx %p", txdata->id_, txdata->slot_rx);
							if (txcount)
							{
								memcpy(txdata->pdata, recvmsg , recvlen);
								txdata->dmac = &query->smac;
								txdata->dstip = query->srcip.sin_addr.s_addr;
								txdata->dstport = query->srcip.sin_port;
								txdata->srcip = query->dstip;
								txdata->srcport = htons(53);

								fio_send(tar, htons(recvlen), txdata, 0);
								//fio_nic_sendmsg(tar, query->srcip.sin_addr.s_addr, query->srcip.sin_port,&query->smac,query->dstip, htons(53), htons(recvlen), txdata, 0);

							}
						}
						else
						{
							//                            hyb_debug("[ ------ reveive a background answer! ------ ]\n");
						}
                        #endif
					}

#if 1
					if(qcount == 1 && acount > 0)
					{
						/*缓存应答包*/
						answer_to_cache(domain,0,recvmsg,recvlen);
					}
#endif
					//dns_query_table_remove(query);
					//dns_query_table_delkey(key);


				}
			}

		
#endif
	

}


static void send_pkt_debug(char *domain,int type)
{
    
	int txcount = 0;
	struct fio_txdata *txdata = NULL;
    struct fio_nic *t =NULL;
    t = fio_nic_fromip("p1p1");


    txcount = fio_nic_reserve_tx(t,&txdata,NULL);
	if (unlikely(txcount == 0))
	{
		return;
	}

    struct sockaddr_in addr;
    bzero(&addr,sizeof(struct sockaddr_in));

    
    char buf[1500] = {0};
    
    int buflen = dns_pack_query(buf,domain,strlen(domain),2,type);

    if(inet_aton(CORE_IP_ADDR,&addr.sin_addr) < 0) {
        fprintf(stderr,"IP error:%sn",strerror(errno));
        exit(1);
    }

	txdata->dstip = addr.sin_addr.s_addr;
    txdata->srcip = inet_aton("192.168.23.226",NULL);
	txdata->dstport = htons(53);
	txdata->srcport = htons(5353);
    
    memcpy(txdata->pdata,buf,buflen);
    
	fio_send(t, htons(buflen), txdata, 1);
    
}


void request_to_core(char *domain,int view, int type)
{
    
	int txcount = 0;
	struct fio_txdata *txdata = NULL;
    struct fio_nic *t =NULL;
    t = fio_nic_fromip("p1p1");


    txcount = fio_nic_reserve_tx(t,&txdata,NULL);
	if (unlikely(txcount == 0))
	{
		return;
	}

    struct sockaddr_in addr;
    bzero(&addr,sizeof(struct sockaddr_in));

    
    char buf[1500] = {0};
    
    int buflen = dns_pack_query(buf,domain,strlen(domain),view,type);

    if(inet_aton(CORE_IP_ADDR,&addr.sin_addr) < 0) {
        fprintf(stderr,"IP error:%sn",strerror(errno));
        exit(1);
    }

	txdata->dstip = addr.sin_addr.s_addr;
    txdata->srcip = inet_aton("192.168.23.226",NULL);
	txdata->dstport = htons(53);
	txdata->srcport = htons(5353);
    
    memcpy(txdata->pdata,buf,buflen);
    
	fio_send(t, htons(buflen), txdata, 1);
    
}



/****************************************************************************
 * 定时器回调
 *****************************************************************************/

static void timer_callback1(void *user_data)
{
	global_now++;
	g_tm = localtime(&global_now);
    
    /*
    struct sockaddr_in addr;
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd < 0) {
        fprintf(stderr,"Socket Error:%sn",strerror(errno));
    }

   
    // 填充服务端的资料
    bzero(&addr,sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(53);
    if(inet_aton("192.168.85.192",&addr.sin_addr) < 0) {
        fprintf(stderr,"IP error:%sn",strerror(errno));
        exit(1);
    }

    char buf[1500] = {0};
    
    int buflen = dns_pack_query(buf,"www.he.com",strlen("www.he.com"), 0x0001);
    
    sendto(sockfd,buf,buflen,0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
    */

    //send_pkt_debug();

    //dns_msg_cache_analyze(NULL);

    /*test*/
    //char msg[1024] = {0};
    //answer_to_mgr("dns", 1, 2, "www.he.com", 0, msg);

         
	dns_lcllog_general();
	dns_lcllog_view();
}


static void timer_callback2(void *user_data)
{
	global_now = time(NULL);
	if(dns_cache_info())
	{
        /*初始化配置*/
        init_to_mgr();
	}

    register_to_mgr();

#ifdef _DISPLAY_TABLE
	display_table();
#endif
}


/*
static void timer_callback31(void *user_data)
{
	dns_errdst_clear();
}*/

static void timer_callback3(void *user_data)
{
	dns_cache_check();
	
}

static void timer_callback4(void *user_data)
{
	dns_cache_clear();
    dns_ext_cache_clear();
}


static void timer_callback5(void *user_data)
{

    
    dns_lcllog_reqcnt();
    
	if (g_mday == 0)
	{
		g_mday = g_tm->tm_mday;
	}
	else
	{
		if (g_mday == g_tm->tm_mday)
		{
			hyb_debug("The Same day,no update\n");   
		}
		else
		{
			dns_lcllog_remake_path();
			hyb_debug("Not The Same day, dns_lcllog_remake_path\n");
			g_mday = g_tm->tm_mday;

		}
	}
}


/****************************************************************************/




static int dns_timer_init()
{
    /*日志、时间*/
	if (!efly_timer_set(1000,1000, timer_callback1, NULL))
	{
		goto FAILED;
	}

    /*内存记录信息screen log*/
	if (!efly_timer_set(10000,10000, timer_callback2, NULL))
	{
		goto FAILED;
	}

    /*失效缓存检查 时间初期不需要太频繁*/
	if (!efly_timer_set(3600000,3600000, timer_callback3, NULL))
	{
		goto FAILED;
	}

     /*缓存节点内存清理*/
	if (!efly_timer_set(17000,17000, timer_callback4, NULL))
	{
		goto FAILED;
	}

    /*一分钟回调一次用*/
	if (!efly_timer_set(10000, 60000, timer_callback5, NULL))
	{
		goto FAILED;
	}

	return 0;

FAILED:

	return -1;
}


static void dns_handle_msg(void *buf, int32_t b_len, char *src,char *answer)
{
	assert(buf);

	//answer_display(buf);

	switch(get_type_from_msg(buf))
	{
		case DOMAIN_TYPE:
			deal_domain_type(buf, answer);
			break;

		case EXTEND_TYPE:
			deal_extend_type(buf, answer);
			break;

		case VIEW_TYPE:
			deal_view_type(buf, answer);
			break;

		case MASK_TYPE:
			deal_mask_type(buf, answer);
			break;

		default:
			answer_failure(answer);
			hyb_debug("Wrong message!\n");
			break;

	}

}




static void dns_handle_errdst(void *buf, int32_t b_len, char *src,char *answer)
{
	assert(buf);
    char ip[MAX_IP_LEN] = {0};
    struct in_addr ip_in;
    int view = 0;
    unsigned char c_view = 0;
    
    switch(get_type_from_msg(buf))
    {
        
    case ERRDST_ADD:
    case ERRDST_MOD:

        
        memcpy(&c_view,buf+1,1);

        view =  (int)c_view;
	    memcpy(&ip_in,buf+2,sizeof(struct in_addr));
        strcpy_n(ip,MAX_IP_LEN,inet_ntoa(ip_in));

        hyb_debug("receive a errdst set msg ip:%s,view:%d\n",ip,view);
	    if (dns_view_err_set(view,ip))
	    {
            answer_failure(answer);
	    }

        if (view == 0)
        {
            dns_view_err_all_on();
            hyb_debug("dns_view_err_all_on\n");
        }
	    answer_sucess(answer);

        break;
        
    case ERRDST_DEL:

        memcpy(&c_view,buf+1,1);

        view =  (int)c_view;
	    memcpy(&ip_in,buf+2,sizeof(struct in_addr));
        strcpy_n(ip,MAX_IP_LEN,inet_ntoa(ip_in));

        hyb_debug("receive a errdst del msg ip:%s,view:%d\n",ip,view);
    
	    if (dns_view_err_unset(view))
	    {
            answer_failure(answer);
	    }

        if (view == 0)
        {
            dns_view_err_all_off();
            hyb_debug("dns_view_err_all_off\n");
        }
	    answer_sucess(answer);

        break;
    default:
        hyb_debug("dns hadle errdst wrong msg opt!\n");
        break;
    }

    return;
}

static void dns_handle_pktlog(void *buf, int32_t b_len, char *src,char *answer)
{
	assert(buf);

	int ip = {0};
	memcpy(&ip,buf,sizeof(struct in_addr));

	//dns_pktlog_set(ip);
	answer_sucess(answer);


}

static void dns_handle_cache_option(void *buf, int32_t b_len, char *src,char *answer)
{
	assert(buf);

    hyb_debug("receive a msg from:[ip:%s]\n",src);
    dns_msg_cache_analyze(buf);
    

}

static void dns_handle_view_option(void *buf, int32_t b_len, char *src,char *answer)
{
	assert(buf);

    dns_msg_view_analyze(buf);
    

}


static int dns_event_init()
{

	signal(SIGPIPE, SIG_IGN);

	if(efly_event_init())
	{
		goto FAILED;
	}

	g_ipc_svr = efly_ipc_init("dns"); 
	if (!g_ipc_svr)
	{
		goto FAILED;

	}

	if (efly_ipc_reg_func(g_ipc_svr, 1, dns_handle_msg))
	{
		goto FAILED;
	}

	if (efly_ipc_reg_func(g_ipc_svr, 2, dns_handle_errdst))
	{
		goto FAILED;
	}

	if (efly_ipc_reg_func(g_ipc_svr, 3, dns_handle_pktlog))
	{
		goto FAILED;
	} 

    if (efly_ipc_reg_func(g_ipc_svr, 4, dns_handle_cache_option))
	{
		goto FAILED;
	} 

    if (efly_ipc_reg_func(g_ipc_svr, 5, dns_handle_view_option))
	{
		goto FAILED;
	} 

	return 0;

FAILED:

	return -1;
}


static void dns_event_destroy()
{

	if (g_ipc_svr)
	{
		efly_ipc_destroy(g_ipc_svr);
	}
}


static void dns_timer_destroy()
{

	efly_timer_clear();     
}


static int dns_table_init()
{
	setTime();
	dns_domain_init();
	dns_extend_init();
	dns_view_init();
	dns_mask_init();

	return 0;
}

#ifdef _DISPLAY_TABLE
static void display_table()
{
	int number = dns_domain_node_count();
	int size = 1024*MAX_DOMAIN_LEN;
	char output[size];
	memset(output,0,size);
	dns_domain_node_display(output,size);
	printf("\n**********************domain\n");
	printf(output);
	printf("\n**********************\n");

	number = dns_extend_node_count();
	memset(output,0,size);
	dns_extend_node_display(output,size);
	printf("\n**********************extend\n");
	printf("\n**********************\n");

	number = dns_mask_control_node_count();
	memset(output,0,size);
	dns_view_display(output,size);
	printf("\n**********************view\n");
	printf(output);
	printf("\n**********************\n");

	char mask_output[size];
	memset(mask_output,0,size);
	dns_mask_view_node_display(mask_output,size);
	printf("\n**********************mask\n");
	printf(mask_output);
	printf("\n**********************\n");
}
#endif


/* init_socket – 初始化socket句柄 */

static int dns_config_init()
{

	/*初始化缓存列表*/
	if (unlikely(dns_cache_init()))
	{
		hyb_debug("dns_cache_init failed!\n");
		return -1;
	}

    /*初始化泛解析缓存列表*/
    if (unlikely(dns_ext_cache_init()))
	{
		hyb_debug("dns_cache_init failed!\n");
		return -1;
	}

	/*初始化视图配置*/
	if (unlikely(dns_table_init()) )
	{
		hyb_debug("dns_table_init failed!\n");
		return -1;
	}

	/*初始化错误重定向*/
	if (unlikely(dns_errdst_init()))
	{
		hyb_debug("dns errdst init failed\n");
		return -1;
	}



	return 0;
}


/*
   static int 
   create_dns_answer(struct context *t, struct fio_rxdata *rxdata,char * packet,
   int packet_len, unsigned short id,struct sockaddr_in *from)
   {


   int txcount = 0;
   struct fio_txdata *txdata = NULL;

   txcount = fio_reserve_tx(t, 1, &txdata,rxdata,1);
   if (txcount)
   {
   memcpy(txdata->pdata, packet , packet_len);
   memcpy(txdata->pdata, &id , 2);
   fio_sendmsg(t, from->sin_addr.s_addr, from->sin_port, 
   htons(53), htons(packet_len), txdata);
   return 0;
   }
   return -1;

   }
   */



/**
 * @brief   检查是否使用劫持包应答
 * @param   t   
 * @param   txdata   
 * @param   domain   
 * @param   domainlen   
 * @param   view_id   
 * @param   from   
 * @param   qid   
 *
 * @return 
 * @remark null
 * @see     
 * @author hyb      @date 2013/08/20
 **/
static int dns_record_answer_check(struct fio_nic *t,struct fio_nic *log_t, struct fio_txdata *txdata,
		char *domain,int domainlen,int view_id,struct sockaddr_in *from, unsigned short qid,uint32_t dip,
		struct ether_addr smac)
{
	int packet_len = 0;

	/*取劫持包*/
	packet_len = dns_record_get(txdata->pdata,domain,domainlen,view_id);
	if (packet_len)
	{
		memcpy(txdata->pdata, &qid , 2);
		txdata->dmac = &smac;
		txdata->dstip = from->sin_addr.s_addr;
		txdata->dstport = from->sin_port;
		txdata->srcip = dip;
		txdata->srcport = htons(53);
		fio_send(t, htons(packet_len), txdata, 0);
		//fio_nic_sendmsg(t, from->sin_addr.s_addr, from->sin_port,&smac,dip, htons(53), htons(packet_len), txdata, 0);

		/*local count*/
		dns_lcllog_record_count();
		dns_lcllog_record_viewcount(view_id);

		//dns_test_log_answer(NIC_EXTRA_CONTEXT(t)->me,dip,from->sin_addr.s_addr,qid,1,domain);

        //dns_pktlog_record(log_t,from->sin_addr.s_addr,view_id,domain,domainlen,NIC_EXTRA_CONTEXT(log_t)->me,from->sin_port);
        dns_rsyslog("Hijack Answer [view:%d]=>[%s] to user[%s:%d]",view_id,domain,inet_ntoa(*(struct in_addr *)&from->sin_addr.s_addr),ntohs(from->sin_port));
		return 0;
	}

	return -1;

}



/**
* @brief   检查错误包是否已缓存
* @param   t   
* @param   log_t   
* @param   txdata   
* @param   domain   
* @param   domainlen   
* @param   view_id   
* @param   from   
* @param   qid   
* @param   dip   
* @param   smac   
*
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/11/07
**/
static int dns_errdst_answer_check(struct fio_nic *t,struct fio_nic *log_t, struct fio_txdata *txdata,
		char *domain,int domainlen,int view_id,struct sockaddr_in *from, unsigned short qid,uint32_t dip,
		struct ether_addr smac)
{
	int packet_len = 0;

	/*取劫持包*/
	packet_len = dns_errdst_answer_cache_get(domain,domainlen,view_id,txdata->pdata);
	if (packet_len)
	{
		memcpy(txdata->pdata, &qid , 2);
		txdata->dmac = &smac;
		txdata->dstip = from->sin_addr.s_addr;
		txdata->dstport = from->sin_port;
		txdata->srcip = dip;
		txdata->srcport = htons(53);
		fio_send(t, htons(packet_len), txdata, 0);

		/*local count*/
		dns_lcllog_errdst_count();
		dns_lcllog_errdst_viewcount(view_id);

		//dns_pktlog_errdst(log_t,from->sin_addr.s_addr,view_id,domain,domainlen,NIC_EXTRA_CONTEXT(log_t)->me,from->sin_port);
        dns_rsyslog("Errno Answer [view:%d]=>[%s] to user[%s:%d]",view_id,domain,inet_ntoa(*(struct in_addr *)&from->sin_addr.s_addr),ntohs(from->sin_port));
		return 0;
	}

	return -1;

}



//#include "fio_sysconfig.h"
static int
dns_packet_handle(struct fio_nic *src, struct fio_nic *in, struct fio_nic *out,
		struct fio_rxdata* pkts, int p_num, void *useless)
{
	int i = 0;

    //hyb_debug("Receiv a packet fromport %d recvport:%d\n",ntohs(pkts->sport),ntohs(pkts->dport));
	for (i = 0; i < p_num; i++)
	{		    
         //hyb_debug("Receiv a packet fromport %d recvport:%d\n",ntohs(pkts->sport),ntohs(pkts->dport));
		if (sysconfig.prmac_info && (pkts->dip == 0 || pkts->sip == 0))
		{
			struct ether_addr *dmac = (struct ether_addr*)(pkts->pbuf+g_eth_dmac);
			struct ether_addr *smac = (struct ether_addr*)(pkts->pbuf+g_eth_dmac+ETH_ALEN);
			sysconfig.maclog.vtbl.print(&sysconfig.maclog, 
					"tid %d recv \"allzero\" pkt, %u -> %u, %02x:%02x:%02x:%02x:%02x:%02x  -> %02x:%02x:%02x:%02x:%02x:%02x\n", 
					NIC_EXTRA_CONTEXT(src)->me, pkts->sip, pkts->dip,
					smac->ether_addr_octet[0], smac->ether_addr_octet[1], smac->ether_addr_octet[2],
					smac->ether_addr_octet[3], smac->ether_addr_octet[4], smac->ether_addr_octet[5],
					dmac->ether_addr_octet[0], dmac->ether_addr_octet[1], dmac->ether_addr_octet[2],
					dmac->ether_addr_octet[3], dmac->ether_addr_octet[4], dmac->ether_addr_octet[5]);
		}

        //unsigned short from_srv = htons(54);
		switch(pkts->dport)
		{
			case 0x3500:
				//TODO REQUEST
				handle_query_msg(src,in,out,pkts);
				break;

			case 0xE914:	 
				handle_answer_msg(src,in,out,pkts);
				break;
		}

		pkts++;
	}

	return 0;
}



static int dns_fio_init()
{

	if (parse_config("./fio.conf"))
	{
		hyb_debug("parse_config error\n");
		return -1;
	}

	if (fio_register_handler(T_FIO_PKT_INTD, dns_packet_handle))
	{
		hyb_debug("fio_register_handler error\n");
		return -1;
	}

	if (0 != fio_init() )
	{
		hyb_debug("fio_init error\n");
		return -1;
	}

	if (0 != fio_start())
	{
		hyb_debug("fio_start error\n");
		return -1;
	}

	if (fio_start_statistics())
	{
		hyb_debug("fio_start_statics error\n");
		return -1;
	}


	return 0;

}

#if 0
static int dns_database_init()
{

	pid_t pid = fork();
	if (pid < 0)
	{
		fprintf(stderr, "error!");
		return -1;
	}
	else if( 0 == pid )
	{
		//chdir("/");
		setsid();
		umask(0);
		//system("php ./dns_php/init.php");

		execl("/bin/sh", "sh", "-c", "php ./dns_php/init.php", (char *)0);
		hyb_debug("Init table sucess.\n");
		exit(0);

	}    

	return 0;


}
#endif


static int dns_socket_init(char *dstip, int port)
{
    assert(dstip);

    g_answer_sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(g_answer_sockfd < 0) {
        fprintf(stderr,"Socket Error:%sn",strerror(errno));
        return -1;
    }

    bzero(&g_answer_addr,sizeof(struct sockaddr_in));
    g_answer_addr.sin_family = AF_INET;
    g_answer_addr.sin_port = htons(port);
    
    if(inet_aton(dstip,&g_answer_addr.sin_addr) < 0) {
        fprintf(stderr,"IP error:%sn",strerror(errno));
        return -1;
    }
    
	return 0;


}


/**
* @brief   请求初始化向init
* @param   nameclass   
* @param   type   
* @param   view   
* @param   data   
* @param   result   
*
* @return 
* @remark null
* @see     
* @author hyb      @date 2014/06/19
**/
static void init_to_mgr()
{
    char msg[128]={0};
    
    int ret = dns_msg_pack_init(msg);
    if(ret)
    {
        hyb_debug("dns_msg_pack_answer error!\n");
        return;
    }
    
    sendto(g_answer_sockfd, msg,strlen(msg),0,(struct sockaddr *)&g_answer_addr,
        sizeof(struct sockaddr_in));
    
}


/**
* @brief   请求初始化向init
* @param   nameclass   
* @param   type   
* @param   view   
* @param   data   
* @param   result   
*
* @return 
* @remark null
* @see     
* @author hyb      @date 2014/06/19
**/
static void register_to_mgr()
{
    char msg[128]={0};
    
    int ret = dns_msg_pack_register(msg);
    if(ret)
    {
        hyb_debug("dns_msg_pack_answer error!\n");
        return;
    }
    
    sendto(g_answer_sockfd, msg,strlen(msg),0,(struct sockaddr *)&g_answer_addr,
        sizeof(struct sockaddr_in));
    
}


/**
* @brief   回复信息到Mgr
* @param   nameclass   
* @param   type   
* @param   view   
* @param   data   
* @param   result   
*
* @return 
* @remark null
* @see     
* @author hyb      @date 2014/05/22
**/
void answer_to_mgr(char *nameclass,int opt,int type,int view,char *data, int result)
{
    char msg[512]={0};
    
    int ret = dns_msg_pack_answer(nameclass,opt,type,view,data,result,msg);
    if(ret)
    {
        hyb_debug("dns_msg_pack_answer error!\n");
        return;
    }

    sendto(g_answer_sockfd, msg,strlen(msg),0,(struct sockaddr *)&g_answer_addr,
        sizeof(struct sockaddr_in));
    
}


/**
 * @brief   主函数
 * @param   argc   
 * @param   argv   
 *
 * @return 
 * @remark null
 * @see     
 * @author hyb      @date 2013/08/20
 **/
int main(int argc, char **argv)
{
	global_now = time(NULL);
	g_tm = localtime(&global_now);

    char mac[17] = {0};
    source_hwaddr("p1p1", mac);
    hyb_debug("p1p1 mac address: [%s]\n", mac);
	hyb_debug(" --- EflyDns Proxy Start! --- \n");

    /*证书认证*/
    if (unlikely(dns_verify(mac,strlen(mac))))
    {
        hyb_debug("[Veirfy] failed!\n");
		goto VERIFY_ERROR;
    }

    dns_syslog_init();

    /*初始化mgr socket*/
	if (unlikely(dns_socket_init("10.10.10.4",54321)))
	{
		hyb_debug("[Fio init] failed!\n");
		goto SOCK_ERROR;
	}

#if 0
	/*初始化日志*/
	if (unlikely(dns_pktlog_init("192.168.22.125", 8675,24,5)))
	{
		hyb_debug("[Log init] failed!\n");
		goto SOCK_ERROR;
	}
#endif

#if 1
    /*初始化本地信息记录*/
	if(unlikely(dns_lcllog_init(24)))
	{
		hyb_debug("[Lcllog init] failed!\n");
		goto QUIT;
	}
#endif

	/*初始化数据结构*/
	if (unlikely(dns_config_init()))
	{
		hyb_debug("[Config init] failed!\n");
		goto QUIT;
	}

	/*初始化libevent通信*/
	if (unlikely(dns_event_init()))
	{
		hyb_debug("[Event init] failed!\n");
		goto QUIT;
	}

	/*初始化定时器*/
	if (unlikely(dns_timer_init()))
	{
		hyb_debug("[Timer init] failed!\n");
		goto QUIT;
	}

	/*导入数据库配置*/
    #if 0
	if (unlikely(dns_database_init()))
	{
		hyb_debug("[Database init] failed!\n");
		goto SOCK_ERROR;
	}
    #endif



	/*初始化fio*/
	if (unlikely(dns_fio_init()))
	{
		hyb_debug("[Fio init] failed!\n");
		goto QUIT;
	}

    /*初始化配置*/
    init_to_mgr();

	/*事件等待*/
	efly_event_loop();


QUIT:
	/*程序退出*/

	fio_wait();

	fio_shutdown();

	dns_cache_destroy();

	//dns_query_table_destroy();

	dns_timer_destroy();

	dns_event_destroy();

    
VERIFY_ERROR:
SOCK_ERROR:

	hyb_debug(" --- EflyDns proxy Exit --- \n");

	return 0;
}
