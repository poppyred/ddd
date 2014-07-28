#include "libbase.h"
#include <netinet/in.h>
#include "dns_cache.h"
#include "dns_pack.h"
#include "dns_unpack.h"
#include <pthread.h>
#include <assert.h>
#include "dns_comdef.h"
#include "dns_view.h"


extern time_t global_now;
//int global_now;

#define MAX_NAME_LEN        (256)
#define MAX_ERRDST_PKTLEN      (1500)
#define MAX_OFF_NUM         (32)
#define MAX_HASH_SPCES  (1<<20)

#define ERRDST_DELETE       (1)

volatile int g_state;
h_hash_st *g_errdst_table = NULL;
//struct in_addr g_errdst_ip = {0};

struct list_head g_errdst_dlist;



typedef struct st_errdst_record
{
    struct list_head list;
    char msg[MAX_ERRDST_PKTLEN];
    unsigned int    msg_len;
   
}st_errdst_record;



static void errdst_member_destroy(void *data)
{
    assert(data);
    st_errdst_record *record = (st_errdst_record *)data;
    list_add_tail(&record->list,&g_errdst_dlist);

    return;
}


/**
* @brief    初始化缓存表
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/10
**/

int dns_errdst_init()
{

    INIT_LIST_HEAD(&g_errdst_dlist);
    g_state = 0;
    g_errdst_table = h_hash_create(errdst_member_destroy,NULL,MAX_HASH_SPCES);
    if (g_errdst_table)
    {
        hyb_debug("[dns errdst init sucess!]\n");
        return 0;
    }

    return -1;
}

/*
int dns_errdst_set(struct in_addr ip)
{
    h_hash_st * new_table = NULL;

    g_errdst_ip = ip;
    hyb_debug("set errdst ip : %s\n",inet_ntoa(ip));
    hyb_debug("set errdst int : %d\n",*(int *)&ip);

    new_table = h_hash_create(errdst_member_destroy,NULL,MAX_HASH_SPCES);
    if (new_table)
    {
        h_hash_st * old = g_errdst_table;
        g_errdst_table = new_table;
        if (old)
        {
            h_hash_destroy(old);
        }

        return 0;
    }

    return -1;

}*/



/**
* @brief    清理缓存表
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/10
**/

void dns_errdst_destroy()
{
    st_errdst_record *pos =NULL;
    st_errdst_record *n =NULL;
    
    if (g_errdst_table)
    {
        h_hash_destroy(g_errdst_table);
        g_errdst_table = NULL;
    }   
    list_for_each_entry_safe(pos,n,&g_errdst_dlist,list)
    {
        list_del(&pos->list);    
        h_free(pos);
    }
}



static void dns_errdst_make_key(char *domain, int view_id,char *key)
{
    sprintf_n(key,256,"%s/%d",domain,view_id);
}


int dns_errdst_answer_get(char *domain, int domainlen ,int view_id,char *result)
{
    char key[256] = {0};
    int key_len = 0;
    struct in_addr errdst_ip = {0};
    st_errdst_record *node = NULL;
    st_errdst_record *temp = NULL;
  
    if (g_state != ERRDST_DELETE)
    {
        /*生成key*/
        dns_errdst_make_key(domain, view_id, key);
        
        key_len = strlen(key);
        if (h_hash_search(g_errdst_table, key, key_len,(void **)&temp) == 0)
        {
            memcpy(result,temp->msg,temp->msg_len);
            return temp->msg_len;
        }
        else
        {
            node = (st_errdst_record *)h_malloc(sizeof(st_errdst_record));
            if (node)
            {
                int offset = dns_pack_head(node->msg, domain, domainlen);
                if (offset == 0)
                {
                    goto FAILED;    
                }

                (void)dns_view_err_get_sockaddr(view_id,&errdst_ip);
                if ((*(int *)&errdst_ip) == 0)
                {
                    // hyb_debug("errdst reply is shut down!!!!\n");
                     node->msg_len = offset;
                     dns_edit_anum(node->msg,0);
                }
                else
                {
                    node->msg_len = dns_pack_answer(node->msg, offset, &errdst_ip, 0);
                    dns_edit_anum(node->msg,1);
                }
                memcpy(result,node->msg,node->msg_len);
                
                (void)h_hash_insert(g_errdst_table,key, key_len, node);
                

                return node->msg_len;
                    
            }
        }
    }
    else
    {
        char answer[512] = {0};
        int answer_len = 0;
        int offset = dns_pack_head(answer, domain, domainlen);
        if (offset == 0)
        {
            goto FAILED;    
        }
        
        (void)dns_view_err_get_sockaddr(view_id,&errdst_ip);
        if ((*(int *)&errdst_ip) == 0)
        {
            //hyb_debug("errdst reply is shut down!!!!\n");
            answer_len = offset;
            dns_edit_anum(answer,0);
        }
        else
        {
            answer_len = dns_pack_answer(answer, offset, &errdst_ip, 0);
            dns_edit_anum(answer,1);
        }

        memcpy(result,answer,answer_len);

        return answer_len;
        
    }
  

FAILED:
    
    if(node)
    {
        h_free(node);
    }

    return 0;
    
}


int dns_errdst_answer_cache_get(char *domain, int domainlen ,int view_id, char *result)
{
    
    st_errdst_record *temp = NULL;
  
    if (g_state != ERRDST_DELETE)
    {
        char key[256] = {0};
         /*生成key*/
        dns_errdst_make_key(domain, view_id, key);
        if (h_hash_search(g_errdst_table, key, strlen(key),(void **)&temp) == 0)
        {
            memcpy(result,temp->msg,temp->msg_len);
            return temp->msg_len;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
    
}




static int errdst_walk(const void *key, int klen, void *val, void *data)
{

    h_hash_delete(g_errdst_table,key,klen);

    return 0;
}


/**
* @brief   遍历内存中所有缓存信息，过期的缓存记录将扔进回收链表
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/10
**/
void dns_errdst_check()
{
    hyb_debug("[errdst check start!]\n");
    if (!g_errdst_table)
    {
        return;
    }

    g_state = ERRDST_DELETE;
    (void)h_hash_walk(g_errdst_table,NULL,errdst_walk);
    g_state = 0;
}


/**
* @brief    清空回收链表
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/10
**/

void dns_errdst_clear()
{
    if (g_state == ERRDST_DELETE)
    {
        return;
    }
    st_errdst_record *pos =NULL;
    st_errdst_record *n =NULL;
    hyb_debug("[errdst clear start!]\n");
    list_for_each_entry_safe(pos,n,&g_errdst_dlist,list)
    {
        list_del(&pos->list);
        h_free(pos);
    }
}


/**
* @brief 获取缓存记录条数
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/08/21
**/
int dns_errdst_count()
{
    return h_hash_count(g_errdst_table);
}


