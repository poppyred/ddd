#include <netinet/in.h>

#ifndef __DNS_PTR_H__
#define __DNS_PTR_H__

#ifndef MAX_DOMAIN_LEN
#define MAX_DOMAIN_LEN	    (255)
#endif

#ifndef MAX_VIEM_NUM
#define MAX_VIEM_NUM        (100)
#endif

#ifndef MAX_IP_LEN
#define MAX_IP_LEN			(16)
#endif

#ifndef MAX_PACKET_LEN
#define MAX_PACKET_LEN      (1024)
#endif


/**
* @brief    构造ptr的回复包
* @param   packet         输入请求包，输出回复包
* @param   len        packet长度
* @remark packet_len
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int make_ptr_answer(char*packet,int len,char*domain,int domainlen);

/**
* @brief    判断目标地址与反射域名查询的关系
* @param   	domain		输入域名
* @param   	domain_len	输入域名的长度
* @remark 	dip			请求目标IP
* @see     
* @return	0:match;1:unmatch;-1:error
* @author hezuoxiang      @date 2014/01/02
**/
int check_ptr_address(char*domain,int domain_len,int dip);

#endif



