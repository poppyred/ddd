//	$Id: dns_pack.c 2013-05-23 likunxiang$
//
#include "dns_pack.h"
#include "fio_nm_util.h"
#ifdef __FIO_RELEASE__
#include "fio_sysconfig.h"
#endif
#include <assert.h>
/*组包大小定义，有可能出问题，暂定512*/
#define MAX_PACKET_SIZE 1024

typedef struct st_dns_struct {
	/* header */
	unsigned short		id;
	unsigned short		logo;
	unsigned short		query;
	unsigned short		answer;
	unsigned short		autho;
	unsigned short		addit;
	/* query */
    unsigned short		q_type;
	unsigned short		q_class;
	/* answer */
	unsigned short		a_name;
    unsigned short		a_type;
	unsigned short		a_class;
    unsigned short      a_null;
    unsigned short      a_ttl;
	unsigned short		a_rdlen;
	unsigned int 	    a_rdata;
    
}st_dns_struct;


char *dns_pack_init()
{
    char *msg = NULL;
    msg = (char *)malloc(MAX_PACKET_SIZE);
    if (msg)
    {
        printf("create\n");
        return msg;
    }

    return NULL;
}


void dns_pack_destroy(char *msg)
{
    if (msg)
    {
        free(msg);
    }
}


int dns_pack_answer(char*result,int total_len,void* ip,unsigned short ttl)
{
    if (!result)
    {
        OD("dns_pack_answer error");
        return 0;
    }
    
    
    int new_len = 0;
	struct st_dns_struct temp;
	struct st_dns_struct *dns_query = &temp;

	dns_query->a_name = htons(0xc00c);
	dns_query->a_type = htons(0x0001);
	dns_query->a_class = htons(0x0001);
	dns_query->a_ttl = htons(ttl);
	dns_query->a_rdlen = htons(0x0004);
    memcpy(&dns_query->a_rdata,ip,4);
	memcpy(result+total_len, &dns_query->a_name, 16);
	new_len = total_len+16;   
    
	return new_len;
}

void dns_edit_ttl(char* result, int total_len, unsigned short ttl)
{
    if (!result)
    {
        OD("dns_edit_ttl error");
        return ;
    }
    
    unsigned int time = htons(ttl);
	memcpy(result+total_len+8, &time, 2);

    return;

}

static int domain_to_q_name(char*domain,int len,char*qname)
{
    assert(domain);
    assert(qname);
    
	char temp_domain[256];
	memcpy(temp_domain,domain,len);
	temp_domain[len] = 0;
	char temp[1024];
	int count = 0;
	char *delims = ".";
	char *myStrBuf=NULL;
	char *p = strtok_r(temp_domain,delims,&myStrBuf);
	while(p!=NULL)
	{
		int size = strlen(p);
		temp[count] = (unsigned char)size;
		memcpy(&temp[count+1],p,size);
		count = count+size+1;
		p = strtok_r(NULL,delims,&myStrBuf);
	}
	temp[count] = 0;
	count++;
	memcpy(qname,temp,count);
	return count;
}


int dns_pack_head(char*result,char*domain,int domain_len)
{
    if (!result || !domain)
    {
        OD("dns_pack_head error");
        return 0;
    }
    
    
	struct st_dns_struct temp;
	struct st_dns_struct *dns_query = &temp;

    dns_query->id = 0;
	dns_query->logo = htons(0x0100);
	dns_query->query = htons(0x0001);
	dns_query->answer = htons(0);
	dns_query->autho = 0;
	dns_query->addit = 0;
	
	memcpy(result,dns_query ,12);

    //hyb_debug("fuck:%s\n",result);
	
	char qname[512];
	int len = domain_to_q_name(domain, domain_len, qname);
	if(len<=0)
	{
		return 0;
	}
	
	memcpy(result+12,qname,len);
	int total_len = 12+len;

	dns_query->q_type = htons(0x0001);
	dns_query->q_class = htons(0x0001);

    memcpy(result+total_len, &dns_query->q_type, 4);
    
    total_len = total_len+4;

    return total_len;
}

void dns_edit_anum(char*result, unsigned short awnser_num)
{
    if (!result)
    {
        OD("dns_pack_head error");
        return ;
    }
     
	unsigned short answer = htons(awnser_num);
	
	memcpy(result+6,&answer,2);
	memset(result+8 ,0 ,2);

    return;
}



/*test*/

/*
int main()
{
    char *msg = NULL;
    msg = dns_pack_init();

    int len = dns_pack_head(msg,"helloworld.com",strlen("helloworld.com"),1);
    
    len = dns_pack_answer(msg,len,inet_addr("127.0.0.1"),1024);

    printf("%d\n",len);
    printf("%02X\n",msg);

    dns_pack_destroy(msg);
}
*/

