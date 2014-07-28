#ifndef __DNS_DOMAIN_H__
#define __DNS_DOMAIN_H__

#ifndef MAX_DOMAIN_LEN
#define MAX_DOMAIN_LEN	    (255)
#endif

#ifndef MAX_VIEM_NUM
#define MAX_VIEM_NUM        (1024)
#endif

#ifndef MAX_IP_LEN
#define MAX_IP_LEN			(16)
#endif

#ifndef MAX_PACKET_LEN
#define MAX_PACKET_LEN      (1024)
#endif

typedef unsigned short	ushort;


/**
* @brief    初始化域名表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_domain_init();

/**
* @brief    同步mysql到内存
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_domain_syn();

/**
* @brief    清理域名表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_domain_destroy();

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
int dns_domain_create(char *domain,int domain_len,ushort view_id);

/**
* @brief   删除域名下视图的劫持
* @param   domain           域名
* @param   domain_len       域名长度
* @param   view_id          视图id
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_domain_drop(char *domain,int domain_len,ushort view_id);


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
int dns_domain_del(char *domain,int domain_len);

/**
* @brief   为域名插入IP
* @param   domain           域名
* @param   domain_len       域名长度
* @param   view_id          视图id
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_domain_insert(char *domain,int domain_len,ushort view_id,char ipaddr[MAX_IP_LEN]);

/**
* @brief   为域名删除IP
* @param   domain           域名
* @param   domain_len       域名长度
* @param   view_id          视图id
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_domain_remove(char *domain,int domain_len,ushort view_id,char ipaddr[MAX_IP_LEN]);


/**
* @brief   获取域名对应包
* @param   domain           域名
* @param   domain_len       域名长度 
* @param   view_id          视图ID
* @param   answer           回复包
*
* @return  成功获取的字节数，0为失败
* @remark null
* @see     
* @author hyb      @date 2013/07/22
**/
int dns_domain_get(char packet[MAX_PACKET_LEN],char *domain,int domain_len,ushort view_id);

/**
* @brief   遍历内存中所有域名信息，计算node数目
* @return node 数目
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_domain_node_count();

/**
* @brief   遍历内存中所有域名信息，生成内存状态输出
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_domain_node_display(char*output,int max_len);



/**
* @brief    清空回收链表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_domain_clear();


#endif