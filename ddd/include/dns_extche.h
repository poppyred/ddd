#ifndef __DNS_EXTCHE_H__
#define __DNS_EXTCHE_H__

#ifndef MAX_DOMAIN_LEN
#define MAX_DOMAIN_LEN	    (255)
#endif

#ifndef MAX_VIEM_NUM
#define MAX_VIEM_NUM        (100)
#endif

#ifndef MAX_IP_LEN
#define MAX_IP_LEN			(16)
#endif


typedef unsigned short	ushort;



/**
* @brief    初始化域名表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_ext_cache_init();


/**
* @brief    清理域名表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_ext_cache_destroy();

/**
* @brief   新建域名劫持
* @param   domain           域名
* @param   domain_len       域名长度
* @param   view_id          视图id
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_ext_cache_set(char *domain,int domain_len,ushort view_id,char *pkt,
        int pkt_len,unsigned short type);

/**
* @brief   删除域名劫持
* @param   domain           域名
* @param   domain_len       域名长度
* @param   view_id          视图id
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_ext_cache_drop(char *domain,int domain_len,ushort view_id,unsigned short type);

/**
* @brief   删除域名劫持
* @param   domain           域名
* @param   domain_len       域名长度
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_ext_cache_del(char *domain,int domain_len,unsigned short type);


/**
* @brief   获取域名对应包
* @param   domain           域名
* @param   domain_len       域名长度 
* @param   view_id          视图ID
*
* @return  成功获取的字节数，0为失败
* @remark null
* @see     
* @author hyb      @date 2013/07/22
**/
int dns_ext_cache_get(char *pkt,char *domain,int domain_len,ushort view_id,unsigned short type);


/**
* @brief    清空回收链表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_ext_cache_clear();


#endif