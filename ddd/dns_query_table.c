#include "libbase.h"
#include "dns_query_table.h"
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <dns_comdef.h>

#define KEY_FORMAT      "%s/%u/%u"
#define MAX_KEY_LEN     (256)
#define MAX_HASH_SPCES  (1<<23)


struct st_query_key 
{
    char key[MAX_KEY_LEN];
    unsigned int key_len;
    
};


h_hash_st *g_query_table = NULL;
static pthread_mutex_t queryid_lock;
static unsigned short queryid = 1;
extern time_t global_now;
struct list_head g_qtable_dellist;
static int g_qdlist_statu = 0; 

static int g_qtchk_lock = 0;


 
static unsigned short getQUERYID()
{
    unsigned short result = 0;
    pthread_mutex_lock(&queryid_lock);
    result = queryid++;
    if (result == 0)
    {
        result++;
    }
    pthread_mutex_unlock(&queryid_lock);
    return result;
}


/**
* @brief  DNS查询对应列表销毁
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/24
**/    
void dns_query_table_destroy()
{
    if (g_query_table)
    {
        h_hash_destroy(g_query_table);
        g_query_table = NULL;
    }
}



static void query_node_desteoy(void *data)
{  
    /*
    st_query_node *node = (st_query_node *)data;

    if (pthread_mutex_trylock(&node->q_lock))
    {
        //hyb_debug("locking delete %d!\n",node->qid);
        return;
    }
    else
    {
        hyb_debug("nolocking delete %d!\n",node->qid);
        pthread_mutex_unlock(&node->q_lock);
        pthread_mutex_destroy(&node->q_lock);
        h_free(node);
        
    }
    */
    
    return;
    //TODO NOTHING
}


/**
* @brief  DNS查询对应列表初始化
* @return 非0失败
* @remark null
* @see     
* @author hyb      @date 2013/07/24
**/
int dns_query_table_init()
{
    g_query_table = NULL;
    INIT_LIST_HEAD(&g_qtable_dellist);
    pthread_mutex_init(&queryid_lock,NULL);
    srand((int)time(0));
    g_query_table = h_hash_create(query_node_desteoy, NULL, MAX_HASH_SPCES);
    //g_query_table = h_hash_create(NULL, NULL, MAX_HASH_SPCES);
    if (!g_query_table)
    {
        hyb_debug("[DNS_Query_Table init failed!]\n");
        return -1;
    }
    hyb_debug("]Dns Query Table init sucess!]\n");
    return 0;
        
}


static int 
query_key_create(unsigned short qid, char *domain,unsigned short port,char *result)
{
    assert(result);
    assert(domain);
    memset(result,0,MAX_KEY_LEN);
    sprintf_n(result, MAX_KEY_LEN, KEY_FORMAT, domain, qid, port);

    return strlen(result);
    
}

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
        ushort q_type, ushort port,struct sockaddr *from, int dip ,struct ether_addr smac,int ethnum,int bg_req)
{
    char key[MAX_KEY_LEN] = {0};
    unsigned short qid = 0;
    int key_len = 0;
    st_query_node *req = NULL;
    st_query_node *temp = NULL;

    if(!g_query_table || g_qtchk_lock)
    {
        return 0;
    }
    
    qid = getQUERYID();
    key_len = query_key_create(qid,domain,port,key);

    if (h_hash_search(g_query_table,key,key_len,(void **)&temp) == 0)
    {
        //hyb_debug("loop..\n");
        qid = dns_query_table_insert(req_buf, domain, view_id, q_type, port,from,dip,smac,ethnum,bg_req);
        
    }
    else
    {
        //备注:这里其实有隐患，有可能两个同时创建导致覆盖
        req = (st_query_node *)h_malloc(sizeof(st_query_node));
        if (!req)
        {
            hyb_debug("Malloc failed!!!!!!!!!!\n");
            return 0;
        }

        pthread_mutex_init(&req->q_lock,NULL);
        req->qid = qid;
        req->qtype = q_type;
        req->dstip = dip;
        req->ethnum = ethnum;
        if (bg_req == 1)
        {
            req->bg_req = bg_req;
        }
        else
        {
            req->bg_req = 0;
        }
        
        memcpy(&req->smac,&smac,sizeof(struct ether_addr));
        memcpy(&req->srcip, from, sizeof(struct sockaddr_in));
        memcpy(&req->clientid, req_buf, 2);
        //hyb_debug("set in  id:%d",req->clientid);
        req->view_id = view_id;
        req->w_time = global_now;

        //hyb_debug("Insert value %s,%d,%d\n",key,qid,key_len);
        if(h_hash_insert(g_query_table, key, key_len, (void *)req) != 0)
        {
            hyb_debug("Insert %s failed!!!!!!!!!!\n",key);
            h_free(req);
            return 0;
        }
        
    }
    
    //hyb_debug("return qid :%d\n",qid);
    return qid;
}


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

st_query_key *dns_query_table_getkey(char *domain, unsigned short qid,unsigned short port)
{   
    st_query_key * key = NULL;
    unsigned int key_len = 0;
    
    if (!domain || qid == 0)
    {
        return NULL;    
    }

    key = (st_query_key *)h_malloc(sizeof(st_query_key));
    if (key)
    {
        key_len = query_key_create(qid, domain,port,key->key);
        if(key_len == 0)
        {
            hyb_debug("query_key_create failed!\n");
            h_free(key);
            return NULL;
        }
//        hyb_debug("Make key:%s![%d]\n",key->key,key_len);
        key->key_len = key_len;
        return key;
        
    }

    return NULL;
}



/**
* @brief   销毁查询表哈希键值
* @param   key          查询表哈希键值
*
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/24
**/

void dns_query_table_delkey(st_query_key *key)
{
    if(key)
    {
        h_free(key);
    }
}


/**
* @brief   通过哈希建寻找查询表中对应客户请求包信息
* @param   key      查询表哈希键值
*
* @return 空为失败
* @remark null
* @see     
* @author hyb      @date 2013/07/24
**/
st_query_node *dns_query_table_find(st_query_key *key)
{
	st_query_node *temp = NULL;

    if (!key || !g_query_table || g_qtchk_lock)
    {
        return NULL;
    }

    if (h_hash_search(g_query_table,key->key, key->key_len, (void **)&temp))
    {
        return NULL;
    }
    else
    {   
        if(pthread_mutex_trylock(&temp->q_lock))
        {
            //hyb_debug(" -- Find query-table failed. Reason: Locked! -- \n");
            //hyb_debug("Conflict key:%s[%d]\n",key->key,key->key_len);
            return NULL;
        }
        else
        {
            /*
            if(h_hash_delete(g_query_table, key->key, key->key_len))
            {
                hyb_debug("h_hash_delete failed!\n");
            }
            */
            return temp;
        }
    }

    return NULL;
}


static void qtable_add_list_tail(struct list_head *tar)
{
    while(g_qdlist_statu)
    {
        usleep(100);
    }
        
    list_add_tail(tar, &g_qtable_dellist);
}


/**
* @brief   通过哈希建删除查询表中对应客户请求包信息
* @param   key      查询表哈希键值
*
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/24
**/

void dns_query_table_remove(st_query_node *node)
{
    if (!node)
    {
        return;
    }
    /*
    
    pthread_mutex_destroy(&node->q_lock);
    h_free(node);
    */
    node->del_time = global_now + 10;
    qtable_add_list_tail(&node->list);

}


/**
* @brief  获取查询表中记录条数
* @return 记录条数值
* @remark null
* @see     
* @author hyb      @date 2013/07/24
**/

int dns_query_table_count()
{
    if (!g_query_table)
    {
        return 0;
    }
    
	return h_hash_count(g_query_table);
}


int query_table_walk(const void *key, int klen, void *val, void *data)
{
    st_query_node *temp = (st_query_node *)val;

    if (global_now - temp->w_time < 30)
    {
        goto END;
    }

    /*
    if (pthread_mutex_trylock(&temp->q_lock))
    {
        hyb_debug("quert tab locking!!\n");
        goto END;
    }
    else
    {
        if(h_hash_delete(g_query_table, key, klen))
        {
            hyb_debug("h_hash_delete failed!\n");
        }
        dns_query_table_remove(temp);
    }
    */ 
    if(h_hash_delete(g_query_table, key, klen))
    {
        hyb_debug("h_hash_delete failed!\n");
    }
    dns_query_table_remove(temp);
    
END:
    return 0;
}



void dns_query_table_check()
{
    
    if (!g_query_table)
    {
        return;
    }
    g_qtchk_lock = 1;
    hyb_debug("[query check start!]\n");

    h_hash_walk(g_query_table,NULL,query_table_walk);

    g_qtchk_lock = 0;

}


void dns_query_table_clear()
{
    if (list_empty(&g_qtable_dellist))
    {
        hyb_debug("[query clear empty!]\n");
        return;
    }
    else
    {
        hyb_debug("[query clear start!]\n");
    }


    g_qdlist_statu = 1;
    
    st_query_node *pos =NULL;
    st_query_node *n =NULL;
    list_for_each_entry_safe(pos,n,&g_qtable_dellist,list)
    {
        
        if ((global_now - pos->del_time) > 0)
        {
//            hyb_debug("[cache free: [%d]-[%p]!]\n",pos->msg_len,pos);
            list_del_init(&pos->list);
            pthread_mutex_destroy(&pos->q_lock);    
            h_free(pos);
        }
    }

    g_qdlist_statu = 0; 

}


