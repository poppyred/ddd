#define WNS_USE_SYSTEM 1

#include "libbase.h"
#include <netinet/in.h>
#include "dns_view.h"
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
#define MAX_NODE_OUTPUT_LEN	(80)
#define MAX_OFF_NUM         (32)
#define MAX_IP_SIZE			(4)

#define MAX_PATH_LEN	(256)
#define MAX_COMMAND_LEN	(512)
#define MAX_LINE_SIZE	(256)
#define MAX_DIVIDE_SIZE	(3)

#define VIEW_MEMORY_FILE	"view_memory.txt"
#define VIEW_DIFF_FILE		"view_differ.txt"


typedef struct view_action
{
	char ipaddr[MAX_IP_LEN];
	sockaddr socket_addr;
}st_view_action;

typedef struct view_struct
{ 
	struct list_head list;
	st_view_action actions[MAX_ACTION_SIZE];
	int action_size;
	struct in_addr error_dns_addr;
	st_view_action non_dns_action;
	time_t del_time;
}st_view_struct;

static st_view_struct *g_veiw_array[MAX_VIEM_NUM];
static struct list_head g_view_delete_list;

static int g_action_all = 0;
static int g_error_all = 0;
static int g_nondns_all = 0;

typedef struct view_syn
{
	ushort type;
	ushort view_id;
	char ipaddr[MAX_IP_LEN];
}st_view_syn;

static 
int view_memory_to_file()
{
	int result;
	char path[MAX_PATH_LEN] = {0};    
	char file[MAX_PATH_LEN] = {0};    
	int number = dns_view_node_count();
	int size = number*MAX_DOMAIN_LEN;
	char output[size];

	if(getcwd(path,MAX_PATH_LEN) == NULL)
	{
		return -1;
	}
	result = sprintf_n(file,MAX_PATH_LEN,"%s/read_domain_view/%s", path,VIEW_MEMORY_FILE);
	if(result <= 0)
	{
		return -1;
	}
	memset(output,0,size);
	result = dns_view_display(output,size);
	if(result < 0)
	{
		return -1;
	}
	result = write_file(file,output,strlen(output));
	return result;
}

static 
int view_exe_php()
{
	char command[MAX_COMMAND_LEN] = {0};
	char path[MAX_PATH_LEN] = {0}; 
	int result;
	
	if(getcwd(path,sizeof(path)) == NULL)
	{
		return -1;
	}
	result = sprintf_n(command,MAX_COMMAND_LEN, "php -c /etc/php.ini %s/read_domain_view/make_view_differ.php", path);
	if(result < 0)
	{
		return -1;
	}
	system(command);
	return 0;
}

static 
void view_syn_exec(st_view_syn * syn)
{
	if(syn->type == 1)
	{
		dns_view_create(syn->view_id);
	}
	else if(syn->type == 2)
	{
		dns_view_insert(syn->view_id,syn->ipaddr);
	}
	else if(syn->type == 3)
	{
		dns_view_remove(syn->view_id,syn->ipaddr);
	}
	else if(syn->type == 4)
	{
		dns_view_drop(syn->view_id);
	}
}

static 
int view_file_to_memory()
{
	FILE *file;
	char temp[MAX_LINE_SIZE];
	char parts[MAX_DIVIDE_SIZE][MAX_LINE_SIZE];
	st_view_syn view_syn;
	char path[MAX_PATH_LEN] = {0};  
	char filename[MAX_PATH_LEN];
	int result;

	if(getcwd(path,MAX_PATH_LEN) == NULL)
	{
		return -1;
	}
	result = sprintf_n(filename,MAX_PATH_LEN,"%s/read_domain_view/%s", path,VIEW_DIFF_FILE);
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

		view_syn.type = atoi(parts[0]);
		if(view_syn.type < 1 && view_syn.type > 4)
		{
			continue;
		}
		

		view_syn.view_id = atoi(parts[1]);
		if(view_syn.view_id < 0 || view_syn.view_id>MAX_VIEM_NUM)
		{
			continue;
		}
		if(i>2)
		{
			memcpy(view_syn.ipaddr,parts[2],MAX_IP_LEN);
		}
		
		
		view_syn_exec(&view_syn);
	}
	fclose(file);
	return 0;
}

static 
int view_syn_from_mysql()
{
	int result;
	
	//输出当前状态
	result = view_memory_to_file();
	if(result != 0)
	{
		he_debug("view_memory_to_file fail.");
		return -1;
	}

	//调用php生成差异化文件
	result = view_exe_php();
	if(result != 0)
	{
		he_debug("view_exe_php fail.");
		return -1;
	}

	//读差异化文件，修改内存
	view_file_to_memory();
	
	return 0;
}



static int
view_ip_insert(st_view_struct *node,char ipaddr[MAX_IP_LEN])
{
	int i;

	if(node->action_size >= MAX_ACTION_SIZE)
	{
		return -1;
	}
	
	for(i=0;i<node->action_size;i++)
	{
		if(memcmp(node->actions[i].ipaddr,ipaddr,MAX_IP_LEN) != 0)
		{
			continue;
		}
		else
		{
			return -1;
		}
	}

	memset(&(node->actions[node->action_size]),0,sizeof(st_view_action));
	memcpy(node->actions[node->action_size].ipaddr,ipaddr,MAX_IP_LEN);
	((sockaddr_in*)&(node->actions[node->action_size].socket_addr))->sin_addr.s_addr = inet_addr(ipaddr);
    ((sockaddr_in*)&(node->actions[node->action_size].socket_addr))->sin_port = htons(53);
    ((sockaddr_in*)&(node->actions[node->action_size].socket_addr))->sin_family = AF_INET;
	node->action_size++;
	return 0;
}

static int
view_ip_remove(st_view_struct *node,char ipaddr[MAX_IP_LEN])
{
	int i;
	
	for(i=0;i<node->action_size;i++)
	{
		if(memcmp(node->actions[i].ipaddr,ipaddr,MAX_IP_LEN) != 0)
		{
			continue;
		}
		else
		{
			if(i<node->action_size-1)
			{
				memcpy(&(node->actions[i]),&(node->actions[node->action_size-1]),sizeof(st_view_action));
			}
			node->action_size--;
			return 0;
		}
	}

	return -1;
}


static int
view_err_dns_ip_set(st_view_struct *node,in_addr_t ipaddr)
{
	memcpy(&(node->error_dns_addr),&ipaddr,sizeof(struct in_addr));
	return 0;
}

static int
view_err_dns_ip_unset(st_view_struct *node)
{
	memset(&(node->error_dns_addr),0,sizeof(struct in_addr));
	return 0;
}


static int
view_non_dns_ip_set(st_view_struct *node,char ipaddr[MAX_IP_LEN])
{
	memset(&(node->non_dns_action),0,sizeof(st_view_action));
	memcpy(node->non_dns_action.ipaddr,ipaddr,MAX_IP_LEN);
	((sockaddr_in*)&(node->non_dns_action.socket_addr))->sin_addr.s_addr = inet_addr(ipaddr);
	if(((sockaddr_in*)&(node->non_dns_action.socket_addr))->sin_addr.s_addr == -1)
	{
		return -1;
	}
    ((sockaddr_in*)&(node->non_dns_action.socket_addr))->sin_port = htons(53);
    ((sockaddr_in*)&(node->non_dns_action.socket_addr))->sin_family = AF_INET;
	return 0;
}

static int
view_non_dns_ip_unset(st_view_struct *node)
{
	memset(&(node->non_dns_action),0,sizeof(st_view_action));
	return 0;
}


/**
* @brief    初始化视图
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
int dns_view_init()
{
	memset(g_veiw_array,0,sizeof(g_veiw_array));
    INIT_LIST_HEAD(&g_view_delete_list);
    //view_syn_from_mysql();
    return 0;
}

/**
* @brief    同步mysql到内存
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_view_syn()
{
	view_syn_from_mysql();
}


/**
* @brief    清理视图
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
void dns_view_destroy()
{
	int i;
	st_view_struct *pos =NULL;
    st_view_struct *n =NULL;

	for(i=0;i<MAX_VIEM_NUM;i++)
	{
		if(g_veiw_array[i] != NULL)
		{
			h_free(g_veiw_array[i]);
			g_veiw_array[i] = NULL;
		}
	}

	list_for_each_entry_safe(pos,n,&g_view_delete_list,list)
    {
    	list_del(&pos->list);
        h_free(pos);
    }
	return;
}

/**
* @brief   新建视图
* @param   view_id          视图id
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
int dns_view_create(ushort view_id)
{
	st_view_struct * temp;
	st_view_struct * newone;

	temp = g_veiw_array[view_id];
	if(temp != NULL)
	{
		//he_debug("[dns_view_create] create view fail:view exist!\n");
		return -1;
	}

	newone = h_malloc(sizeof(st_view_struct));
	g_veiw_array[view_id] = newone;
	he_debug("create view[%d]\n",view_id);
	return 0;
}

/**
* @brief   删除域名劫持
* @param   view_id          视图id
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
int dns_view_drop(ushort view_id)
{
	st_view_struct * temp;

	temp = g_veiw_array[view_id];
	if(temp == NULL)
	{
		he_debug("[dns_view_drop] drop view fail:view not exist!\n");
		return -1;
	}

	temp->del_time = time(0) + 3;
    list_add_tail(&temp->list, &g_view_delete_list);
    g_veiw_array[view_id] = NULL;
    he_debug("drop view[%d]\n",view_id);
	return 0;
}

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
int dns_view_insert(ushort view_id,char ipaddr[MAX_IP_LEN])
{
	st_view_struct * temp;
	
	if(ipaddr == NULL)
	{
		he_debug("[dns_view_insert] insert ip to view fail:wront ipaddr!\n");
		return -1;
	}
	
	temp = g_veiw_array[view_id];
	if(temp == NULL)
	{
		he_debug("[dns_view_insert] insert ip to view fail:view not exist!\n");
		return -1;
	}

	if(view_ip_insert(temp,ipaddr) != 0)
	{
		he_debug("[dns_view_insert] insert ip to view fail:insert fail!\n");
		return -1;
	}
	he_debug("insert ip[%s] into view[%d]\n",ipaddr,view_id);
	return 0;
}

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
int dns_view_remove(ushort view_id,char ipaddr[MAX_IP_LEN])
{
	st_view_struct * temp;
	
	if(ipaddr == NULL)
	{
		he_debug("[dns_view_remove] remove ip from view fail:wront ipaddr!\n");
		return -1;
	}
	
	temp = g_veiw_array[view_id];
	if(temp == NULL)
	{
		he_debug("[dns_view_remove] remove ip from view fail:view not exist!\n");
		return -1;
	}

	if(view_ip_remove(temp,ipaddr) != 0)
	{
		he_debug("[dns_view_remove] remove ip from view fail:remove fail!\n");
		return -1;
	}
	
	if(view_id == 0 && temp->action_size <= 0)
	{
		dns_view_server_all_off();
	}
	
	he_debug("remove ip[%s] from view[%d]\n",ipaddr,view_id);
	return 0;
}


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
int dns_view_get_sockaddr(ushort view_id,sockaddr * ipaddrs[MAX_ACTION_SIZE])
{
	int i;
	st_view_struct * temp;
	
	if(g_action_all == 1)
	{
		view_id = 0;
	}
	
	temp = g_veiw_array[view_id];
	if(temp == NULL)
	{
		he_debug("[dns_view_remove] remove ip from view fail:view not exist!\n");
		return -1;
	}

	for(i=0;i<temp->action_size;i++)
	{
		ipaddrs[i] = &(temp->actions[i].socket_addr);
	}
	return temp->action_size;
}


/**
* @brief   遍历内存中所有视图信息，计算有效视图数目
* @return node 数目
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
int dns_view_node_count()
{
	int i,count=0;

	for(i=0;i<MAX_VIEM_NUM;i++)
	{
		if(g_veiw_array[i] != NULL)
		{
			count++;
		}
	}
	return count;
}


/**
* @brief   遍历内存中所有视图信息，生成内存状态输出
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/25
**/
int dns_view_display(char*output,int max_len)
{
	memset(output,0,max_len);
	int i,j,len;
	char node_output[MAX_NODE_OUTPUT_LEN] = {0};
	char ipaddr_output[MAX_IP_LEN+4] = {0};

	for(i=0;i<MAX_VIEM_NUM;i++)
	{
		if(g_veiw_array[i] != NULL)
		{
			len = sprintf_n(node_output,MAX_NODE_OUTPUT_LEN,"\n%d|",i);
			if(len<=0)
			{
				goto FAILED;
			}
			for(j=0;j<g_veiw_array[i]->action_size;j++)
			{
				len = sprintf_n(ipaddr_output,MAX_NODE_OUTPUT_LEN,"%s ",g_veiw_array[i]->actions[j].ipaddr);
				if(len<=0)
				{
					goto FAILED;
				}
				strcat_n(node_output,MAX_NODE_OUTPUT_LEN,ipaddr_output);
			}

			len = strcat_n(output,max_len,node_output);
			if(len<0)
			{
				goto FAILED;
			}
		}
		
	}
	return 0;
	
FAILED:
	he_debug("[dns_view_display] output error!\n");
	return -1;

}



/**
* @brief    清空回收链表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_view_clear()
{
	st_view_struct*pos =NULL;
    st_view_struct *n =NULL;
    
    list_for_each_entry_safe(pos,n,&g_view_delete_list,list)
    {
        //hyb_debug("[cache free: [%s]!]\n",pos->domain);
        if ((time(0) - pos->del_time) > 0)
        {
            list_del(&pos->list);
            h_free(pos);
        }
    }
}

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
int dns_view_err_set(ushort view_id,char ipaddr[MAX_IP_LEN])
{
	st_view_struct * temp;
	
	if(ipaddr == NULL)
	{
		he_debug("[dns_view_err_set] insert ip to error_dns_action fail:wront ipaddr!\n");
		return -1;
	}
	
	in_addr_t addr = inet_addr(ipaddr);
	if(addr == -1)
	{
		he_debug("[dns_view_err_set] insert ip to error_dns_action fail:wront ipaddr!\n");
		return -1;
	}
	
	temp = g_veiw_array[view_id];
	if(temp == NULL)
	{
		he_debug("[dns_view_err_set] view not exist!\n");
		return -1;
	}

	if(view_err_dns_ip_set(temp,addr) != 0)
	{
		he_debug("[dns_view_err_set] insert ip to error_dns_action fail:wront ipaddr!\n");
		return -1;
	}
	he_debug("insert ip into error_dns_action[%d]\n",view_id);
	return 0;
}

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
int dns_view_err_unset(ushort view_id)
{
	st_view_struct * temp;
	
	temp = g_veiw_array[view_id];
	if(temp == NULL)
	{
		he_debug("[dns_view_err_unset] remove ip from view fail:view not exist!\n");
		return -1;
	}

	view_err_dns_ip_unset(temp);
	he_debug("unset ip from error_dns_action[%d]\n",view_id);
	return 0;
}


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
int dns_view_err_get_sockaddr(ushort view_id,struct in_addr * ipaddr)
{
	st_view_struct * temp;
	
	if(g_error_all == 1)
	{
		view_id = 0;
	}
	
	temp = g_veiw_array[view_id];
	if(temp == NULL)
	{
		he_debug("[dns_view_err_get_sockaddr] view not exist!\n");
		return -1;
	}

	memcpy(ipaddr,&(temp->error_dns_addr),sizeof(struct in_addr));
	return 0;
}


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
int dns_view_non_set(ushort view_id,char ipaddr[MAX_IP_LEN])
{
	st_view_struct * temp;
	
	if(ipaddr == NULL)
	{
		he_debug("[dns_view_non_set] insert ip to error_dns_action fail:wront ipaddr!\n");
		return -1;
	}
	
	temp = g_veiw_array[view_id];
	if(temp == NULL)
	{
		he_debug("[dns_view_non_set] view not exist!\n");
		return -1;
	}

	if(view_non_dns_ip_set(temp,ipaddr) != 0)
	{
		he_debug("[dns_view_non_set] insert ip to error_dns_action fail:wront ipaddr!\n");
		return -1;
	}
	he_debug("insert ip[%s] into non_dns_action[%d]\n",ipaddr,view_id);
	return 0;
}

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
int dns_view_non_unset(ushort view_id)
{
	st_view_struct * temp;
	
	temp = g_veiw_array[view_id];
	if(temp == NULL)
	{
		he_debug("[dns_view_non_unset] remove ip from view fail:view not exist!\n");
		return -1;
	}

	view_non_dns_ip_unset(temp);
	he_debug("unset ip from error_dns_action[%d]\n",view_id);
	return 0;
}


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
int dns_view_non_get_sockaddr(ushort view_id,sockaddr * ipaddr)
{
	st_view_struct * temp;
	
	if(g_nondns_all == 1)
	{
		view_id = 0;
	}
	
	temp = g_veiw_array[view_id];
	if(temp == NULL)
	{
		he_debug("[dns_view_non_get_sockaddr] view not exist!\n");
		return -1;
	}

	ipaddr = &(temp->non_dns_action.socket_addr);
	return 0;
}

/**
* @brief   开启DNS服务器全局设置
*
* @return 成功：0	失败：-1
* @remark null
* @see     
* @author hezuoxiang      @date 2013/11/26
**/
int dns_view_server_all_on()
{
	st_view_struct * temp;
	
	temp = g_veiw_array[0];
	if(temp == NULL)
	{
		he_debug("[dns_view_remove] remove ip from view fail:view not exist!\n");
		return -1;
	}
	
	if(temp->action_size <= 0)
	{
		return -1;
	}
	
	g_action_all = 1;
	return 0;
}

/**
* @brief   关闭DNS服务器全局设置
*
* @return 无
* @remark null
* @see     
* @author hezuoxiang      @date 2013/11/26
**/
void dns_view_server_all_off()
{
	g_action_all = 0;
}


/**
* @brief   开启错误DNS流量全局设置
*
* @return 无
* @remark null
* @see     
* @author hezuoxiang      @date 2013/11/26
**/
void dns_view_err_all_on()
{
	g_error_all = 1;
}

/**
* @brief   关闭错误DNS流量全局设置
*
* @return 无
* @remark null
* @see     
* @author hezuoxiang      @date 2013/11/26
**/
void dns_view_err_all_off()
{
	g_error_all = 0;
}


/**
* @brief   开启非DNS流量全局设置
*
* @return 无
* @remark null
* @see     
* @author hezuoxiang      @date 2013/11/26
**/
void dns_view_non_all_on()
{
	g_nondns_all = 1;
}

/**
* @brief   关闭非DNS流量全局设置
*
* @return 无
* @remark null
* @see     
* @author hezuoxiang      @date 2013/11/26
**/
void dns_view_non_all_off()
{
	g_nondns_all = 0;
}