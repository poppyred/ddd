#ifndef __DNS_VIEW_H__
#define __DNS_VIEW_H__

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

#ifndef MAX_ACTION_SIZE
#define MAX_ACTION_SIZE      (4)
#endif

typedef unsigned short	ushort;
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;


/**
* @brief    初始化视图
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
int dns_view_init();

/**
* @brief    同步mysql到内存
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_view_syn();

/**
* @brief    清理视图
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
void dns_view_destroy();

/**
* @brief   新建视图
* @param   view_id          视图id
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
int dns_view_create(ushort view_id);

/**
* @brief   删除域名劫持
* @param   view_id          视图id
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
int dns_view_drop(ushort view_id);

/**
* @brief   为视图插入IP
* @param   view_id          视图id
* @param   ipaddr           ip地址
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
int dns_view_insert(ushort view_id,char ipaddr[MAX_IP_LEN]);

/**
* @brief   为视图删除IP
* @param   view_id          视图id
* @param   ipaddr           ip地址
*
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
int dns_view_remove(ushort view_id,char ipaddr[MAX_IP_LEN]);


/**
* @brief   获取视图ip地址
* @param   view_id          视图ID
* @param   ipaddr         	ip地址
*
* @return  成功获取的字节数，0为失败
* @remark null
* @see     
* @author hyb      @date 2013/07/25
**/
int dns_view_get_sockaddr(ushort view_id,sockaddr * ipaddrs[MAX_ACTION_SIZE]);

/**
* @brief   遍历内存中所有视图信息，计算有效视图数目
* @return node 数目
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/

int dns_view_node_count();

/**
* @brief   遍历内存中所有视图信息，生成内存状态输出
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/

int dns_view_display(char*output,int max_len);



/**
* @brief    清空回收链表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
void dns_view_clear();



/**
* @brief   为视图设置错误重定向IP
* @param   view_id          视图id
* @param   ipaddr           ip地址
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/11/25
**/
int dns_view_err_set(ushort view_id,char ipaddr[MAX_IP_LEN]);


/**
* @brief   为视图清空错误重定向IP
* @param   view_id          视图id
*
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
int dns_view_err_unset(ushort view_id);

/**
* @brief   获取视图错误重定向IP地址
* @param   view_id          视图ID
* @param   ipaddr         	ip地址
*
* @return  成功获取的字节数，0为失败
* @remark null
* @see     
* @author hyb      @date 2013/07/25
**/
int dns_view_err_get_sockaddr(ushort view_id,struct in_addr * ipaddr);


/**
* @brief   为视图设置非DNS流量指向IP
* @param   view_id          视图id
* @param   ipaddr           ip地址
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/11/25
**/
int dns_view_non_set(ushort view_id,char ipaddr[MAX_IP_LEN]);

/**
* @brief   为视图清空非DNS流量指向IP
* @param   view_id          视图id
*
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
int dns_view_non_unset(ushort view_id);


/**
* @brief   获取视图非DNS流量指向IP
* @param   view_id          视图ID
* @param   ipaddr         	ip地址
*
* @return  成功获取的字节数，0为失败
* @remark null
* @see     
* @author hyb      @date 2013/07/25
**/
int dns_view_non_get_sockaddr(ushort view_id,sockaddr * ipaddr);


/**
* @brief   开启DNS服务器全局设置
*
* @return 成功：0	失败：-1
* @remark null
* @see     
* @author hezuoxiang      @date 2013/11/26
**/
int dns_view_server_all_on();

/**
* @brief   关闭DNS服务器全局设置
*
* @return 无
* @remark null
* @see     
* @author hezuoxiang      @date 2013/11/26
**/
void dns_view_server_all_off();


/**
* @brief   开启错误DNS流量全局设置
*
* @return 无
* @remark null
* @see     
* @author hezuoxiang      @date 2013/11/26
**/
void dns_view_err_all_on();

/**
* @brief   关闭错误DNS流量全局设置
*
* @return 无
* @remark null
* @see     
* @author hezuoxiang      @date 2013/11/26
**/
void dns_view_err_all_off();


/**
* @brief   开启非DNS流量全局设置
*
* @return 无
* @remark null
* @see     
* @author hezuoxiang      @date 2013/11/26
**/
void dns_view_non_all_on();

/**
* @brief   关闭非DNS流量全局设置
*
* @return 无
* @remark null
* @see     
* @author hezuoxiang      @date 2013/11/26
**/
void dns_view_non_all_off();

#endif