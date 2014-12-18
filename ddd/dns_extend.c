#define WNS_USE_SYSTEM 1

#include "libbase.h"
#include <netinet/in.h>
#include "dns_extend.h"
#include "dns_pack.h"
#include "dns_unpack.h"
#include <pthread.h>
#include <assert.h>
#include "dns_tool.h"
#include <stdio.h>


#define he_debug(fmt, args ...) \
    do {fprintf(stderr,"[%s][%d][he]:"fmt, __FILE__, __LINE__,##args);}while(0)

#define random(x) (rand()%x)



extern time_t global_now;
//int global_now;

#define MAX_NAME_LEN        (256)
#define MAX_NODE_OUTPUT_LEN	(300)
#define MAX_OFF_NUM         (32)
#define MAX_IP_SIZE			(4)

#define MAX_PATH_LEN	(256)
#define MAX_COMMAND_LEN	(512)
#define MAX_LINE_SIZE	(256)
#define MAX_DIVIDE_SIZE	(4)

#define EXTEND_MEMORY_FILE	"extend_memory.txt"
#define EXTEND_DIFF_FILE	"extend_differ.txt"



static int g_extend_view_node_num;
static int g_extend_view_node_size;
static char * g_extend_view_node_output;

//红黑树
h_rbtree_st * g_extend_tree;


struct list_head g_extend_delete_list;

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

typedef struct extend_view_node
{ 
	struct list_head list;
	char domain[MAX_NAME_LEN];
	ushort view_id;					//domain 与 view_id用作输出
	st_extend_ipaddr_msg msg_array[MAX_IP_SIZE];
	int msg_size;
	unsigned short  standup;
	time_t del_time;
    
}st_extend_view_node;


typedef struct extend_veiw_array
{
    st_extend_view_node *view[MAX_VIEM_NUM];
        
}st_extend_view_array;

typedef struct extend_syn
{
	ushort type;
	char domain[MAX_DOMAIN_LEN];
	ushort view_id;
	char ipaddr[MAX_IP_LEN];
}st_extend_syn;


static 
int extend_memory_to_file()
{
	int result;
	char path[MAX_PATH_LEN] = {0};    
	char file[MAX_PATH_LEN] = {0};    
	int number = dns_extend_node_count()+1;
	int size = number*MAX_DOMAIN_LEN;
	char output[size];

	if(getcwd(path,MAX_PATH_LEN) == NULL)
	{
		he_debug("get directory fail.\n");
		return -1;
	}
	result = sprintf_n(file,MAX_PATH_LEN,"%s/read_domain_view/%s", path,EXTEND_MEMORY_FILE);
	if(result <= 0)
	{
		he_debug("get file fail.\n");
		return -1;
	}
	memset(output,0,size);
	result = dns_extend_node_display(output,size);
	if(result < 0)
	{
		he_debug("dns_extend_node_display fail.\n");
		return -1;
	}
	result = write_file(file,output,strlen(output));
	return result;
}


static 
int extend_exe_php()
{
	char command[MAX_COMMAND_LEN] = {0};
	char path[MAX_PATH_LEN] = {0}; 
	int result;
	
	if(getcwd(path,sizeof(path)) == NULL)
	{
		return -1;
	}
	result = sprintf_n(command,MAX_COMMAND_LEN, "php -c /etc/php.ini %s/read_domain_view/make_extend_differ.php", path);
	if(result < 0)
	{
		return -1;
	}
	system(command);
	return 0;
}

static 
void extend_syn_exec(st_extend_syn * syn)
{
	if(syn->type == 1)
	{
		dns_extend_create(syn->domain,strlen(syn->domain),syn->view_id);
	}
	else if(syn->type == 2)
	{
		dns_extend_insert(syn->domain,strlen(syn->domain),syn->view_id,syn->ipaddr);
	}
	else if(syn->type == 3)
	{
		dns_extend_remove(syn->domain,strlen(syn->domain),syn->view_id,syn->ipaddr);
	}
	else if(syn->type == 4)
	{
		dns_extend_drop(syn->domain,strlen(syn->domain),syn->view_id);
	}
}

static 
int extend_file_to_memory()
{
	FILE *file;
	char temp[MAX_LINE_SIZE];
	char parts[MAX_DIVIDE_SIZE][MAX_LINE_SIZE];
	st_extend_syn extend_syn;
	char path[MAX_PATH_LEN] = {0};  
	char filename[MAX_PATH_LEN];
	int result;

	if(getcwd(path,MAX_PATH_LEN) == NULL)
	{
		return -1;
	}
	result = sprintf_n(filename,MAX_PATH_LEN,"%s/read_domain_view/%s", path,EXTEND_DIFF_FILE);
	if(result <= 0)
	{
		return -1;
	}
	file=fopen(filename,"r");
	
	while(fgets(temp, MAX_LINE_SIZE, file))
	{
		temp[strlen(temp)-1] = 0;
		memset(parts,0,sizeof(parts));
		char *myStrBuf=NULL;
		char * p=strtok_r(temp,"|",&myStrBuf); 
		int i = 0;
		while(p!=NULL && i<MAX_DIVIDE_SIZE)
		{  
			memcpy(parts[i],p,MAX_LINE_SIZE);
			p=strtok_r(NULL,"|",&myStrBuf);  
			i++;
		}

		extend_syn.type = atoi(parts[0]);
		if(extend_syn.type < 1 && extend_syn.type > 4)
		{
			continue;
		}
		
		memcpy(extend_syn.domain,parts[1],MAX_DOMAIN_LEN);

		extend_syn.view_id = atoi(parts[2]);
		if(extend_syn.view_id < 0 || extend_syn.view_id>MAX_VIEM_NUM)
		{
			continue;
		}
		
		memcpy(extend_syn.ipaddr,parts[3],MAX_IP_LEN);
		
		extend_syn_exec(&extend_syn);
	}
	fclose(file);
	return 0;
}

static 
int extend_syn_from_mysql()
{
	int result;
	
	//输出当前状态
	result = extend_memory_to_file();
	if(result != 0)
	{
		he_debug("extend_memory_to_file fail.");
		return -1;
	}

	//调用php生成差异化文件
	result = extend_exe_php();
	if(result != 0)
	{
		he_debug("extend_exe_php fail.");
		return -1;
	}

	//读差异化文件，修改内存
	extend_file_to_memory();
	
	return 0;
}


//红黑树比较函数
int extend_member_compare(const void *key1, uint32_t domain_len1,const void *key2, uint32_t domain_len2)
{
	int i,j;
	char * domain1 = (char*)key1;
	char * domain2 = (char*)key2;
	
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
	return 0;
}


static void extend_member_destroy(void *data)
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
extend_view_create(st_extend_view_array *domain_array,char*domain,int domain_len,ushort view_id)
{
	int errno;
    st_extend_view_node *node = NULL;
    
    node = (st_extend_view_node *)h_malloc(sizeof(st_extend_view_node));
    if (node)
    {
        INIT_LIST_HEAD(&node->list);
		errno = memcpy_safe(node->domain,MAX_DOMAIN_LEN,domain,domain_len);
		if(errno != 0)
		{
			goto FAILED;
		}
		node->view_id = view_id;
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

static
st_extend_view_node * get_extend_veiw_node(char * domain,int domain_len,ushort view_id)
{
	st_extend_view_array *temp = NULL;
	st_extend_view_node*node = NULL;

	if(h_rbtree_search(g_extend_tree,domain,domain_len,(void **)&temp) != 0)
	{
		/*hash find failed*/
		//he_debug("[get_extend_veiw_node] extend not exist,get_extend_veiw_node failed!\n");
		return NULL;
	}
	
	node = temp->view[view_id];
	return node;
}

/*
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
*/

static
int make_dns_answer(char result[MAX_PACKET_LEN],char*domain,int domain_len,char ip[MAX_IP_LEN])
{
	int errno;
	struct dns_struct temp;
	struct dns_struct *dns_query = &temp;
	
	dns_query->logo = htons(0x8180);
	dns_query->query = htons(0x0001);
	dns_query->answer = htons(0x0001);
    //TO EDIT
	dns_query->autho = 0;
	dns_query->addit = 0;
	
	errno = memcpy_safe(result,MAX_PACKET_LEN,dns_query,12);
	if(errno != 0)
	{
		return -1;
	}
	
	char qname[MAX_DOMAIN_LEN];
	int len = domain_to_q_name(domain,domain_len,qname);
	if(len<=0)
	{
		return -1;
	}
	
	errno = memcpy_safe(result+12,MAX_PACKET_LEN-12,qname,len);
	if(errno != 0)
	{
		return -1;
	}
	int total_len = 12+len;

	dns_query->q_type = htons(0x0001);
	dns_query->q_class = htons(0x0001);

	dns_query->a_name = htons(0xc00c);
	dns_query->a_type = htons(0x0001);
	dns_query->a_class = htons(0x0001);
	dns_query->a_ttl = htons(0x0000);
	dns_query->a_fill = htons(0x0000);
	dns_query->a_rdlen = htons(0x0004);
	dns_query->a_rdata = inet_addr(ip);
	errno = memcpy_safe(result+total_len,MAX_PACKET_LEN-total_len, &dns_query->q_type, 20);
	if(errno != 0)
	{
		return -1;
	}
	
	total_len = total_len+20;
	
	return total_len;
}


static int
extend_view_ip_insert(st_extend_view_node *node,char *domain,int domain_len,char ipaddr[MAX_IP_LEN])
{
	int i;

	if(node->msg_size >= MAX_IP_SIZE)
	{
		return -1;
	}
	
	for(i=0;i<node->msg_size;i++)
	{
		if(memcmp(node->msg_array[i].ipaddr,ipaddr,MAX_IP_LEN) != 0)
		{
			continue;
		}
		else
		{
			return -1;
		}
	}

	memset(&(node->msg_array[node->msg_size]),0,sizeof(st_extend_ipaddr_msg));
	memcpy(node->msg_array[node->msg_size].ipaddr,ipaddr,MAX_IP_LEN);

	node->msg_size++;
	return 0;
}

static int
extend_view_ip_remove(st_extend_view_node *node,char *domain,int domain_len,char ipaddr[MAX_IP_LEN])
{
	int i;
	
	for(i=0;i<node->msg_size;i++)
	{
		if(memcmp(node->msg_array[i].ipaddr,ipaddr,MAX_IP_LEN) != 0)
		{
			continue;
		}
		else
		{
			if(i<node->msg_size-1)
			{
				memcpy(&(node->msg_array[i]),&(node->msg_array[node->msg_size-1]),sizeof(st_extend_ipaddr_msg));
			}
			node->msg_size--;
			return 0;
		}
	}

	return -1;
}

static 
int extend_view_display(const void *key, int klen, void *val, void *data)
{
    int i,j,len;
	char node_output[MAX_NODE_OUTPUT_LEN] = {0};
	char ipaddr_output[MAX_IP_LEN+4] = {0};
    st_extend_view_array *domain = (st_extend_view_array*)val;
	
    for(i = 0; i < MAX_VIEM_NUM; i++)
    {
        st_extend_view_node*temp = NULL;
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
			for(j=0;j<temp->msg_size;j++)
			{
				len = sprintf_n(ipaddr_output,MAX_NODE_OUTPUT_LEN,"%s ",temp->msg_array[j].ipaddr);
				if(len<=0)
				{
					goto FAILED;
				}
				strcat_n(node_output,MAX_NODE_OUTPUT_LEN,ipaddr_output);
			}
			
			strcat_n(g_extend_view_node_output,g_extend_view_node_size,node_output);
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
        st_extend_view_node*temp = NULL;
        temp = domain->view[i];
        if (!temp)
        {
            continue;
        }
        else
        { 
			g_extend_view_node_num++;
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
int dns_extend_init()
{
	g_extend_tree = NULL;

    INIT_LIST_HEAD(&g_extend_delete_list);
    g_extend_tree = h_rbtree_create(extend_member_destroy,extend_member_compare);
    if (g_extend_tree)
    {
        he_debug("[dns extend init sucess!]\n");
        //extend_syn_from_mysql();
        return 0;
    }

    return -1;
}

/**
* @brief    同步mysql到内存
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_extend_syn()
{
	extend_syn_from_mysql();
}


/**
* @brief    清理域名表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_extend_destroy()
{
	st_extend_view_node *pos =NULL;
    st_extend_view_node *n =NULL;
    if (g_extend_tree)
    {
        h_rbtree_destroy(g_extend_tree);
        g_extend_tree = NULL;
    }   
	
    list_for_each_entry_safe(pos,n,&g_extend_delete_list,list)
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
int dns_extend_create(char *domain,int domain_len,ushort view_id)
{
	if(domain == NULL)
	{
		he_debug("[create extend failed!] illegal input,dns_extend_create failed!\n");
		return -1;
	}

	st_extend_view_array *temp = NULL;
    st_extend_view_array *new_domain = NULL;

	if(h_rbtree_search(g_extend_tree,domain,domain_len,(void **)&temp) != 0)
	{
		/*rbtree find failed*/
		new_domain = (st_extend_view_array *)h_malloc(sizeof(st_extend_view_array));
        if (!new_domain)
        {
        	he_debug("[create extend failed!] malloc fail,dns_extend_create failed!\n");
			return -1;
        }
		(void)h_rbtree_insert(g_extend_tree,domain,domain_len ,(void *)new_domain);
		if(extend_view_create(new_domain,domain,domain_len,view_id) != 0)
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
		if(extend_view_create(temp,domain,domain_len,view_id) != 0)
		{
			he_debug("[create extend failed!] extend_view_create fail,dns_extend_create failed!\n");
			return -1;
		}
	}
	he_debug("create extend[%s]->view[%d]\n",domain,view_id);
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
int dns_extend_drop(char *domain,int domain_len,ushort view_id)
{
	st_extend_view_array *temp = NULL;
	st_extend_view_node*node = NULL;

	if(domain == NULL)
	{
		he_debug("[drop extend failed!] illegal input,dns_extend_create failed!\n");
		return -1;
	}

	if(h_rbtree_search(g_extend_tree,domain,domain_len,(void **)&temp) != 0)
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
    list_add_tail(&node->list, &g_extend_delete_list);
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
int dns_extend_del(char *domain,int domain_len)
{
	int i;
	st_extend_view_array *temp = NULL;
	st_extend_view_node*node = NULL;

	if(domain == NULL)
	{
		he_debug("[delete extend failed!] illegal input,dns_domain_create failed!\n");
		return -1;
	}

	if(h_rbtree_search(g_extend_tree,domain,domain_len,(void **)&temp) != 0)
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
	    list_add_tail(&node->list, &g_extend_delete_list);
	    
	}

	h_rbtree_delete(g_extend_tree,domain,domain_len);
	he_debug("delete extend[%s]->view[all]\n",domain);
	
	return 0;
}


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
int dns_extend_insert(char *domain,int domain_len,ushort view_id,char ipaddr[MAX_IP_LEN])
{
	st_extend_view_node*node = NULL;

	if(domain == NULL || ipaddr == NULL)
	{
		he_debug("[insert extend ip failed!] illegal input,dns_extend_insert failed!\n");
		return -1;
	}
	
	node = get_extend_veiw_node(domain,domain_len,view_id);
	if(node == NULL)
	{
		he_debug("[insert extend ip failed!] view not exist,dns_extend_insert failed!\n");
		return -1;
	}

	if(extend_view_ip_insert(node,domain,domain_len,ipaddr) != 0)
	{
		he_debug("[insert extend ip failed!] insert ip error,dns_extend_insert failed!\n");
		return -1;
	}
	he_debug("insert ip[%s] into extend[%s]->view[%d]\n",ipaddr,domain,view_id);
	return 0;
}

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
int dns_extend_remove(char *domain,int domain_len,ushort view_id,char ipaddr[MAX_IP_LEN])
{
	st_extend_view_node*node = NULL;
	
	if(domain == NULL || ipaddr == NULL)
	{
		he_debug("[dns_extend_remove] illegal input!\n");
		return -1;
	}
	
	node = get_extend_veiw_node(domain,domain_len,view_id);
	if(node == NULL)
	{
		he_debug("[dns_extend_remove] view not exist!\n");
		return -1;
	}

	if(extend_view_ip_remove(node,domain,domain_len,ipaddr) != 0)
	{
		he_debug("[dns_extend_remove] remove ip error!\n");
		return -1;
	}
	else
	{
		if(node->msg_size <=0)
		{
			dns_extend_drop(domain,domain_len,view_id);
		}
	}
	he_debug("remove ip[%s] from extend[%s]->view[%d]\n",ipaddr,domain,view_id);
	return 0;
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
int dns_extend_get(char *packet,char *domain,int domain_len,ushort view_id)
{
	int index,packet_len;
	st_extend_view_node*node = NULL;

    /*
	if(unlikely(domain == NULL))
	{
		he_debug("[dns_extend_get] illegal input,dns_extend_get failed!\n");
		return 0;
	}
	*/
	
	node = get_extend_veiw_node(domain,domain_len,view_id);
	if(likely(node == NULL))
	{
		//he_debug("[dns_extend_get] view not exist,dns_extend_get failed!\n");
		return 0;
	}

	if(node->msg_size == 0)
	{
		return 0;
	}
	
	index = random(node->msg_size);
	//he_debug("[dns_extend_get] extend veiw hold:%s--%d--%s\n",extend,view_id,node->msg_array[index].ipaddr);
	packet_len = make_dns_answer(packet,domain,domain_len,node->msg_array[index].ipaddr);
	if(packet_len <= 0)
	{
		return -1;
	}
	return packet_len;
}


/**
* @brief   遍历内存中所有域名信息，计算node数目
* @return node 数目
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/

int dns_extend_node_count()
{
	int errno;
	g_extend_view_node_num = 0;
	(void)h_rbtree_walk(g_extend_tree,NULL,extend_view_count);
	return g_extend_view_node_num;
}


/**
* @brief   遍历内存中所有域名信息，生成内存状态输出
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/

int dns_extend_node_display(char*output,int max_len)
{
	int errno;
	g_extend_view_node_size = max_len;
	g_extend_view_node_output = h_malloc(max_len);
	if(g_extend_view_node_output == NULL)
	{
		he_debug("g_extend_view_node_output is NULL.\n");
		return -1;
	}
	(void)h_rbtree_walk(g_extend_tree,NULL,extend_view_display);
	errno = memcpy_safe(output,max_len,g_extend_view_node_output,strlen(g_extend_view_node_output));
	h_free(g_extend_view_node_output);
	if(errno != 0)
	{
		he_debug("memcpy_safe fail.\n");
		return -1;
	}
	return 0;
}



/**
* @brief    清空回收链表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_extend_clear()
{
	st_extend_view_node*pos =NULL;
    st_extend_view_node *n =NULL;
    list_for_each_entry_safe(pos,n,&g_extend_delete_list,list)
    {
        //hyb_debug("[cache free: [%s]!]\n",pos->domain);
        if ((time(0) - pos->del_time) > 0)
        {
            list_del(&pos->list);
            h_free(pos);
        }
    }
}



