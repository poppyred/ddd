#define WNS_USE_SYSTEM 1

#include <stdlib.h>
#include "libbase.h"
#include <netinet/in.h>
#include "dns_domain.h"
#include "dns_pack.h"
#include "dns_unpack.h"
#include <pthread.h>
#include <assert.h>
#include "dns_tool.h"


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

#define DEFAULT_DOMAIN_IPADDR	"0.0.0.0"

#define DOMAIN_MEMORY_FILE	"domain_memory.txt"
#define DOMAIN_DIFF_FILE	"domain_differ.txt"


static int g_domain_view_node_num;
static int g_domain_view_node_size;
static char * g_domain_view_node_output;

h_hash_st *g_domain_table;

struct list_head g_domain_delete_list;

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


typedef struct domain_ipaddr_msg
{
	char ipaddr[MAX_IP_LEN];
	char msg[MAX_PACKET_LEN];
	unsigned int msg_len;
}st_domain_ipaddr_msg;

typedef struct domain_view_node
{ 
	struct list_head list;
	char domain[MAX_NAME_LEN];
	ushort view_id;					//domain 与 view_id用作输出
	st_domain_ipaddr_msg msg_array[MAX_IP_SIZE];
	unsigned int msg_size;
	unsigned short  standup;
	time_t del_time;
}st_domain_view_node;


typedef struct domain_veiw_array
{
    st_domain_view_node *view[MAX_VIEM_NUM];
        
}st_domain_view_array;

typedef struct domain_syn
{
	ushort type;
	char domain[MAX_DOMAIN_LEN];
	ushort view_id;
	char ipaddr[MAX_IP_LEN];
}st_domain_syn;

static 
int domain_memory_to_file()
{
	int result;
	char path[MAX_PATH_LEN] = {0};    
	char file[MAX_PATH_LEN] = {0};    
	int number = dns_domain_node_count();
	int size = number*MAX_DOMAIN_LEN;
	char output[size];

	if(getcwd(path,MAX_PATH_LEN) == NULL)
	{
		return -1;
	}
	result = sprintf_n(file,MAX_PATH_LEN,"%s/read_domain_view/%s", path,DOMAIN_MEMORY_FILE);
	if(result <= 0)
	{
		return -1;
	}
	memset(output,0,size);
	result = dns_domain_node_display(output,size);
	if(result < 0)
	{
		return -1;
	}
	result = write_file(file,output,strlen(output));
	return result;
}

static 
int domain_exe_php()
{
	char command[MAX_COMMAND_LEN] = {0};
	char path[MAX_PATH_LEN] = {0}; 
	int result;
	
	if(getcwd(path,sizeof(path)) == NULL)
	{
		return -1;
	}
	result = sprintf_n(command,MAX_COMMAND_LEN, "php -c /etc/php.ini %s/read_domain_view/make_domain_differ.php", path);
	if(result < 0)
	{
		return -1;
	}
	system(command);
	return 0;
}

static 
void domain_syn_exec(st_domain_syn * syn)
{
	if(syn->type == 1)
	{
		dns_domain_create(syn->domain,strlen(syn->domain),syn->view_id);
	}
	else if(syn->type == 2)
	{
		dns_domain_insert(syn->domain,strlen(syn->domain),syn->view_id,syn->ipaddr);
	}
	else if(syn->type == 3)
	{
		dns_domain_remove(syn->domain,strlen(syn->domain),syn->view_id,syn->ipaddr);
	}
	else if(syn->type == 4)
	{
		dns_domain_drop(syn->domain,strlen(syn->domain),syn->view_id);
	}
}

static 
int domain_file_to_memory()
{
	FILE *file;
	char temp[MAX_LINE_SIZE];
	char parts[MAX_DIVIDE_SIZE][MAX_LINE_SIZE];
	st_domain_syn domain_syn;
	char path[MAX_PATH_LEN] = {0};  
	char filename[MAX_PATH_LEN];
	int result;

	if(getcwd(path,MAX_PATH_LEN) == NULL)
	{
		return -1;
	}
	result = sprintf_n(filename,MAX_PATH_LEN,"%s/read_domain_view/%s", path,DOMAIN_DIFF_FILE);
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

		domain_syn.type = atoi(parts[0]);
		if(domain_syn.type < 1 && domain_syn.type > 4)
		{
			continue;
		}
		
		memcpy(domain_syn.domain,parts[1],MAX_DOMAIN_LEN);

		domain_syn.view_id = atoi(parts[2]);
		if(domain_syn.view_id < 0 || domain_syn.view_id>MAX_VIEM_NUM)
		{
			continue;
		}

		if(i>=MAX_DIVIDE_SIZE)
		{
			memcpy(domain_syn.ipaddr,parts[3],MAX_IP_LEN);
		}
		else
		{
			memcpy(domain_syn.ipaddr,DEFAULT_DOMAIN_IPADDR,strlen(DEFAULT_DOMAIN_IPADDR));
		}
		
		domain_syn_exec(&domain_syn);
	}
	fclose(file);
	return 0;
}

static 
int domain_syn_from_mysql()
{
	int result;
	
	//输出当前状态
	result = domain_memory_to_file();
	if(result != 0)
	{
		he_debug("domain_memory_to_file fail.");
		return -1;
	}

	//调用php生成差异化文件
	result = domain_exe_php();
	if(result != 0)
	{
		he_debug("domain_exe_php fail.");
		return -1;
	}

	//读差异化文件，修改内存
	domain_file_to_memory();
	
	return 0;
}

static void domain_member_destroy(void *data)
{
    assert(data);
    int i = 0;
    
    st_domain_view_array *domain = (st_domain_view_array *)data;
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
domain_view_create(st_domain_view_array *domain_array,char*domain,int domain_len,ushort view_id)
{
	int errno;
    st_domain_view_node *node = NULL;
    
    node = (st_domain_view_node *)h_malloc(sizeof(st_domain_view_node));
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
st_domain_view_node * get_domain_veiw_node(char * domain,int domain_len,ushort view_id)
{
	st_domain_view_array *temp = NULL;
	st_domain_view_node*node = NULL;

	if(h_hash_search(g_domain_table,domain,domain_len,(void **)&temp) != 0)
	{
		/*hash find failed*/
		//he_debug("[get_domain_veiw_node] domain not exist,get_domain_veiw_node failed!\n");
		return NULL;
	}
	
	node = temp->view[view_id];
	return node;
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

static
int make_dns_answer(char result[MAX_PACKET_LEN],char*domain,int domain_len,char ip[MAX_IP_LEN])
{
	int errno,len;
	char qname[MAX_DOMAIN_LEN];
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
	
	len = domain_to_q_name(domain,domain_len,qname);
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
domain_view_ip_insert(st_domain_view_node *node,char *domain,int domain_len,char ipaddr[MAX_IP_LEN])
{
	int i,packet_len;

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

	memset(&(node->msg_array[node->msg_size]),0,sizeof(st_domain_ipaddr_msg));
	memcpy(node->msg_array[node->msg_size].ipaddr,ipaddr,MAX_IP_LEN);
	packet_len = make_dns_answer(node->msg_array[node->msg_size].msg,domain,domain_len,ipaddr);
	if(packet_len <= 0)
	{
		return -1;
	}
	node->msg_array[node->msg_size].msg_len = packet_len;
	node->msg_size++;
	return 0;
}

static int
domain_view_ip_remove(st_domain_view_node *node,char *domain,int domain_len,char ipaddr[MAX_IP_LEN])
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
				memcpy(&(node->msg_array[i]),&(node->msg_array[node->msg_size-1]),sizeof(st_domain_ipaddr_msg));
			}
			node->msg_size--;
			return 0;
		}
	}

	return -1;
}

static 
int domain_view_display(const void *key, int klen, void *val, void *data)
{
    int i,j,len;
	char node_output[MAX_NODE_OUTPUT_LEN] = {0};
	char ipaddr_output[MAX_IP_LEN+4] = {0};
    st_domain_view_array *domain = (st_domain_view_array*)val;
	
    for(i = 0; i < MAX_VIEM_NUM; i++)
    {
        st_domain_view_node*temp = NULL;
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
			
			strcat_n(g_domain_view_node_output,g_domain_view_node_size,node_output);
        }
    }

    return 0;
FAILED:
	he_debug("[domain_view_display] output error,domain_view_display failed!\n");
	return -1;

}

static 
int domain_view_count(const void *key, int klen, void *val, void *data)
{
    int i = 0;
    st_domain_view_array *domain = (st_domain_view_array*)val;
    for(i = 0; i < MAX_VIEM_NUM; i++)
    {
        st_domain_view_node*temp = NULL;
        temp = domain->view[i];
        if (!temp)
        {
            continue;
        }
        else
        { 
			g_domain_view_node_num++;
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
int dns_domain_init()
{
	g_domain_table = NULL;

    INIT_LIST_HEAD(&g_domain_delete_list);
    g_domain_table = h_hash_create(domain_member_destroy,NULL,0);
    if (g_domain_table)
    {
        he_debug("[dns domain init sucess!]\n");
        //domain_syn_from_mysql();
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
void dns_domain_syn()
{
	domain_syn_from_mysql();
}


/**
* @brief    清理域名表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_domain_destroy()
{
	st_domain_view_node *pos =NULL;
    st_domain_view_node *n =NULL;
    if (g_domain_table)
    {
        h_hash_destroy(g_domain_table);
        g_domain_table = NULL;
    }   
    list_for_each_entry_safe(pos,n,&g_domain_delete_list,list)
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
int dns_domain_create(char *domain,int domain_len,ushort view_id)
{
	if(domain == NULL)
	{
		he_debug("[create domain failed!] illegal input,dns_domain_create failed!\n");
		return -1;
	}

	st_domain_view_array *temp = NULL;
    st_domain_view_array *new_domain = NULL;

	if(h_hash_search(g_domain_table,domain,domain_len,(void **)&temp) != 0)
	{
		/*hash find failed*/
		new_domain = (st_domain_view_array *)h_malloc(sizeof(st_domain_view_array));
        if (!new_domain)
        {
        	he_debug("[create domain failed!] malloc fail,dns_domain_create failed!\n");
			return -1;
        }
		(void)h_hash_insert(g_domain_table,domain,domain_len ,(void *)new_domain);
		if(domain_view_create(new_domain,domain,domain_len,view_id) != 0)
		{
			he_debug("[create domain failed!] domain_view_create fail,dns_domain_create failed!\n");
			return -1;
		}
	}
	else
	{
		if(temp->view[view_id] != NULL)
		{
			he_debug("[create domain failed!] domain exist,dns_domain_create failed!\n");
			return -1;
		}
		if(domain_view_create(temp,domain,domain_len,view_id) != 0)
		{
			he_debug("[create domain failed!] domain_view_create fail,dns_domain_create failed!\n");
			return -1;
		}
	}
	he_debug("create domain[%s]->view[%d]\n",domain,view_id);
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
int dns_domain_drop(char *domain,int domain_len,ushort view_id)
{
	st_domain_view_array *temp = NULL;
	st_domain_view_node*node = NULL;

	if(domain == NULL)
	{
		he_debug("[drop domain failed!] illegal input,dns_domain_create failed!\n");
		return -1;
	}

	if(h_hash_search(g_domain_table,domain,domain_len,(void **)&temp) != 0)
	{
		/*hash find failed*/
		he_debug("[drop domain failed!] domain not exist,get_domain_veiw_node failed!\n");
		return -1;
	}
	
	node = temp->view[view_id];
	if(node == NULL)
	{
		he_debug("[drop domain failed!] view not exist,dns_domain_drop failed!\n");
		return -1;
	}

	node->standup = 1;
    temp->view[view_id] = NULL;
    node->del_time = time(0) + 3;
    list_add_tail(&node->list, &g_domain_delete_list);
    he_debug("drop domain[%s]->view[%d]\n",domain,view_id);
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
int dns_domain_del(char *domain,int domain_len)
{
	int i;
	st_domain_view_array *temp = NULL;
	st_domain_view_node*node = NULL;

	if(domain == NULL)
	{
		he_debug("[drop domain failed!] illegal input,dns_domain_create failed!\n");
		return -1;
	}

	if(h_hash_search(g_domain_table,domain,domain_len,(void **)&temp) != 0)
	{
		/*hash find failed*/
		he_debug("[drop domain failed!] domain not exist,get_domain_veiw_node failed!\n");
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
	    list_add_tail(&node->list, &g_domain_delete_list);
	    
	}

	h_hash_delete(g_domain_table,domain,domain_len);
	he_debug("drop domain[%s]->view[all]\n",domain);
	
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
int dns_domain_insert(char *domain,int domain_len,ushort view_id,char ipaddr[MAX_IP_LEN])
{
	st_domain_view_node*node = NULL;

	if(domain == NULL || ipaddr == NULL)
	{
		he_debug("[insert domain ip failed!] illegal input,dns_domain_insert failed!\n");
		return -1;
	}
	
	node = get_domain_veiw_node(domain,domain_len,view_id);
	if(node == NULL)
	{
		he_debug("[insert domain ip failed!] view not exist,dns_domain_insert failed!\n");
		return -1;
	}

	if(domain_view_ip_insert(node,domain,domain_len,ipaddr) != 0)
	{
		he_debug("[insert domain ip failed!] insert ip error,dns_domain_insert failed!\n");
		return -1;
	}
	he_debug("insert ip[%s] into domain[%s]->view[%d]\n",ipaddr,domain,view_id);
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
int dns_domain_remove(char *domain,int domain_len,ushort view_id,char ipaddr[MAX_IP_LEN])
{
	st_domain_view_node*node = NULL;
	
	if(domain == NULL || ipaddr == NULL)
	{
		he_debug("[dns_domain_remove] illegal input!\n");
		return -1;
	}
	
	node = get_domain_veiw_node(domain,domain_len,view_id);
	if(node == NULL)
	{
		he_debug("[dns_domain_remove] view not exist!\n");
		return -1;
	}

	if(domain_view_ip_remove(node,domain,domain_len,ipaddr) != 0)
	{
		he_debug("[dns_domain_remove] remove ip error!\n");
		return -1;
	}
	else
	{
		if(node->msg_size <=0)
		{
			dns_domain_drop(domain,domain_len,view_id);
		}
	}
	he_debug("remove ip[%s] from domain[%s]->view[%d]\n",ipaddr,domain,view_id);
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
int dns_domain_get(char *packet,char *domain,int domain_len,ushort view_id)
{
	int index;
	st_domain_view_node*node = NULL;

	if(unlikely(domain == NULL))
	{
		he_debug("[dns_domain_get] illegal input,dns_domain_get failed!\n");
		return 0;
	}
	
	node = get_domain_veiw_node(domain,domain_len,view_id);
	if(likely(node == NULL))
	{
		//he_debug("[dns_domain_get] view not exist,dns_domain_get failed!\n");
		return 0;
	}

	if(node->msg_size <= 0)
	{
		return 0;
	}
	
	index = random(node->msg_size);
	//he_debug("[dns_domain_get] domain veiw hold:%s--%d--%s\n",domain,view_id,node->msg_array[index].ipaddr);
	memcpy(packet,node->msg_array[index].msg,MAX_PACKET_LEN);
	return node->msg_array[index].msg_len;
}


/**
* @brief   遍历内存中所有域名信息，计算node数目
* @return node 数目
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/

int dns_domain_node_count()
{
	int errno;
	g_domain_view_node_num = 0;
	(void)h_hash_walk(g_domain_table,NULL,domain_view_count);
	return g_domain_view_node_num;
}


/**
* @brief   遍历内存中所有域名信息，生成内存状态输出
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/

int dns_domain_node_display(char*output,int max_len)
{
	int errno;
	g_domain_view_node_size = max_len;
	g_domain_view_node_output = h_malloc(max_len);
	if(g_domain_view_node_output == NULL)
	{
		return -1;
	}
	(void)h_hash_walk(g_domain_table,NULL,domain_view_display);
	errno = memcpy_safe(output,max_len,g_domain_view_node_output,strlen(g_domain_view_node_output));
	h_free(g_domain_view_node_output);
	if(errno != 0)
	{
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
void dns_domain_clear()
{
	st_domain_view_node*pos =NULL;
    st_domain_view_node *n =NULL;
    list_for_each_entry_safe(pos,n,&g_domain_delete_list,list)
    {
        //hyb_debug("[cache free: [%s]!]\n",pos->domain);
        if ((time(0) - pos->del_time) > 0)
        {
            list_del(&pos->list);
            h_free(pos);
        }
    }
}



