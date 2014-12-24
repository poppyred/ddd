#include "libbase.h"
#include <netinet/in.h>
#include "dns_cache.h"
#include "dns_pack.h"
#include <pthread.h>
#include <assert.h>
#include "dns_comdef.h"
#include "dns_pktlog.h"
#include "dns_syslog.h"


extern time_t global_now;

#define MAX_NAME_LEN        (256)
#define MAX_PACKET_LEN      (1500)
#define MAX_OFF_NUM         (32)
#define MAX_HASH_SPCES  (1<<20)
#define MAX_TABLE_NUM       (10)


h_hash_st *g_cache_table[MAX_TABLE_NUM] = {0};

h_hash_st *g_a_table = NULL;
h_hash_st *g_aaaa_table = NULL;
h_hash_st *g_txt_table = NULL;
h_hash_st *g_ns_table = NULL;
h_hash_st *g_mx_table = NULL;
h_hash_st *g_cname_table = NULL;
h_hash_st *g_any_table = NULL;
h_hash_st *g_ptr_table = NULL;



volatile int g_list_statu = 0;

/*功能开关*/
volatile int g_cache_off = 0;


static struct list_head g_delete_list;



typedef struct cache_view_node
{
    struct list_head list;
    char msg[MAX_PACKET_LEN];
    volatile unsigned short  standup;
    unsigned int    msg_len;
    time_t create_time;
    time_t del_time;

    /*防止删除过快*/
    int is_del;
    pthread_mutex_t lock;
    
    
}st_cache_view_node;


typedef struct cache_domain
{
    st_cache_view_node *view[MAX_VIEM_NUM];
    pthread_mutex_t lock[MAX_VIEM_NUM];
        
}st_cache_domain;




static void cache_member_destroy(void *data)
{
    assert(data);
    int i = 0;

    st_cache_domain *domain = (st_cache_domain *)data;
    for (i = 0; i < MAX_VIEM_NUM; i++)
    {

        st_cache_view_node *temp = domain->view[i];
        if (temp)
        {
            h_free(temp);
            domain->view[i] = NULL;
        }
        pthread_mutex_destroy(&domain->lock[i]);
    }
    h_free(domain);

    return;
}


/**
* @brief    初始化缓存表
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/10
**/

int dns_cache_init()
{
    INIT_LIST_HEAD(&g_delete_list);
    
    g_a_table = h_hash_create(cache_member_destroy,NULL,MAX_HASH_SPCES);
    if (!g_a_table)
    {
        hyb_debug("[dns cache init sucess!]\n");
        return -1;
    }
    
    g_aaaa_table = h_hash_create(cache_member_destroy,NULL,MAX_HASH_SPCES);
    if (!g_aaaa_table)
    {
        hyb_debug("[dns cache init sucess!]\n");
        return -1;
    }
    
    g_txt_table = h_hash_create(cache_member_destroy,NULL,MAX_HASH_SPCES);
    if (!g_txt_table)
    {
        hyb_debug("[dns cache init sucess!]\n");
        return -1;
    }
    
    g_ns_table = h_hash_create(cache_member_destroy,NULL,MAX_HASH_SPCES);
    if (!g_ns_table)
    {
        hyb_debug("[dns cache init sucess!]\n");
        return -1;
    }
    
    g_mx_table = h_hash_create(cache_member_destroy,NULL,MAX_HASH_SPCES);
    if (!g_mx_table)
    {
        hyb_debug("[dns cache init sucess!]\n");
        return -1;
    }

    g_cname_table = h_hash_create(cache_member_destroy,NULL,MAX_HASH_SPCES);
    if (!g_cname_table)
    {
        hyb_debug("[dns cache init sucess!]\n");
        return -1;
    }

    g_ptr_table = h_hash_create(cache_member_destroy,NULL,MAX_HASH_SPCES);
    if (!g_ptr_table)
    {
        hyb_debug("[dns cache init sucess!]\n");
        return -1;
    }

    g_any_table = h_hash_create(cache_member_destroy,NULL,MAX_HASH_SPCES);
    if (!g_any_table)
    {
        hyb_debug("[dns cache init sucess!]\n");
        return -1;
    }

    g_cache_table[0] = g_a_table;
    g_cache_table[1] = g_aaaa_table;
    g_cache_table[2] = g_txt_table;
    g_cache_table[3] = g_ns_table;
    g_cache_table[4] = g_mx_table;
    g_cache_table[5] = g_cname_table;
    g_cache_table[6] = g_any_table;
    g_cache_table[7] = g_ptr_table;
    
    return 0;
}


/**
* @brief    清理缓存表
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/10
**/

void dns_cache_destroy()
{
    st_cache_view_node *pos =NULL;
    st_cache_view_node *n =NULL;
    if (g_a_table)
    {
        h_hash_destroy(g_a_table);
        g_a_table = NULL;
    }   

    if (g_aaaa_table)
    {
        h_hash_destroy(g_aaaa_table);
        g_aaaa_table = NULL;
    }
    
    if (g_txt_table)
    {
        h_hash_destroy(g_txt_table);
        g_txt_table = NULL;
    }

    if (g_ns_table)
    {
        h_hash_destroy(g_ns_table);
        g_ns_table = NULL;
    }    

    if (g_mx_table)
    {
        h_hash_destroy(g_mx_table);
        g_mx_table = NULL;
    }

    if (g_cname_table)
    {
        h_hash_destroy(g_cname_table);
        g_cname_table = NULL;
    }

    if (g_ptr_table)
    {
        h_hash_destroy(g_ptr_table);
        g_ptr_table = NULL;
    }

    if (g_any_table)
    {
        h_hash_destroy(g_any_table);
        g_any_table = NULL;
    }
    
    list_for_each_entry_safe(pos,n,&g_delete_list,list)
    {
        list_del(&pos->list);
        pthread_mutex_destroy(&pos->lock);    
        h_free(pos);
    }
}

#if 0
static st_cache_view_node*
dns_cache_view_create(st_dns_info *packet_info,int view_id)
{
    int i,j= 0;
    st_cache_view_node *node = NULL;

    if (packet_info->head->ans_count <= 0)
    {
        return NULL;
    }
    
    node = (st_cache_view_node *)h_malloc(sizeof(st_cache_view_node));
    if (node)
    {
        INIT_LIST_HEAD(&node->list);
        node->standup = 0;
        node->count = packet_info->head->ans_count;
        pthread_mutex_init(&node->lock,NULL);

        int offset = dns_pack_head(node->msg, packet_info->question->Domain,
                strlen(packet_info->question->Domain));
        
        if(offset == 0)
        {
            goto FAILED;
        }
      
        for(i = 0;i < node->count; i++)
        {
            if (packet_info->answer[i].Type == 1)
            {
                /*小于10不作提前更新*/
                if (packet_info->answer[i].TTL < 10)
                {
                    goto FAILED;
                }

                
                if (node->end_time == 0)
                {
                    node->end_time = global_now + packet_info->answer[i].TTL;
                    //node->end_time = global_now + 20;
                    
                }
                node->off[j] = offset;
                offset = dns_pack_answer(node->msg,offset, 
                       packet_info->answer[i].data, packet_info->answer[i].TTL);
                j++;
            }
        }
        node->is_send = 0;
        node->count = j;
        dns_edit_anum(node->msg,j);
        node->msg_len = offset;
        
        return node;
    }

FAILED:
     
    if(node)
    {
        h_free(node);
    }

    return NULL;
}
#endif


static void cache_add_list_tail(struct list_head *tar)
{
    while(g_list_statu)
    {
        usleep(100);
    }
        
    list_add_tail(tar, &g_delete_list);
}


/*该参数需求是保证view指针不为空*/
static void dns_view_destroy(st_cache_view_node *view, st_cache_domain *domain ,int view_id)
{
    assert(domain); 
    assert(view);

    if (pthread_mutex_trylock(&view->lock))
    {
        return;
    }

    if (view->standup == 1)
    {
        //pthread_mutex_unlock(&view->lock);
        return;
    }

    view->standup = 1;
    domain->view[view_id] = NULL;
    view->del_time = global_now + 10;
    cache_add_list_tail(&view->list);

    //pthread_mutex_unlock(&view->lock);
}



static int dns_view_set(char *answer, int answer_len,st_cache_domain *domain,int view_id)
{
    st_cache_view_node *node = NULL;
    
    if (pthread_mutex_trylock(&domain->lock[view_id]))
    {
        hyb_debug("pthread_mutex_trylock failed!\n");
        return -1;
    }
    
    node = (st_cache_view_node *)h_malloc(sizeof(st_cache_view_node));
    if (node)
    {
        INIT_LIST_HEAD(&node->list);
        node->standup = 0;
        pthread_mutex_init(&node->lock,NULL);

        node->is_del = 0;
        node->msg_len = answer_len;
        node->create_time = global_now;
        
        memcpy(node->msg,answer,answer_len);
        if (domain->view[view_id] != NULL)
        {
            dns_view_destroy(domain->view[view_id],domain,view_id);
        }
        domain->view[view_id] = node;

    }
    

    pthread_mutex_unlock(&domain->lock[view_id]);   
    
    return 0;
    
}


#if 0
int walk_test(const void *key, int klen, void *val, void *data)
{
    st_cache_domain *domain = NULL;
    domain = (st_cache_domain *)val;
    hyb_debug("msg_len:%d\n",domain->view[2]->msg_len);
    hyb_debug("count:%d\n",domain->view[2]->count);
    hyb_debug("end_time:%ld\n",domain->view[2]->end_time);
    int i = 0;
    for(i = 0;i< domain->view[2]->count;i++)
    {
        hyb_debug("ttl[%d]:%d\n",i,domain->view[2]->off[i]);
    }

    return 0;
    
}
#endif


static h_hash_st* table_select(unsigned short type)
{
    h_hash_st * select_table = NULL;
    
    switch(type)
    {
    case 0x0001:
        select_table = g_a_table;
        break;
        
    case 0x0002:
        select_table = g_ns_table;
        break;
        
    case 0x0005:
        select_table = g_cname_table;
        break;
                
    case 0x000C:
        select_table = g_ptr_table;
        break;
        
    case 0x000F:
        select_table = g_mx_table;
        break;
        
    case 0x0010:
        select_table = g_txt_table;
        break;

    case 0x001c:
        select_table = g_aaaa_table;
        break;
        
    case 0x00FF:
        select_table = g_any_table;
        break;
        
    default:
        break; 
    }

    return select_table;
}

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
        int answer_len,unsigned short type)
{
    int i = 0;    
    
    if (!domain || !answer ||view_id > MAX_VIEM_NUM )
    {
        hyb_debug("[set cache failed!] illegal input,dns_cache_get failed!\n");
        return -1;
    }
    
    if (g_cache_off)
    {
        hyb_debug("cache off!");
        return -1;
    }

    st_cache_domain *temp = NULL;
    st_cache_domain *new_domain = NULL;

    h_hash_st * cache_table = table_select(type);
    if (!cache_table)
    {
        hyb_debug("Set wrong type cache!\n");
        return -1;
    }
    

    if(h_hash_search(cache_table,domain,domain_len,(void **)&temp) != 0)
    {
        /*hash find failed*/ 
        new_domain = (st_cache_domain *)h_malloc(sizeof(st_cache_domain));
        if (new_domain)
        {
            
            for (i = 0;i < MAX_VIEM_NUM;i++)
            {
                new_domain->view[i] = NULL;
                pthread_mutex_init(&new_domain->lock[i],NULL);
            }
            if (dns_view_set(answer,answer_len,new_domain,view_id))
            {
                hyb_debug("[set cache failed!] reason:dns_view_set error!!\n");
                goto FAILED;
            }

            (void)h_hash_insert(cache_table, domain, domain_len,(void *)new_domain);

            /*
            hyb_debug("[set cache sucess!] - [%s]-[view_id:%d]=>ttl:%ld\n",
                domain,view_id,new_domain->view[view_id]->end_time-global_now);*/
        }
        else
        {
            goto FAILED;
        }
    }
    else
    {
        st_cache_view_node *temp_view = NULL;
        temp_view = temp->view[view_id];
 
        if (dns_view_set(answer,answer_len,temp,view_id))
        {
            hyb_debug("[set cache failed!] reason:dns_view_set error!!\n");
            goto FAILED;
        }


	
    }

            
    hyb_debug("[set cache sucess!] - [%s]-[view_id:%d]-[type:%d]\n",domain,view_id,type);
        
	return 0;

FAILED:
    
    return -1;
}




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
int dns_cache_delete(char *domain,int domain_len,unsigned int view_id,unsigned short type)
{
      
    if (!domain ||view_id > MAX_VIEM_NUM )
    {
        hyb_debug("[set cache failed!] illegal input,dns_cache_get failed!\n");
        return -1;
    }
    
    if (g_cache_off)
    {
        hyb_debug("cache off!");
        return -1;
    }

    st_cache_domain *temp = NULL;
    
    h_hash_st * cache_table = table_select(type);
    if (!cache_table)
    {
        hyb_debug("Set wrong type cache,The wrong type = %d\n", type);
        return -1;
    }

    if(h_hash_search(cache_table,domain,domain_len,(void **)&temp) == 0)
    {
        st_cache_view_node *temp_view = NULL;
        temp_view = temp->view[view_id];
        if (temp_view)
        {
            dns_view_destroy(temp_view, temp ,view_id);
        }
        else
        {
            goto FAILED;
        }
    }
    else
    {
        goto FAILED;	
    }

    return 0;

FAILED:
    
    return -1;
}





/*
static int dns_edit_ttl_check(char* result, int total_len, unsigned short ttl)
{
    char *temp = result+total_len+8;
    int ret = memcmp(temp, &ttl, 2);
    return ret;
}*/


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
int dns_cache_get(char* answer, char* domain, int domain_len, int view_id, 
        unsigned short type)
{  
    if (g_cache_off)
    {
        hyb_debug("cache off!");
        return 0;
    }

    h_hash_st * cache_table = table_select(type);
    if (!cache_table)
    {
        hyb_debug("Get wrong type cache,The wrong type = %d\n", type);
        return -1;
    }
    
    st_cache_domain *temp = NULL;

    if (likely(h_hash_search(cache_table,domain,domain_len,(void **)&temp) == 0))
    {
        st_cache_view_node *view_temp = temp->view[view_id];
        if (likely(view_temp))
        {
                            
            memcpy(answer, view_temp->msg,view_temp->msg_len);               
            return view_temp->msg_len;
            
        }
        else
        {
            st_cache_view_node *view_temp = temp->view[1];
            if (likely(view_temp))
            {
           
                memcpy(answer, view_temp->msg,view_temp->msg_len);        
                return view_temp->msg_len;
            
            }
        }
        
    }

    return 0;
}


static int cache_walk(const void *key, int klen, void *val, void *data)
{
    int i = 0;
    st_cache_domain *domain = (st_cache_domain *)val;
    h_hash_st * cache_table = (h_hash_st *)data;
    int cnt = 0;
    
    for(i = 0; i < MAX_VIEM_NUM; i++)
    {
        
        st_cache_view_node *temp = NULL;
        temp = domain->view[i];
        
        if (pthread_mutex_trylock(&domain->lock[i]))
        {
            continue;
        }
        else
        {
        
            if (!temp)
            {
                cnt ++;
            }
            
            pthread_mutex_unlock(&domain->lock[i]);
        }
        
    }
    
    if(cnt == MAX_VIEM_NUM)
    {
        h_hash_delete(cache_table,key,klen);
    }
    

    return 0;
}



/**
* @brief   遍历内存中所有缓存信息，过期的缓存记录将扔进回收链表
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/10
**/

void dns_cache_check()
{
    /*涉及到delete，所以要锁*/
    g_cache_off = 1;

    hyb_debug("[cache check start!]\n");

    int i = 0;
    for (i=0; i<MAX_TABLE_NUM; i++)
    {
        if (!g_cache_table[i])
        {
            break;
        }

        (void)h_hash_walk(g_cache_table[i], g_cache_table[i],cache_walk);
    }

    g_cache_off = 0;
}



/**
* @brief    清空回收链表
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/07/10
**/
void dns_cache_clear()
{
    if (list_empty(&g_delete_list))
    {
        hyb_debug("[cache clear empty!]\n");
        return;
    }
    else
    {
        hyb_debug("[cache clear start!]\n");
    }

    g_list_statu = 1;
    
    st_cache_view_node *pos =NULL;
    st_cache_view_node *n =NULL;
    list_for_each_entry_safe(pos,n,&g_delete_list,list)
    {
        
        if ((global_now - pos->del_time) > 0 || pos->is_del)
        {
            
            list_del_init(&pos->list);
            pthread_mutex_unlock(&pos->lock); 
            pthread_mutex_destroy(&pos->lock);    
            h_free(pos);
        }
        else
        {
            pos->is_del = 1;
        }
    }

    g_list_statu = 0;
}


/**
* @brief 获取缓存记录条数
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/08/21
**/
int dns_cache_count()
{
    return h_hash_count(g_a_table);
}



/**
* @brief  查看缓存条目存储情况
* @return 
* @remark null
* @see     
* @author hyb      @date 2014/05/13
**/
int dns_cache_info()
{

    int a_cnt = h_hash_count(g_a_table);
    int aaaa_cnt = h_hash_count(g_aaaa_table);
    int mx_cnt = h_hash_count(g_mx_table);
    int ns_cnt = h_hash_count(g_ns_table);
    int cname_cnt = h_hash_count(g_cname_table);
    int txt_cnt = h_hash_count(g_txt_table);
    int ptr_cnt = h_hash_count(g_ptr_table);
    
    hyb_debug("[The A Cache-Table num:%d]\n",a_cnt);
    hyb_debug("[The AAAA Cache-Table num:%d]\n",aaaa_cnt);
    hyb_debug("[The MX Cache-Table num:%d]\n",mx_cnt);
    hyb_debug("[The NS Cache-Table num:%d]\n",ns_cnt);
    hyb_debug("[The CNAME Cache-Table num:%d]\n",cname_cnt);
    hyb_debug("[The TXT Cache-Table num:%d]\n",txt_cnt);
    hyb_debug("[The PTR Cache-Table num:%d]\n",ptr_cnt);


    if (a_cnt || aaaa_cnt || mx_cnt || ns_cnt || cname_cnt || txt_cnt || ptr_cnt)
    {
        return 1;
    }

    return 0;
}

