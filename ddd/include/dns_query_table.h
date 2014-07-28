#ifndef __DNS_QUERY_TABLE_H__
#define __DNS_QUERY_TABLE_H__

#include <netinet/in.h>
#include <netinet/ether.h>
#include <libbase.h>

typedef struct st_query_node 
{
    struct list_head    list;
	unsigned short		qid;
	unsigned short		qtype;
	struct sockaddr_in	srcip;
	unsigned short		clientid;
	int					view_id;
	time_t         		w_time;
    int                 dstip;
    int                 ethnum;
    struct ether_addr   smac;
    int                 bg_req;
    

   
    pthread_mutex_t     q_lock;
    time_t              del_time;
    
}st_query_node;

typedef struct st_query_key st_query_key;

/**
* @brief   生成查询表哈希键值
* @param   domain       查询域名
* @param   qid          query id
*
* @return  空为失败
* @remark null
* @see     
* @author hyb      @date 2013/07/24
**/
st_query_key *dns_query_table_getkey(char *domain, unsigned short qid,ushort port);



/**
* @brief   销毁查询表哈希键值
* @param   key          查询表哈希键值
*
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/24
**/

void dns_query_table_delkey(st_query_key *key);



/**
* @brief  DNS查询对应列表初始化
* @return 非0失败
* @remark null
* @see     
* @author hyb      @date 2013/07/24
**/
int dns_query_table_init();



/**
* @brief  DNS查询对应列表销毁
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/24
**/

void dns_query_table_destroy();



/**
* @brief   把客户请求包信息插入到查询表中
* @param   req_buf      客户请求包
* @param   domain       域名
* @param   view_id      视图名称
* @param   q_type       请求包类型
* @param   from         客户ip
*
* @return -1失败，成功返回新的随机quiry id
* @remark null
* @see     
* @author hyb      @date 2013/07/24
**/

unsigned short dns_query_table_insert(char *req_buf, char *domain, int view_id,
        ushort q_type, ushort port,struct sockaddr *from, int dip ,struct ether_addr smac,int ethnum,int bg_req);



/**
* @brief   通过哈希建寻找查询表中对应客户请求包信息
* @param   key      查询表哈希键值
*
* @return 空为失败
* @remark null
* @see     
* @author hyb      @date 2013/07/24
**/
st_query_node *dns_query_table_find(st_query_key *key);



/**
* @brief   通过哈希建删除查询表中对应客户请求包信息
* @param   key      查询表哈希键值
*
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/24
**/
void dns_query_table_remove(st_query_node *node);



/**
* @brief  获取查询表中记录条数
* @return 记录条数值
* @remark null
* @see     
* @author hyb      @date 2013/07/24
**/
int dns_query_table_count();



/**
* @brief  遍历检查删除不必要的记录
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/08/26
**/
void dns_query_table_check();


/**
* @brief 
* @return 定时释放删除链表中的资源
* @remark null
* @see     
* @author hyb      @date 2013/11/21
**/
void dns_query_table_clear();


#endif
