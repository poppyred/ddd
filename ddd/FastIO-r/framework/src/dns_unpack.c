//	$Id: fio_unpack.c 2013-05-23 likunxiang$
//
#include "dns_unpack.h"
#include "fio_nm_util.h"

static void DNS_Clear_Count(st_dns_info * dns);


static int is_pointer(int in)
{
    return ((in & 0xc0) == 0xc0);
}

static int checkLength(int * totalLength,int packetLength,int handleLength)
{
	if(handleLength == -1)
    {
        return -1;
    }
    *totalLength += handleLength;
    if(*totalLength > packetLength)
    {
        return -1;
    }
	return 0;
}

/*
static int DNS_Generate_Format(const char *name ,  unsigned char *buf , int *len)
{
	char *pos;
    unsigned char *ptr;
    int n;

    *len = 0;
    ptr = buf;
    pos = (char*)name;
    while(1)
	{
        n = strlen(pos) - (strstr(pos , ".") ? strlen(strstr(pos , ".")) : 0);
        *ptr ++ = (unsigned char)n;
        memcpy(ptr , pos , n);
        *len += n + 1;
        ptr += n;
        if(!strstr(pos , "."))
		{
            *ptr = (unsigned char)0;
            ptr ++;
            *len += 1;
            break;
        }
        pos += n + 1;
    }
	return strlen(pos);
}
*/


static int DNS_Ungenerate_Format(char*packet , char * buf ,int packetLength , char*name , int *len,int recursion)
{
	int n ,flag;
	char *pos = name + (*len);
	char * ptr = buf;
	int length = 0;
	
	while(1)
	{
		flag = (int)ptr[0];
		length++;
		if(flag == 0)
		{
		    return length;
		}
		if(is_pointer(flag) && recursion)
		{
		    n = (int)ptr[1]+((int)ptr[0] & 0x3f)*0xff;
		    ptr = packet + n;
		    DNS_Ungenerate_Format(packet , ptr ,packetLength, name , len,0);
			return 2;
		    break;
		}
		else if(flag>0 && flag<64)
		{
			if((long)(ptr+flag+1)>(long)(packet+packetLength))
			{
				return -1;
			}
		    ptr ++;
		    memcpy(pos , ptr , flag);
		    pos += flag;
		    ptr += flag;
		    *len += flag;
			length += flag;
		    if((int)ptr[0] != 0)
			{
		    	memcpy(pos , "." , 1);
		    	pos += 1;
		    	(*len) += 1;
		    }
		}
		else
		{
			return -1;
		}
	}
}


static int DNS_Get_Question(char * packet,char*begin,int packetLength,struct DNS_Q * question)
{
	char domain [1024];
	int length = 0;
	int nameLength = DNS_Ungenerate_Format(packet,begin,packetLength,domain,&length,1);
	if(nameLength == -1)
	{
		printf("DNS_Get_Record nameLength=-1 in query\n");
		return -1;
	}
	question->Name_Length = nameLength;
	question->Name = malloc(nameLength);
	memset(question->Name,0,nameLength);
	memcpy(question->Name,begin,nameLength);
	domain[length] = 0;
	question->Domain = malloc(length+1);
	memset(question->Domain,0,length+1);
	memcpy(question->Domain,domain,length+1);
	begin += nameLength;
	question->Type = ntohs(((unsigned short*)begin)[0]);
	question->Class = ntohs(((unsigned short*)begin)[1]);
	return nameLength+4;
}


static int DNS_Get_Record(char * packet,char*begin,int packetLength,struct DNS_RR * record)
{
	char domain [2048];
    int length = 0;
    int nameLength = DNS_Ungenerate_Format(packet,begin,packetLength,domain,&length,1);
	if(nameLength == -1)
	{
		printf("DNS_Get_Record nameLength=-1 in record\n");
		return -1;
	}
    record->Name_Length = nameLength;
    record->Name = malloc(nameLength);
	memset(record->Name,0,nameLength);
    memcpy(record->Name,begin,nameLength);
    domain[length] = 0;
    record->Domain = malloc(length+1);
	memset(record->Domain,0,length+1);
    memcpy(record->Domain,domain,length+1);
    begin += nameLength;
    record->Type = ntohs(((unsigned short*)begin)[0]);
    record->Class = ntohs(((unsigned short*)begin)[1]);
	begin += 4;
	record->TTL = ntohl(((unsigned int*)begin)[0]);
	begin += 4;
	record->Length = ntohs(((unsigned short*)begin)[0]);
	begin += 2;
	record->data = malloc(record->Length);
	memset(record->data,0,record->Length);
	if((long)begin+record->Length > (long)packet+packetLength)
	{
		printf("recieve bad format packet.\n");
		return -1;
	}
	memcpy(record->data,begin,record->Length);
	if(record->Type == TYPE_A)
	{
		struct in_addr * ipv4 = (struct in_addr *)record->data;
        char ip_address[16];
        inet_ntop(AF_INET, ipv4, ip_address, 16);
        record->dataDomain = malloc(16);
		memcpy(record->dataDomain,ip_address,16);
	}
	if(record->Type == TYPE_CNAME || record->Type == TYPE_NS)	//cname或 ns
	{
		length = 0;
		int nameLength = DNS_Ungenerate_Format(packet,begin,packetLength,domain,&length,1);
		if(nameLength == -1)
		{
			printf("%s--DNS_Get_Record nameLength=-1\n",record->Domain);
			return -1;
		}
		domain[length] = 0;
		record->dataDomain = malloc(length+1);
		memcpy(record->dataDomain,domain,length+1);
	}
    return nameLength+10+record->Length;
}




int dns_unpack(char * packet , int packetLength ,st_dns_info * dns)
{
	int headLength = sizeof(struct DNS_HEADER);
	int totalLength = headLength;
	dns->head = malloc(headLength);
	memset(dns->head,0,headLength);
	char * ptr = packet;
	memcpy(dns->head,ptr,headLength);

	dns->head->q_count = ntohs(dns->head->q_count);
	dns->head->ans_count = ntohs(dns->head->ans_count);
	dns->head->auth_count = ntohs(dns->head->auth_count);
	dns->head->add_count = ntohs(dns->head->add_count);
	if(dns->head->q_count>0 && dns->head->q_count<10)
	{
		dns->question = malloc(sizeof(struct DNS_Q)*dns->head->q_count);
		memset(dns->question,0,sizeof(struct DNS_Q)*dns->head->q_count);
	}
	else if(dns->head->q_count != 0)
	{
		printf("q_count error:%d\n",dns->head->q_count);
		DNS_Clear_Count(dns);
		return -1;
	}

	if(dns->head->ans_count>0 && dns->head->ans_count<50)
    {
		dns->answer = malloc(sizeof(struct DNS_RR)*dns->head->ans_count);
		memset(dns->answer,0,sizeof(struct DNS_RR)*dns->head->ans_count);
    }
	else if(dns->head->ans_count != 0)
	{
		printf("ans_count error:%d\n",dns->head->ans_count);
		DNS_Clear_Count(dns);
		return -1;
	}

	if(dns->head->auth_count>0 && dns->head->auth_count<50)
    {
		dns->authority = malloc(sizeof(struct DNS_RR)*dns->head->auth_count);
		memset(dns->authority,0,sizeof(struct DNS_RR)*dns->head->auth_count);
    }
	else if(dns->head->auth_count != 0)
	{
		printf("auth_count error:%d\n",dns->head->auth_count);
		DNS_Clear_Count(dns);
		return -1;
	}

	if(dns->head->add_count>0 && dns->head->add_count<50)
    {
		dns->additional = malloc(sizeof(struct DNS_RR)*dns->head->add_count);
		memset(dns->additional,0,sizeof(struct DNS_RR)*dns->head->add_count);
    }
	else if(dns->head->add_count != 0)
	{
		printf("add_count error:%d\n",dns->head->add_count);
		DNS_Clear_Count(dns);
		return -1;
	}

	ptr += headLength;

	if(dns->head->ans_count == 0 && dns->head->add_count == 0 && dns->head->auth_count == 0)
	{
		printf("query packet.\n");
		if(dns->head->q_count == 0)
		{
			return -1;
		}
		else
		{
			int i;
			for(i = 0;i<dns->head->q_count;i++)
			{
				int length = DNS_Get_Question(packet,ptr,packetLength,&dns->question[i]);
				if(checkLength(&totalLength,packetLength,length) == -1)
				{
					return -1;
				}
				ptr += length;
			}
		}
	}
	else
	{
		if(dns->head->q_count != 0)
		{
			int i;
			for(i = 0;i<dns->head->q_count;i++)
			{
				int length = DNS_Get_Question(packet,ptr,packetLength,&dns->question[i]);
				if(checkLength(&totalLength,packetLength,length) == -1)
				{
					printf("checkLength error.\n");
					return -1;
				}
				ptr += length;
			}
		}
		
		if(dns->head->ans_count != 0)
        {
            int i;
            for(i = 0;i<dns->head->ans_count;i++)
            {
                int length = DNS_Get_Record(packet,ptr,packetLength,&dns->answer[i]);
                if(checkLength(&totalLength,packetLength,length) == -1)
                {
                	printf("checkLength error.\n");
                    return -1;
                }
                ptr += length;
            }
        }
        
		if(dns->head->auth_count != 0)
        {
            int i;
            for(i = 0;i<dns->head->auth_count;i++)
            {
                int length = DNS_Get_Record(packet,ptr,packetLength,&dns->authority[i]);
                if(checkLength(&totalLength,packetLength,length) == -1)
                {
                	printf("checkLength error.\n");
                    return -1;
                }
                ptr += length;
            }
        }
        
		if(dns->head->add_count != 0)
        {
            int i;
            for(i = 0;i<dns->head->add_count;i++)
            {
                int length = DNS_Get_Record(packet,ptr,packetLength,&dns->additional[i]);
                if(checkLength(&totalLength,packetLength,length) == -1)
                {
                	printf("checkLength error.\n");
                    return -1;
                }
                ptr += length;
            }
        }
	}
	return 0;
}

static void DNS_Clear_Count(st_dns_info * dns)
{
	dns->head->q_count = 0;
	dns->head->ans_count = 0;
	dns->head->auth_count = 0;
	dns->head->add_count = 0;
}

static void DNS_Free_Q(struct DNS_Q * question)
{
	if(question->Name != 0)
	{
		free(question->Name);
		//printf("free question->Name\n");
	}

	if(question->Domain != 0)
	{
		free(question->Domain);
		//printf("free question->Domain\n");
	}
}

static void DNS_Free_RR(struct DNS_RR * record)
{
	if(record->Name != 0)
	{
		free(record->Name);
		//printf("free record->Name\n");
	}

	if(record->Domain != 0)
	{
		free(record->Domain);
		//printf("free record->Domain\n");
	}

	if(record->data != 0)
	{
		free(record->data);
		//printf("free record->data\n");
	}
	
	if(record->dataDomain != 0)
	{
		free(record->dataDomain);
		//printf("free record->data\n");
	}
}

void dns_unpack_clear(st_dns_info * dns)
{
	int i;

    if(!dns)
    {
        return;
    }

	if(dns->head == 0)
	{
		return;
	}

	for(i = 0;i<dns->head->q_count;i++)
	{
		DNS_Free_Q(&dns->question[i]);
	}
	if(dns->head->q_count > 0)
	{
		free(dns->question);
	}

	for(i = 0;i<dns->head->ans_count;i++)
    {
		DNS_Free_RR(&dns->answer[i]);
    }
	if(dns->head->ans_count > 0)
	{
		free(dns->answer);
	}
	for(i = 0;i<dns->head->auth_count;i++)
    {
		DNS_Free_RR(&dns->authority[i]);
    }
	if(dns->head->auth_count > 0)
	{
		free(dns->authority);
	}
	for(i = 0;i<dns->head->add_count;i++)
    {
		DNS_Free_RR(&dns->additional[i]);
    }
    if(dns->head->add_count > 0)
	{
		free(dns->additional);
	}
    free(dns->head);

}

/*
int main()
{

    (void)dns_unpack(NULL,0,NULL);
    return 0;
}
*/
