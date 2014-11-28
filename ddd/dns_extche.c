#define WNS_USE_SYSTEM 1

#include "libbase.h"
#include <netinet/in.h>
#include "dns_extche.h"
#include "dns_pack.h"
#include "dns_unpack.h"
#include <pthread.h>
#include <assert.h>
#include "dns_tool.h"
#include <stdio.h>
#include "dns_comdef.h"
#include "dns_lcllog.h"

#define he_debug(fmt, args ...) \
    do {fprintf(stderr,"[%s][%d][he]:"fmt, __FILE__, __LINE__,##args);}while(0)

#define random(x) (rand()%x)



extern time_t global_now;
//int global_now;

#define MAX_PACKET_LEN      (1500)
#define MAX_NAME_LEN        (256)
#define MAX_NODE_OUTPUT_LEN	(300)
#define MAX_OFF_NUM         (32)
#define MAX_IP_SIZE			(4)

#define MAX_PATH_LEN	(256)
#define MAX_COMMAND_LEN	(512)
#define MAX_LINE_SIZE	(256)
#define MAX_DIVIDE_SIZE	(4)

#define MAX_DOMAIN_LEVEL    (10)
#define EXTEND_MEMORY_FILE	"extend_memory.txt"
#define EXTEND_DIFF_FILE	"extend_differ.txt"

#define DNS_HEAD_SIZE   (12)

static int g_extche_view_node_num;
static int g_extche_view_node_size;
static char * g_extche_view_node_output;

//红黑树
h_rbtree_st * g_a_tree[MAX_DOMAIN_LEVEL];
h_rbtree_st * g_aaaa_tree[MAX_DOMAIN_LEVEL];
h_rbtree_st * g_mx_tree[MAX_DOMAIN_LEVEL];
h_rbtree_st * g_ns_tree[MAX_DOMAIN_LEVEL];
h_rbtree_st * g_cname_tree[MAX_DOMAIN_LEVEL];
h_rbtree_st * g_txt_tree[MAX_DOMAIN_LEVEL];


struct list_head g_extche_delete_list;

struct dns_struct 
{
	unsigned short		id;
	unsigned short		logo;
	unsigned short		query;
	unsigned short		answer;
	unsigned short		autho;
	unsigned short		addit;
	
    unsigned short		q_type;
	unsigned short		q_class;
	
	unsigned short		a_name;
    unsigned short		a_type;
	unsigned short		a_class;
	unsigned short		a_ttl;
	unsigned short		a_fill;
	unsigned short		a_rdlen;
	in_addr_t 			a_rdata;
};


typedef struct extend_ipaddr_msg
{
	char ipaddr[MAX_IP_LEN];
}st_extend_ipaddr_msg;

typedef struct extche_view_node
{ 
	struct list_head list;
	char domain[MAX_NAME_LEN];
	ushort view_id;					//domain 与 view_id用作输出
    int domainlen;
    
    int answer_cnt;
    int auth_cnt;
    int add_cnt;

    int mark[100];
    int mark_num;
    
	char head[DNS_HEAD_SIZE];

    char payload[MAX_PACKET_LEN];
    int payload_size;
    
	unsigned int pkt_size;
    
	unsigned short  standup;
	time_t del_time;
    
}st_extche_view_node;


typedef struct extend_veiw_array
{
    st_extche_view_node *view[MAX_VIEM_NUM];
        
}st_extend_view_array;

typedef struct extend_syn
{
	ushort type;
	char domain[MAX_DOMAIN_LEN];
	ushort view_id;
	char ipaddr[MAX_IP_LEN];
}st_extend_syn;

static int domain_to_q_name(char*domain,int len,char*qname);


static h_rbtree_st* tree_select(unsigned short type,int level)
{
    h_rbtree_st * select_table = NULL;
    
    switch(type)
    {
    case 0x0001:
        select_table = g_a_tree[level];
        break;
        
    case 0x0002:
        select_table = g_ns_tree[level];
        break;
        
    case 0x0005:
        select_table = g_cname_tree[level];
        break;
        
    case 0x000F:
        select_table = g_mx_tree[level];
        break;
        
    case 0x0010:
        select_table = g_txt_tree[level];
        break;

    case 0x001c:
        select_table = g_aaaa_tree[level];
        break;
        
    default:
        break; 
    }

    return select_table;
}



//红黑树比较函数
static int extche_member_compare(const void *key1, uint32_t domain_len1,const void *key2, uint32_t domain_len2)
{
	int i,j;
	char * domain1 = (char*)key1;
	char * domain2 = (char*)key2;
	
    //he_debug("domain1:%s len:%d domian2:%s len:%d\n",domain1,domain_len1,domain2,domain_len2);
	for(i=domain_len1-1,j=domain_len2-1;i>=0&&j>=0;i--,j--)
	{
		if(domain1[i] > domain2[j])
		{
			return 1;
		}
		else if(domain1[i] < domain2[j])
		{
			return -1;
		}
	}

    if (domain_len1 > domain_len2)
    {
        return 1;
    }

    if (j>0)
    {
        if (domain2[j] != '.')
        {
            return -1;
        }
    }
    
	return 0;
}


static void extche_member_destroy(void *data)
{
    assert(data);
    int i = 0;
    
    st_extend_view_array *domain = (st_extend_view_array *)data;
    for (i = 0; i < MAX_VIEM_NUM; i++)
    {

        if (domain->view[i])
        {
            h_free(domain->view[i]);
            domain->view[i] = NULL;
        }
    }
    h_free(domain);

    return;
}



static int 
extche_view_create(st_extend_view_array *domain_array,char*domain,int domain_len,
        ushort view_id,char *pkt,int pkt_len)
{
    int j = 0,i = 0;
	int errno;
    st_extche_view_node *node = NULL;
    
    node = (st_extche_view_node *)h_malloc(sizeof(st_extche_view_node));
    if (node)
    {
        INIT_LIST_HEAD(&node->list);
		errno = memcpy_safe(node->domain, MAX_DOMAIN_LEN, domain, domain_len);
		if(errno != 0)
		{
			goto FAILED;
		}

        errno = memcpy_safe(node->head, DNS_HEAD_SIZE, pkt, 12);
		if(errno != 0)
		{
			goto FAILED;
		}

        struct MAIN_DNS_HEADER*head = (struct MAIN_DNS_HEADER*)pkt; 
        
        node->auth_cnt = ntohs(head->auth_count);
        node->answer_cnt = ntohs(head->ans_count); 
        node->add_cnt = ntohs(head->add_count);

        //hyb_debug("node->auth_cnt:%d ans:%d add:%d\n",node->auth_cnt,node->answer_cnt,node->add_cnt);
        char d[256] = {0};
        int dlen = domain_to_q_name(domain, domain_len, d);

        //hyb_debug("domain:%s %s,dlen:%d\n", domain ,node->domain, dlen);
        
        /*存储时候的域名省略了*.*/
        //dlen += 2;

        
        char *ptr = pkt;

        ptr += 12+dlen;

        node->payload_size = pkt_len-12-dlen;
         
        errno = memcpy_safe(node->payload,MAX_PACKET_LEN, ptr, node->payload_size);
		if(errno != 0)
		{
			goto FAILED;
		}
        
        #if 0  
        if(node->answer_cnt != 0)
        {
            int i;
            for(i = 0;i < node->answer_cnt;i++)
            {
                int length = DNS_Get_Name_Position(pkt,ptr,pkt_len,&node->mark[j]);
                if(length == -1)
                {
                	printf("checkLength error.\n");
                    goto FAILED;
                }
                node->mark[j] += offset;
                j++;
                offset +=length;
                ptr += length;
            }
        }

        
        
		if(node->auth_cnt != 0)
        {
            int i;
            for(i = 0;i<node->auth_cnt ;i++)
            {
                int length = DNS_Get_Name_Position(pkt,ptr,pkt_len,&node->mark[j]);
                if(length == -1)
                {
                	printf("checkLength error.\n");
                    goto FAILED;
                }
                node->mark[j] += offset;
                j++;
                offset +=length;
                ptr += length;
            }
        }
        
		if(node->add_cnt != 0)
        {
            int i;
            for(i = 0;i< node->add_cnt;i++)
            {
                int length = DNS_Get_Name_Position(pkt,ptr,pkt_len,&node->mark[j]);
                if(length == -1)
                {
                	printf("checkLength error.\n");
                    goto FAILED;
                }
                node->mark[j] += offset;
                j++;
                offset +=length;
                ptr += length;

            }
        }
        #endif

        for(i = 0 ; i < node->payload_size;i++)
        {
            char h1[10] = {0};
            char h2[10] = {0};
            sprintf_n(h1,10,"%02hhx",ptr[i]);
            if(strcmp(h1,"c0") == 0)
            {
                sprintf_n(h2,10,"%02hhx",ptr[i+1]);
                if(strcmp(h2,"0c") != 0)
                {
                    node->mark[j] = i+1;
                    j++;
                }
            }
        }

        /*
        for (i = 0 ; i < j ; i++)
        {
            hyb_debug("mark[%d]:%d [%02hhx]\n",i,node->mark[i],node->payload[node->mark[i]]);
        }*/

        node->mark_num = j;
		node->view_id = view_id;
        node->domainlen = domain_len;
        domain_array->view[view_id] = node;
        
        return 0;
    }

FAILED:
	if(node != NULL)
	{
		h_free(node);
	}

    return -1;   
}

static int get_domain_level(char *str,int strlen)
{
    int i = 0;
    int count = 0;
    for(i = 0;i < strlen;i++)
    {
        if(str[i] == '.')
        {
            count ++;
        }
    }
    if (count > MAX_DOMAIN_LEVEL)
    {
        return MAX_DOMAIN_LEVEL-1;
    }
    if (count > 0)
    {
        return count-1;
    }
    return 0;
}
static
st_extche_view_node * get_extche_veiw_node(char * domain,int domain_len,ushort view_id,unsigned short type)
{
    int i = 0;
	st_extend_view_array *temp = NULL;
	st_extche_view_node*node = NULL;

    int level = get_domain_level(domain,domain_len);
    for (i=level;i >= 0;i--)
    {
        h_rbtree_st * tree = tree_select(type,i);
    if (!tree)
    {
            continue;
    }
        
	if(h_rbtree_search(tree,domain,domain_len,(void **)&temp) != 0)
	{
		/*hash find failed*/
		//he_debug("[get_extend_veiw_node] extend not exist,get_extend_veiw_node failed!\n");
		    continue;
	}
	
	node = temp->view[view_id];

    if (node)
    {
	    return node;
    }
    else
    {
        return temp->view[1];
    }
    }
    return NULL;
}

static 
int extend_view_display(const void *key, int klen, void *val, void *data)
{
    int i,len;
	char node_output[MAX_NODE_OUTPUT_LEN] = {0};
    st_extend_view_array *domain = (st_extend_view_array*)val;
	
    for(i = 0; i < MAX_VIEM_NUM; i++)
    {
        st_extche_view_node*temp = NULL;
        temp = domain->view[i];
        if (!temp)
        {
            continue;
        }
        else
        { 
			len = sprintf_n(node_output,MAX_NODE_OUTPUT_LEN,"\n%s|%d|",temp->domain,temp->view_id);
			if(len<=0)
			{
				goto FAILED;
			}
			
			strcat_n(g_extche_view_node_output,g_extche_view_node_size,node_output);
        }
    }

    return 0;
FAILED:
	he_debug("[extend_view_display] output error,extend_view_display failed!\n");
	return -1;

}

static 
int extend_view_count(const void *key, int klen, void *val, void *data)
{
    int i = 0;
    st_extend_view_array *domain = (st_extend_view_array*)val;
    for(i = 0; i < MAX_VIEM_NUM; i++)
    {
        st_extche_view_node*temp = NULL;
        temp = domain->view[i];
        if (!temp)
        {
            continue;
        }
        else
        { 
			g_extche_view_node_num++;
        }
    }

    return 0;
}


/**
* @brief    初始化域名表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
int dns_ext_cache_init()
{

    INIT_LIST_HEAD(&g_extche_delete_list);
    int i = 0;
    for(i=0;i<MAX_DOMAIN_LEVEL;i++)
    {
        g_a_tree[i] = h_rbtree_create(extche_member_destroy,extche_member_compare);
        if (!g_a_tree[i])
    {
        he_debug("[dns extend init fail!]\n");
        //extend_syn_from_mysql();
        return -1;
    }
        g_aaaa_tree[i] = h_rbtree_create(extche_member_destroy,extche_member_compare);
        if (!g_aaaa_tree[i])
    {
        he_debug("[dns extend init fail!]\n");
        //extend_syn_from_mysql();
        return -1;
    }
        g_cname_tree[i] = h_rbtree_create(extche_member_destroy,extche_member_compare);
        if (!g_cname_tree[i])
    {
        he_debug("[dns extend init fail!]\n");
        //extend_syn_from_mysql();
        return -1;
    }
        g_txt_tree[i] = h_rbtree_create(extche_member_destroy,extche_member_compare);
        if (!g_txt_tree[i])
    {
        he_debug("[dns extend init fail!]\n");
        //extend_syn_from_mysql();
        return -1;
    }
        g_mx_tree[i] = h_rbtree_create(extche_member_destroy,extche_member_compare);
        if (!g_mx_tree[i])
    {
        he_debug("[dns extend init fail!]\n");
        //extend_syn_from_mysql();
        return -1;
    }
        g_ns_tree[i] = h_rbtree_create(extche_member_destroy,extche_member_compare);
        if (!g_ns_tree[i])
    {
        he_debug("[dns extend init fail!]\n");
        //extend_syn_from_mysql();
        return -1;
    }
    }
    return 0;
}


/**
* @brief    清理域名表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_ext_cache_destroy()
{
	st_extche_view_node *pos =NULL;
    st_extche_view_node *n =NULL;
    int i = 0;
    
    for(i=0;i<MAX_DOMAIN_LEVEL;i++)
    {
        if (g_a_tree[i])
    {
            h_rbtree_destroy(g_a_tree[i]);
            g_a_tree[i] = NULL;
    }   

        if (g_aaaa_tree[i])
    {
            h_rbtree_destroy(g_aaaa_tree[i]);
            g_aaaa_tree[i] = NULL;
    }
    
        if (g_cname_tree[i])
    {
            h_rbtree_destroy(g_cname_tree[i]);
            g_cname_tree[i] = NULL;
    }
    
        if (g_mx_tree[i])
    {
            h_rbtree_destroy(g_mx_tree[i]);
            g_mx_tree[i] = NULL;
    } 
    
        if (g_ns_tree[i])
    {
            h_rbtree_destroy(g_ns_tree[i]);
            g_ns_tree[i] = NULL;
    } 

        if (g_txt_tree[i])
    {
            h_rbtree_destroy(g_txt_tree[i]);
            g_txt_tree[i] = NULL;
        } 
    } 
    list_for_each_entry_safe(pos,n,&g_extche_delete_list,list)
    {
    	list_del(&pos->list);
        h_free(pos);
    }
}

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
        int pkt_len,unsigned short type)
{
	if(domain == NULL)
	{
		he_debug("[create extend failed!] illegal input,dns_extend_create failed!\n");
		return -1;
	}

	st_extend_view_array *temp = NULL;
    st_extend_view_array *new_domain = NULL;

    int level = get_domain_level(domain,domain_len);
    h_rbtree_st * tree = tree_select(type,level);
    if (!tree)
    {
        return -1;
    }

	if(h_rbtree_search(tree,domain,domain_len,(void **)&temp) != 0)
	{
		/*rbtree find failed*/
		new_domain = (st_extend_view_array *)h_malloc(sizeof(st_extend_view_array));
        if (!new_domain)
        {
        	he_debug("[create extend failed!] malloc fail,dns_extend_create failed!\n");
			return -1;
        }
        /*这里根据匹配规则，对域名作缩减*/
		(void)h_rbtree_insert(tree,domain+2 ,domain_len-2 ,(void *)new_domain);
        
		if(extche_view_create(new_domain,domain,domain_len,view_id,pkt,pkt_len) != 0)
		{
			he_debug("[create extend failed!] extend_view_create fail,dns_extend_create failed!\n");
			return -1;
		}
	}
	else
	{
		if(temp->view[view_id] != NULL)
		{
			he_debug("[create extend failed!] extend exist,dns_extend_create failed!\n");
			return -1;
		}
		if(extche_view_create(temp,domain,domain_len,view_id,pkt,pkt_len) != 0)
		{
			he_debug("[create extend failed!] extend_view_create fail,dns_extend_create failed!\n");
			return -1;
		}
	}
    
	hyb_debug("[set extche sucess!] - [%s]-[view_id:%d]-[type:%d]\n",domain,view_id,type);
	return 0;
}

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
int dns_ext_cache_drop(char *domain,int domain_len,ushort view_id,unsigned short type)
{
	st_extend_view_array *temp = NULL;
	st_extche_view_node*node = NULL;

	if(domain == NULL)
	{
		he_debug("[drop extend failed!] illegal input,dns_extend_create failed!\n");
		return -1;
	}

    int level = get_domain_level(domain,domain_len);
    h_rbtree_st * tree = tree_select(type,level);
    if (!tree)
    {
        return -1;
    }

	if(h_rbtree_search(tree,domain,domain_len,(void **)&temp) != 0)
	{
		/*rbtree find failed*/
		he_debug("[drop extend failed!] extend not exist,get_extend_veiw_node failed!\n");
		return -1;
	}
	
	node = temp->view[view_id];
	if(node == NULL)
	{
		he_debug("[drop extend failed!] view not exist,dns_extend_drop failed!\n");
		return -1;
	}

	node->standup = 1;
    temp->view[view_id] = NULL;
    node->del_time = time(0) + 3;
    list_add_tail(&node->list, &g_extche_delete_list);
    he_debug("drop extend[%s]->view[%d]\n",domain,view_id);
	return 0;
}

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
int dns_ext_cache_del(char *domain,int domain_len,unsigned short type)
{
	int i;
	st_extend_view_array *temp = NULL;
	st_extche_view_node*node = NULL;

	if(domain == NULL)
	{
		he_debug("[delete extend failed!] illegal input,dns_domain_create failed!\n");
		return -1;
	}
    
    int level = get_domain_level(domain,domain_len);
    h_rbtree_st * tree = tree_select(type,level);
    if (!tree)
    {
        return -1;
    }

	if(h_rbtree_search(tree,domain,domain_len,(void **)&temp) != 0)
	{
		/*hash find failed*/
		he_debug("[delete extend failed!] domain not exist,get_domain_veiw_node failed!\n");
		return -1;
	}

	for(i=0;i<MAX_VIEM_NUM;i++)
	{
		node = temp->view[i];
		if(node == NULL)
		{
			continue;
		}

		node->standup = 1;
	    temp->view[i] = NULL;
	    node->del_time = time(0) + 3;
	    list_add_tail(&node->list, &g_extche_delete_list);
	    
	}

	h_rbtree_delete(tree,domain,domain_len);
	he_debug("delete extend[%s]->view[all]\n",domain);
	
	return 0;
}


static int domain_to_q_name(char*domain,int len,char*qname)
{
    assert(domain);
    assert(qname);
    
	char temp_domain[256];
	memcpy_s(temp_domain,domain,len);
	temp_domain[len] = 0;
	char temp[1024];
	int count = 0;
	char *delims = ".";
	char *myStrBuf=NULL;
	char *p = strtok_r(temp_domain,delims,&myStrBuf);
	while(p!=NULL)
	{
		int size = strlen(p);
		temp[count] = (unsigned char)size;
		memcpy_s(&temp[count+1],p,size);
		count = count+size+1;
		p = strtok_r(NULL,delims,&myStrBuf);
	}
	temp[count] = 0;
	count++;
	memcpy_s(qname,temp,count);
	return count;
}


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
int dns_ext_cache_get(char *packet,char *domain,int domain_len,ushort view_id,
        unsigned short type,int pid)
{
	st_extche_view_node*node = NULL;
	
	node = get_extche_veiw_node(domain,domain_len,view_id, type);
	if(likely(node == NULL))
	{
		//he_debug("[dns_extend_get] view not exist,dns_extend_get failed!\n");
		return 0;
	}

	if(node->payload_size == 0)
	{
		return 0;
	}

    /*防止解析@记录*/
    if(domain_len == (node->domainlen-2))
    {
        return 0;
    }
    
    char ext_domain[MAX_NAME_LEN] = {0};
    char use_domain[MAX_NAME_LEN] = {0};
    
    memset(packet, 0, MAX_PACKET_LEN);
    /*头大小*/
    memcpy(packet,node->head,12);

    int ext_domainlen = domain_to_q_name(node->domain,node->domainlen,ext_domain);
    int use_domainlen = domain_to_q_name(domain,domain_len,use_domain);

    //ext_domainlen +=2;
    packet +=12; 
    
    memcpy(packet,use_domain,use_domainlen);

    packet += use_domainlen;

    memcpy(packet,node->payload,node->payload_size);
    
    int offset = use_domainlen - ext_domainlen;
    int limit = node->mark_num;
    int i = 0;
    for(i = 0; i < limit; i++)
    {
        packet[node->mark[i]] += offset;
    }

   
   // hyb_debug("[set extche sucess!] - [%s]-[view_id:%d]-[type:%d]\n",domain,view_id,type);

    dns_lcllog_reqcnt_log(node->domain,node->domainlen,view_id,pid);
    
	return (node->payload_size+12+use_domainlen);
    
   //return 0;
}


#if 0
/**
* @brief   遍历内存中所有域名信息，生成内存状态输出
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/

int dns_ext_cache_node_display(char*output,int max_len)
{
	int errno;
	g_extche_view_node_size = max_len;
	g_extche_view_node_output = h_malloc(max_len);
	if(g_extche_view_node_output == NULL)
	{
		he_debug("g_extche_view_node_output is NULL.\n");
		return -1;
	}
	(void)h_rbtree_walk(g_extche_tree,NULL,extend_view_display);
	errno = memcpy_safe(output,max_len,g_extche_view_node_output,strlen(g_extche_view_node_output));
	h_free(g_extche_view_node_output);
	if(errno != 0)
	{
		he_debug("memcpy_safe fail.\n");
		return -1;
	}
	return 0;
}
#endif


/**
* @brief    清空回收链表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_ext_cache_clear()
{
	st_extche_view_node*pos =NULL;
    st_extche_view_node *n =NULL;
    list_for_each_entry_safe(pos,n,&g_extche_delete_list,list)
    {
        //hyb_debug("[cache free: [%s]!]\n",pos->domain);
        if ((time(0) - pos->del_time) > 0)
        {
            list_del(&pos->list);
            h_free(pos);
        }
    }
}



