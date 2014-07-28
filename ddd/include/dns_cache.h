#ifndef __DNS_CACHE_H__
#define __DNS_CACHE_H__

#define MAX_DOMAIN_LEN	    (255)
#define MAX_VIEM_NUM        (1024)

#include "dns_unpack.h"



/**
* @brief 获取缓存记录条数
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/08/21
**/
int dns_cache_count();


/**
* @brief    初始化缓存表
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/10
**/
int dns_cache_init();



/**
* @brief    清理缓存表
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/10
**/
void dns_cache_destroy();

/**
* @brief   设置dns缓存
* @param   domain           域名
* @param   domain_len       域名长度
* @param   view_id          视图id
* @param   answer           回复包
* @param   answer_len       回复包长度
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hyb      @date 2013/07/09
**/
int dns_cache_set(char *domain,int domain_len,unsigned int view_id,char* answer,
        int answer_len,unsigned short type);


/**
* @brief   获取dns缓存
* @param   domain           域名
* @param   domain_len       域名长度 
* @param   view_id          视图ID
* @param   answer           回复包
*
* @return  成功获取的字节数，0为失败
* @remark null
* @see     
* @author hyb      @date 2013/07/10
**/
int dns_cache_get(char* answer,char* domain,int domain_len,int view_id,
    unsigned short type);


/**
* @brief   缓存信息去除
* @param   domain   
* @param   domain_len   
* @param   view_id   
* @param   type   
*
* @return 
* @remark null
* @see     
* @author hyb      @date 2014/05/21
**/
int dns_cache_delete(char *domain,int domain_len,unsigned int view_id,
    unsigned short type);


/**
* @brief   遍历内存中所有缓存信息，过期的缓存记录将扔进回收链表
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/10
**/

void dns_cache_check();



/**
* @brief    清空回收链表
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/10
**/
void dns_cache_clear();


/**
* @brief  查看缓存条目存储情况
* @return 
* @remark null
* @see     
* @author hyb      @date 2014/05/13
**/
int dns_cache_info();

#endif