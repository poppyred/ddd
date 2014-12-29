#include "dns_pack.h"
#include <string.h>
#include "libbase.h"
#include <assert.h>
/*组包大小定义，有可能出问题，暂定512*/
#define MAX_PACKET_SIZE 1024

#define hyb_debug(fmt, args ...) \
    do {printf("[%s][%d]:"fmt, __FILE__, __LINE__,##args);}while(0)


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


typedef struct st_edns_struct {
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
	unsigned char		a_name;
    unsigned short		a_type;
	unsigned short		a_payload;
    unsigned char       a_rcode;
    unsigned char       a_version;
	unsigned short		a_z;
	unsigned short	    a_datalen;
    unsigned char       a_addflag[4];
    unsigned int        a_data;
    
}st_edns_struct;


char *dns_pack_init()
{
    char *msg = NULL;
    msg = (char *)h_malloc(MAX_PACKET_SIZE);
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
        h_free(msg);
    }
}


int dns_pack_answer(char*result,int total_len,void* ip,unsigned short ttl)
{
    if (!result)
    {
        hyb_debug("dns_pack_answer error\n");
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
    memcpy_s(&dns_query->a_rdata,ip,4);
	memcpy_s(result+total_len, &dns_query->a_name, 16);
	new_len = total_len+16;   
    
	return new_len;
}

void dns_edit_ttl(char* result, int total_len, unsigned short ttl)
{
    /*
    if (!result)
    {
        hyb_debug("dns_edit_ttl error\n");
        return ;
    }
    */
    
    //unsigned int time = htons(ttl);
	memcpy(result+total_len+8, &ttl, 2);

    return;

}



int domain_to_q_name(char*domain,int len,char*qname)
{
    assert(domain);
    assert(qname);
    
	char temp_domain[256];
	memcpy_s(temp_domain,domain,len);
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
		memcpy_s(&temp[count+1],p,size);
		count = count+size+1;
		p = strtok_r(NULL,delims,&myStrBuf);
	}
	temp[count] = 0;
	count++;
	memcpy_s(qname,temp,count);
	return count;
}

int ip_to_q_name(char*ip,int len,char*qname)
{
    assert(ip);
    assert(qname);

    //hyb_debug("origin:%s\n",ip);
    int ptr_cnt = 0;
    int count = 0;
    int i = 0;
	char *delims = ".";
	char *myStrBuf=NULL;
    char *ips[20];
	char *p =NULL;
	while(p = strtok_r(ip,delims,&myStrBuf))
	{
        ip = NULL;

        ips[count] = p;
        count ++;
        
	}

    for(i = count-1;i>=0;i--)
    {
               
        int size = strlen(ips[i]);
        //hyb_debug("ip:%s size:%d\n",p,size);
        memcpy(qname+ptr_cnt,ips[i],size);
        
        ptr_cnt += size;
        memcpy(qname+ptr_cnt,delims,1);
        ptr_cnt += 1;    
    }
    //memcpy_s(qname,ip,len);
    strcat_n(qname,256,"in-addr.arpa");
    //hyb_debug("New ipptr is :%s\n",qname);

    return strlen(qname);

}


int dns_pack_head(char*result,char*domain,int domain_len)
{
    if (!result || !domain)
    {
        hyb_debug("dns_pack_head error\n");
        return 0;
    }
    
    
	struct st_dns_struct temp;
	struct st_dns_struct *dns_query = &temp;

    dns_query->id = 0;
	dns_query->logo = htons(0x8180);
	dns_query->query = htons(0x0001);
	dns_query->answer = htons(0);
	dns_query->autho = 0;
	dns_query->addit = 0;
	
	memcpy_s(result,dns_query ,12);

    //hyb_debug("fuck:%s\n",result);
	
	char qname[512] = {0};
	int len = domain_to_q_name(domain, domain_len, qname);
	if(len<=0)
	{
		return 0;
	}
	
	memcpy_s(result+12,qname,len);
	int total_len = 12+len;

	dns_query->q_type = htons(0x0001);
	dns_query->q_class = htons(0x0001);

    memcpy_s(result+total_len, &dns_query->q_type, 4);
    
    total_len = total_len+4;

    return total_len;
}


int dns_pack_query(char*result,char*domain,int domain_len, int view, int type,unsigned int msg_id)
{
    if (!result || !domain)
    {
        hyb_debug("dns_pack_head error\n");
        return 0;
    }
    
    
	struct st_edns_struct temp = {0};
	struct st_edns_struct *dns_query = &temp;

    dns_query->id = rand();
	dns_query->logo = htons(0x0100);
	dns_query->query = htons(0x0001);
	dns_query->answer = htons(0);
	dns_query->autho = 0;
	dns_query->addit = htons(0x0001);
	
	memcpy_s(result,dns_query ,12);

    //hyb_debug("fuck:%s\n",result);
	
	char qname[512] = {0};
    char inaddr[512] = {0};
    int qlen = 0;
    if (type == 0X000C)
    {
        qlen = ip_to_q_name(domain, domain_len, inaddr);
	    if(qlen<=0)
	    {
		    return 0;
	    }

        qlen = domain_to_q_name(inaddr, strlen(inaddr), qname);
	    if(qlen<=0)
	    {
		    return 0;
	    }
    }
    else
    {
	    qlen = domain_to_q_name(domain, domain_len, qname);
	    if(qlen<=0)
	    {
		    return 0;
	    }
    }

	memcpy_s(result+12,qname,qlen);
	int total_len = 12+qlen;

	dns_query->q_type = htons(type);
	dns_query->q_class = htons(0x0001);

    memcpy_s(result+total_len, &dns_query->q_type, 4);
    
    total_len = total_len+4;

    dns_query->a_name = 0;
    dns_query->a_type = htons(0x0029);
    dns_query->a_payload= htons(view);
    dns_query->a_rcode = 0;
    dns_query->a_version = 0;
    dns_query->a_z = 0;
    dns_query->a_datalen = 0;

    memcpy_s(dns_query->a_addflag, "\x01\x01\x00\x00", 4);
    dns_query->a_data = msg_id;
    memcpy_s(result+total_len, &dns_query->a_name+1, 11+8);

    total_len = total_len+11+8;
    

    return total_len;
}


void dns_edit_anum(char*result, unsigned short awnser_num)
{
    if (!result)
    {
        hyb_debug("dns_pack_head error\n");
        return ;
    }
     
	unsigned short answer = htons(awnser_num);
	
	memcpy_s(result+6,&answer,2);
	memset(result+8 ,0 ,2);

    return;
}


void dns_edit_logo(char*result, unsigned short logo)
{
    if (!result)
    {
        hyb_debug("dns_pack_head error\n");
        return ;
    }
     
	unsigned short logo_n = htons(logo);
	
	memcpy_s(result+2,&logo_n,2);

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
