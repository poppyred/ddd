#include "dns_ptr.h"
#include <stdlib.h>
#include "libbase.h"
#include <netinet/in.h>
#include <pthread.h>
#include <assert.h>

#define he_debug(fmt, args ...) \
    do {printf("[%s][%d][he]:"fmt, __FILE__, __LINE__,##args);}while(0)

#define random(x) (rand()%x)


struct dns_query_struct 
{
	unsigned short		id;
	unsigned short		logo;
	unsigned short		query;
	unsigned short		answer;
	unsigned short		autho;
	unsigned short		addit;
	
};

struct dns_answer_struct 
{
	unsigned short		a_name;
    unsigned short		a_type;
	unsigned short		a_class;
	unsigned short		a_ttl;
	unsigned short		a_fill;
	unsigned short		a_rdlen;
	char 				a_rdata[5];
};

struct ptr_ipaddr_answer
{
	char size;
	char domain[4];
};


#define MAX_NAME_LEN        (256)
#define MAX_NODE_OUTPUT_LEN	(300)
#define MAX_OFF_NUM         (32)
#define MAX_IP_SIZE			(4)

#define MAX_PATH_LEN	(256)
#define MAX_COMMAND_LEN	(512)
#define MAX_LINE_SIZE	(256)
#define MAX_DIVIDE_SIZE	(4)
#define MAX_PTR_PARTS_SIZE	(6)
#define uchar	unsigned char
#define LEN_MIN(a,b) ((a) < (b) ? (a) : (b))

static int ptr_domain_to_answer(char * domain,int len,struct ptr_ipaddr_answer answer[4])
{
	//he_debug("rpt test:domain:%s -- len:%d\n",domain,len);

	int domainindex = 0,partcount=0,partindex=0;
	char parts[MAX_PTR_PARTS_SIZE][MAX_DOMAIN_LEN];
	memset(parts,0,sizeof(parts));
	int partslens[MAX_PTR_PARTS_SIZE];
	while(domainindex<len && partcount<MAX_PTR_PARTS_SIZE && partindex<MAX_DOMAIN_LEN)
	{
		if(domain[domainindex] == '.')
		{
			partslens[partcount] = partindex;
			partcount++;
			partindex = 0;
		}
		else
		{
			parts[partcount][partindex] = domain[domainindex];
			partindex++;
		}
		domainindex++;
	}

	if(partcount != 5 || partslens[4]!=7 || partslens[0]>3 || partslens[1]>3 || partslens[2]>3 || partslens[3]>3)
	{
		he_debug("rpt test:partcount:%d\n",partcount);
		return 0;
	}

	answer[0].size = partslens[3];
	memcpy(answer[0].domain,parts[3],partslens[3]);
	
	answer[1].size = partslens[2];
	memcpy(answer[1].domain,parts[2],partslens[2]);
	
	answer[2].size = partslens[1];
	memcpy(answer[2].domain,parts[1],partslens[1]);
	
	answer[3].size = partslens[0];
	memcpy(answer[3].domain,parts[0],partslens[0]);
	
	return 1;
}
/*
static int ipaddr_reverse(char*ipaddr,int len,char * reverse)
{
	char parts[4][3];
	int wordindexs[4] = {0};
	int partindex = 0,charindex=0;
	int i,rev_len=0;
	char * pt = reverse;
	
	while(charindex<len)
	{
		if(ipaddr[charindex] != '.')
		{
			if(wordindexs[partindex] >= 3 || partindex>=4)
			{
				return 0;
			}
			parts[partindex][wordindexs[partindex]] = ipaddr[charindex];
			wordindexs[partindex]++;
			charindex++;
		}
		else
		{
			charindex++;
			partindex++;
		}
	}
	
	for(i=3;i>=0;i--)
	{
		memcpy(pt,parts[i],wordindexs[i]);
		pt[wordindexs[i]] = '.';
		rev_len =rev_len+wordindexs[i]+1;
	}
	
	return rev_len;
}
*/

static int ipaddr_reverse(in_addr_t * s_addr,char * reverse,int len)
{
	uchar * ipaddr_char = (uchar *)s_addr;
	return sprintf_n(reverse,len,"%u.%u.%u.%u.",ipaddr_char[3],ipaddr_char[2],ipaddr_char[1],ipaddr_char[0]);
}

/**
* @brief    判断目标地址与反射域名查询的关系
* @param   	domain		输入域名
* @param   	domain_len	输入域名的长度
* @remark 	dip			请求目标IP
* @see     
* @return	0:match;1:unmatch;-1:error
* @author hezuoxiang      @date 2014/01/02
**/
int check_ptr_address(char*domain,int domain_len,int dip)
{
	char rev_ipaddr[20];
	int ipaddr_len = ipaddr_reverse((in_addr_t *)(&dip),rev_ipaddr,20);
	if(ipaddr_len<8)
	{
		return -1;
	}
	
	//he_debug("reverse ipaddr:%s\n",rev_ipaddr);
	
	int len = LEN_MIN(ipaddr_len,domain_len);
	
	if(memcmp(domain,rev_ipaddr,len) == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/**
* @brief    构造ptr的回复包
* @param   packet         输入请求包，输出回复包
* @param   len        packet长度

* @remark packet_len
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int make_ptr_answer(char*packet,int len,char*domain,int domainlen)
{
	int i;
	int errno,resultlen;
	struct ptr_ipaddr_answer answer[4];
	
	struct dns_query_struct *dns_query = (struct dns_query_struct *)packet;
	struct dns_answer_struct *dns_answer = (struct dns_answer_struct *)(packet+len);

	
	dns_query->logo = htons(0x8180);
	dns_query->query = htons(0x0001);
	dns_query->answer = htons(0x0001);
    //TO EDIT
	dns_query->autho = 0;
	dns_query->addit = 0;


	dns_answer->a_name = htons(0xc00c);
	dns_answer->a_type = htons(0x000c);
	dns_answer->a_class = htons(0x0001);
	dns_answer->a_ttl = htons(0x0000);
	dns_answer->a_fill = htons(0x0000);
	dns_answer->a_rdlen = htons(0x0004);

	resultlen = ptr_domain_to_answer(domain,domainlen,answer);
	if(resultlen > 0)
	{
		int templen = 0;
		for(i=0;i<4;i++)
		{
			dns_answer->a_rdata[templen] = answer[i].size;
			memcpy(&(dns_answer->a_rdata[templen+1]),answer[i].domain,answer[i].size);
			templen = templen+answer[i].size+1;
		}
		dns_answer->a_rdata[templen] = 0;
		return len+12+templen+1;
	}
	else
	{
		dns_answer->a_rdata[0] = 3;
		memcpy(&(dns_answer->a_rdata[1]),"dns",3);
		dns_answer->a_rdata[4] = 0;
		
		return len+17;
	}
	
}



