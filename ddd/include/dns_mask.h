#ifndef __DNS_MASK_H__
#define __DNS_MASK_H__

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

typedef unsigned short	ushort;
typedef unsigned long	ulong;


/**
* @brief    初始化mask表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_mask_init();

/**
* @brief    同步mysql到内存
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_mask_syn();

/**
* @brief    清理mask表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_mask_destroy();

/**
* @brief   新建mask
* @param   ipaddr           掩码起始地址
* @param   mask       		掩码位数
* @param   view_id          视图id
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_mask_insert(char ipaddr[MAX_IP_LEN],int mask,ushort view_id);

/**
* @brief   删除mask
* @param   ipaddr           掩码起始地址
* @param   mask       		掩码位数
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_mask_remove(char ipaddr[MAX_IP_LEN],int mask);

/**
* @brief   设置已有mask的view
* @param   ipaddr           掩码起始地址
* @param   mask       		掩码位数
* @param   view_id          视图id
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_mask_set(char ipaddr[MAX_IP_LEN],int mask,ushort view_id);

/**
* @brief   刷新更新到读结构
* @param   无
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
int dns_mask_fresh_view_to_control();


/**
* @brief   刷新更新到读结构-nowait
* @param   无
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
int dns_mask_fresh_view_to_control_nowait();


/**
* @brief   获取ip对应的view
* @param   ipaddr           域名
*
* @return  成功获取到的view_id，-1为失败
* @remark view id
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
ushort dns_mask_get_view(struct in_addr * addr);

/**
* @brief   遍历内存中所有mask_view信息，计算node数目
* @return node 数目
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_mask_view_node_count();

/**
* @brief   遍历内存中所有mask_control信息，计算node数目
* @return node 数目
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_mask_control_node_count();

/**
* @brief   遍历内存中所有mask_view信息，生成内存状态输出
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_mask_view_node_display(char*output,int max_len);

/**
* @brief   遍历内存中所有mask_control信息，生成内存状态输出
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_mask_control_node_display(char*output,int max_len);

/**
* @brief    清空回收链表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_mask_clear();


#endif