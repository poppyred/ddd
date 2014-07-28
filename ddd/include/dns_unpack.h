#ifndef __DNS_UNPACK_H__
#define __DNS_UNPACK_H__

#include <netinet/in.h>


#define TYPE_A		1
#define TYPE_NS		2
#define TYPE_CNAME	5
#define TYPE_DNAME	(0x27)


struct DNS_HEADER
{
    struct in_addr server;       // change to server addr by hezuoxiang
    unsigned short q_count;      // number of question entries
    unsigned short ans_count;    // number of answer entries
    unsigned short auth_count;   // number of authority entries
    unsigned short add_count;    // number of resource entries
};


struct DNS_Q
{
	unsigned short Name_Length;
    char * Name;
	char * Domain;
    unsigned short Type;
    unsigned short Class;
};

struct DNS_RR
{
	unsigned short Name_Length;
    char * Name;
    char * Domain;
    unsigned short Type;
    unsigned short Class;
    unsigned int TTL;
    unsigned short Length;
    char * data;
    char * dataDomain;
};

typedef struct DNS_INFO
{
    struct DNS_HEADER *  head;
    struct DNS_Q *  question;
    struct DNS_RR *  answer;
    struct DNS_RR *  authority;
    struct DNS_RR *  additional;
}st_dns_info;



void dns_unpack_clear(st_dns_info * dns);
int dns_unpack(char * packet , int packetLength ,st_dns_info * dns);
int domain_toLowerCase(char*domain,int len);
int DNS_Get_Name_Position(char * packet,char*begin,int packetLength,int *array);
    

#endif

