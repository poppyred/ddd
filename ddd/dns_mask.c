#define WNS_USE_SYSTEM 1

#include "libbase.h"
#include <netinet/in.h>
#include "dns_mask.h"
#include "dns_pack.h"
#include "dns_unpack.h"
#include <pthread.h>
#include <assert.h>
#include "dns_tool.h"
#include <stdio.h>



#define he_debug(fmt, args ...) \
    do {fprintf(stderr,"[%s][%d][he]:"fmt, __FILE__, __LINE__,##args);}while(0)

#define random(x) (rand()%x)

#define MAX_PATH_LEN	(256)
#define MAX_COMMAND_LEN	(512)
#define MAX_LINE_SIZE	(256)
#define MAX_DIVIDE_SIZE	(4)


#define MASK_MEMORY_FILE	"mask_memory.txt"
#define MASK_DIFF_FILE		"mask_differ.txt"


extern time_t global_now;
//int global_now;

#define MAX_NODE_OUTPUT_LEN	(64)
#define MAX_IP_SIZE			(3)

typedef unsigned char	uchar;

typedef struct mask_control_struct
{
	struct list_head list;
	ulong begin;
	ulong number;
	ushort view_id;
}st_mask_control_struct;

typedef struct mask_veiw_struct
{ 
	struct list_head list;
	char ipaddr[MAX_IP_LEN];
	ushort mask;
	st_mask_control_struct control;
}st_mask_veiw_struct;

typedef struct mask_control_array
{
	struct list_head list;
	st_mask_control_struct * mask_array;
	int size;
	time_t del_time;
}st_mask_control_array;


static int g_mask_control_node_num;

static st_mask_control_array * g_mask_control_array[2];
static int g_mask_control_array_select;


static struct list_head g_mask_view_list;
static struct list_head g_mask_delete_list;
static struct list_head g_mask_temp_control_list;
static st_mask_control_struct * g_mask_temp_pre;	//拆分过程中，记录前置节点在临时链的位置

static ulong in_addr_to_long(struct in_addr * addr)
{
	unsigned char*addr_p = (unsigned char*)addr;
	return addr_p[0]*16777216+addr_p[1]*65536+addr_p[2]*256+addr_p[3];
}



typedef struct mask_syn
{
	ushort type;
	char ipaddr[MAX_IP_LEN];
	ushort mask;
	ushort view_id;
}st_mask_syn;

static 
int mask_memory_to_file()
{
	int result;
	char path[MAX_PATH_LEN] = {0};    
	char file[MAX_PATH_LEN] = {0};    
	int number = dns_mask_view_node_count();
	int size = number*MAX_DOMAIN_LEN;
	char output[size];

	if(getcwd(path,MAX_PATH_LEN) == NULL)
	{
		return -1;
	}
	result = sprintf_n(file,MAX_PATH_LEN,"%s/read_domain_view/%s", path,MASK_MEMORY_FILE);
	if(result <= 0)
	{
		return -1;
	}
	memset(output,0,size);
	result = dns_mask_view_node_display(output,size);
	if(result < 0)
	{
		return -1;
	}
	result = write_file(file,output,strlen(output));
	return result;
}

static 
int mask_exe_php()
{
	char command[MAX_COMMAND_LEN] = {0};
	char path[MAX_PATH_LEN] = {0}; 
	int result;
	
	if(getcwd(path,sizeof(path)) == NULL)
	{
		return -1;
	}
	result = sprintf_n(command,MAX_COMMAND_LEN, "php -c /etc/php.ini %s/read_domain_view/make_mask_differ.php", path);
	if(result < 0)
	{
		return -1;
	}
	system(command);
	return 0;
}

static 
void mask_syn_exec(st_mask_syn * syn)
{
	if(syn->type == 1)
	{
		dns_mask_insert(syn->ipaddr,syn->mask,syn->view_id);
	}
	else if(syn->type == 4)
	{
		dns_mask_remove(syn->ipaddr,syn->mask);
	}
}

static 
int mask_file_to_memory()
{
	char * mask_temp;
	FILE *file;
	char temp[MAX_LINE_SIZE];
	char parts[MAX_DIVIDE_SIZE][MAX_LINE_SIZE];
	st_mask_syn mask_syn;
	char path[MAX_PATH_LEN] = {0};  
	char filename[MAX_PATH_LEN];
	int result;

	if(getcwd(path,MAX_PATH_LEN) == NULL)
	{
		return -1;
	}
	result = sprintf_n(filename,MAX_PATH_LEN,"%s/read_domain_view/%s", path,MASK_DIFF_FILE);
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

		mask_syn.type = atoi(parts[0]);
		if(mask_syn.type < 1 && mask_syn.type > 4)
		{
			continue;
		}

		mask_temp = strchr(parts[1],'/');
		if(mask_temp == NULL)
		{
			continue;
		}
		mask_syn.mask = atoi(mask_temp+1);
		memset(mask_temp,0,strlen(mask_temp));
		memcpy(mask_syn.ipaddr,parts[1],MAX_IP_LEN);
		

		if(mask_syn.type != 4)
		{
			mask_syn.view_id = atoi(parts[2]);
			if(mask_syn.view_id < 0 || mask_syn.view_id>MAX_VIEM_NUM)
			{
				continue;
			}
		}
		
		mask_syn_exec(&mask_syn);
	}
	fclose(file);
	dns_mask_fresh_view_to_control();
	return 0;
}

static 
int mask_syn_from_mysql()
{
	int result;
	
	//输出当前状态
	result = mask_memory_to_file();
	if(result != 0)
	{
		he_debug("mask_memory_to_file fail.");
		return -1;
	}

	//调用php生成差异化文件
	result = mask_exe_php();
	if(result != 0)
	{
		he_debug("mask_exe_php fail.");
		return -1;
	}

	//读差异化文件，修改内存
	mask_file_to_memory();
	
	return 0;
}


static 
ulong addr_to_long(char ipaddr[MAX_IP_LEN])
{
	struct in_addr addr;
	int ret;
	
	ret = inet_aton(ipaddr,&addr);
	if(ret == 0)
	{
		return 0;
	}
	
	return in_addr_to_long(&addr);
}

static
int long_to_addr(char*addr,ulong val)
{
	uchar addr_char[4];
	
	addr_char[0] = (uchar)(val/16777216);
	val = val - addr_char[0]*16777216;
	
	addr_char[1] = (uchar)(val/65536);
	val = val - addr_char[1]*65536;
	
	addr_char[2] = (uchar)(val/256);
	val = val - addr_char[2]*256;
	
	addr_char[3] = (uchar)val;
	
	inet_ntop(AF_INET, addr_char, addr, 16);
	return 0;
}


static
ulong mask_to_number(ushort mask)
{
	return 1<<((ulong)(32-mask));
}

static
int mask_view_struct_fill(st_mask_veiw_struct * mask_view,char ipaddr[MAX_IP_LEN],ushort mask,ushort view_id)
{
	memcpy(mask_view->ipaddr,ipaddr,MAX_IP_LEN);
	mask_view->mask = mask;
	mask_view->control.begin = addr_to_long(ipaddr);
	if(mask_view->control.begin == 0)
	{
		return -1;
	}
	mask_view->control.number = mask_to_number(mask);
	mask_view->control.view_id = view_id;
	return 0;
}

//1比2地址较小，个数较多时，返回正数
static 
int mask_view_compare(st_mask_veiw_struct * temp1,st_mask_veiw_struct * temp2)
{
	if(temp1->control.begin<temp2->control.begin)
	{
		return 1;
	}
	else if(temp1->control.begin>temp2->control.begin)
	{
		return -1;
	}
	else
	{
		if(temp1->control.number>temp2->control.number)
		{
			return 1;
		}
		else if(temp1->control.number<temp2->control.number)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
}


static 
int mask_view_insert(char ipaddr[MAX_IP_LEN],ushort mask,ushort view_id)
{
	int compare;
	st_mask_veiw_struct*pos =NULL;
    st_mask_veiw_struct *n =NULL;
	st_mask_veiw_struct * newone;

	newone = h_malloc(sizeof(st_mask_veiw_struct));
	mask_view_struct_fill(newone,ipaddr,mask,view_id);
    
    list_for_each_entry_safe(pos,n,&g_mask_view_list,list)
    {
        compare = mask_view_compare(newone,pos);
        if(compare==0)
        {
        	//node exist
			goto FAILURE;
        }
        else if(compare>0)	//当新值比当前位置地址小或个数多时
        {
			__list_add(&newone->list,pos->list.prev,&pos->list);
			goto SUCCEED;
        }
    }
    list_add_tail(&newone->list,&g_mask_view_list);
    
SUCCEED:
	return 0;
    
FAILURE:
	h_free(newone);
	return -1;
}

static 
int mask_view_remove(char ipaddr[MAX_IP_LEN],ushort mask)
{
	int compare;
	st_mask_veiw_struct*pos =NULL;
    st_mask_veiw_struct *n =NULL;
	st_mask_veiw_struct newone;

	mask_view_struct_fill(&newone,ipaddr,mask,0);
    
    list_for_each_entry_safe(pos,n,&g_mask_view_list,list)
    {
        compare = mask_view_compare(pos,&newone);
        if(compare==0)
        {
        	__list_del(pos->list.prev,pos->list.next);
        	h_free(pos);
        	goto SUCCEED;
        }
    }

	return -1;
SUCCEED:
	return 0;
}

static
int get_negative(int select)
{
	return (select+1)%2;
}

static
st_mask_control_array * mask_get_write_control_array()
{
	return g_mask_control_array[get_negative(g_mask_control_array_select)];
}

static
st_mask_control_array * mask_get_read_control_array()
{
	return g_mask_control_array[g_mask_control_array_select];
}

static
void mask_control_rw_change()
{
	g_mask_control_array_select = get_negative(g_mask_control_array_select);
}

static
int mask_view_contain(st_mask_control_struct *low,st_mask_control_struct *hight)
{
	ulong low_end = low->begin+low->number-1;
	ulong hight_end = hight->begin+hight->number-1;
	
	if(low->begin < hight->begin && low_end > hight_end)
	{
		return 3;
	}
	else if(low->begin == hight->begin && low_end > hight_end)
	{
		return 2;
	}
	else if(low->begin < hight->begin && low_end == hight_end)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static
int mask_divide_add_tail(st_mask_control_struct * node)
{
	st_mask_control_struct*newone = NULL;
	int size;

	size = sizeof(st_mask_control_struct);
	newone = h_malloc(size);
	if(newone == NULL)
	{
		return -1;
	}
	
	memcpy(newone,node,size);
	list_add_tail(&newone->list,&g_mask_temp_control_list);
	g_mask_control_node_num++;
	g_mask_temp_pre = newone;
	return 0;
}


//拆分节点，对临时链直接操作
//larger_in_link是临时链中的结构

static 
int mask_divide_node(st_mask_control_struct * larger_in_link,st_mask_control_struct * less,int status)
{
	if(status == 3)
	{
		//begin和end都不相等，number缩小，再新建两个node
		st_mask_control_struct *mid_node;
		st_mask_control_struct *tail_node;
		int size = sizeof(st_mask_control_struct);
		
		mid_node = h_malloc(size);
		tail_node = h_malloc(size);

		memcpy(mid_node,less,size);

		tail_node->begin = less->begin+less->number;
		tail_node->number = larger_in_link->begin+larger_in_link->number-tail_node->begin;
		tail_node->view_id = larger_in_link->view_id;

		larger_in_link->number = less->begin-larger_in_link->begin;
		
		__list_add(&mid_node->list,&larger_in_link->list,larger_in_link->list.next);
		__list_add(&tail_node->list,&mid_node->list,mid_node->list.next);

		g_mask_control_node_num = g_mask_control_node_num+2;
		g_mask_temp_pre = mid_node;
	}
	else if(status == 2)
	{
		//begin相等，number缩小，新增一个node
		st_mask_control_struct *tail_node;
		int size = sizeof(st_mask_control_struct);
		
		tail_node = h_malloc(size);

		tail_node->begin = larger_in_link->begin+larger_in_link->number+1;
		tail_node->number = larger_in_link->number-less->number;
		tail_node->view_id = larger_in_link->view_id;

		larger_in_link->number = less->number;
		larger_in_link->view_id = less->view_id;
		
		__list_add(&tail_node->list,&larger_in_link->list,larger_in_link->list.next);

		g_mask_control_node_num++;
		g_mask_temp_pre = larger_in_link;
	}
	else if(status == 1)
	{
		//end相等，number缩小，新增一个node
		st_mask_control_struct *tail_node;
		int size = sizeof(st_mask_control_struct);
		
		tail_node = h_malloc(size);

		memcpy(tail_node,less,size);

		larger_in_link->number = larger_in_link->number-less->number;
		
		__list_add(&tail_node->list,&larger_in_link->list,larger_in_link->list.next);

		g_mask_control_node_num++;
		g_mask_temp_pre = tail_node;
	}
	else
	{
		return -1;
	}
	return 0;
}

static
void mask_temp_list_free()
{
	st_mask_control_struct*pos_control = NULL;
	st_mask_control_struct*n_control = NULL;
	
	list_for_each_entry_safe(pos_control,n_control,&g_mask_temp_control_list,list)
    {
    	list_del(&(pos_control->list));
        h_free(pos_control);
    }
}

static
void mask_view_list_free()
{
	st_mask_veiw_struct*pos = NULL;
	st_mask_veiw_struct*n = NULL;
	
	list_for_each_entry_safe(pos,n,&g_mask_view_list,list)
    {
    	list_del(&(pos->list));
        h_free(pos);
    }
}



//检查ip是否属于这个mask
int check_ipaddr_in_mask(st_mask_control_struct*mask,ulong check)
{
	ulong diff;
	if(check > mask->begin)
	{
		diff = check-mask->begin;
		if(diff < mask->number)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else if(check < mask->begin)
	{
		return -1;
	}
	else
	{
		return 0;
	}
	
}

//return the view id,if faiil,return 0(default)
static
int get_view_by_ipaddr_long(st_mask_control_array * read_array,ulong check)
{
	int result;
	int low=0,high=read_array->size-1,mid;
	int index;

	if(read_array == NULL || read_array->mask_array == NULL)
	{
		goto FAILURE;
	}
	
	if(check_ipaddr_in_mask(&read_array->mask_array[low],check) == 0)
	{
		index = low;
		goto SUCCEED;
	}
	
	if(check_ipaddr_in_mask(&read_array->mask_array[high],check) == 0)
	{
		index = high;
		goto SUCCEED;
	}
	
	while(low <= high)
	{
		mid=low+((high-low)/2);
		
		result = check_ipaddr_in_mask(&read_array->mask_array[mid],check);
		if(result == 0)
		{
			index = mid;
			goto SUCCEED;
		}
		else if(result == -1)
		{
			high = mid-1;
		}
		else if(result == 1)
		{
			low = mid+1;
		}
		
		if(low>high)
		{
			goto FAILURE;
		}
	}
SUCCEED:
	return read_array->mask_array[index].view_id;
FAILURE:
	return 1;
}


static
void control_add_node(st_mask_veiw_struct*pos)
{
	st_mask_control_struct*temp_tail = NULL;
    st_mask_control_struct*temp_pre = NULL;
    int contain_status;

	temp_tail = list_entry(g_mask_temp_control_list.prev,st_mask_control_struct,list);	//临时链的尾部
	temp_pre = g_mask_temp_pre;	//mask_view_list链的上一个节点在临时链的位置

    if(g_mask_control_node_num > 0)
    {
		contain_status = mask_view_contain(temp_pre,&(pos->control));
		if(contain_status == 0)		//当前节点不被前一个节点包含
		{
			if(temp_tail != temp_pre)
			{
				contain_status = mask_view_contain(temp_tail,&(pos->control));
			}
			if(contain_status == 0)		//当前节点不被新链尾部节点包含
			{
				mask_divide_add_tail(&(pos->control));
			}
			else
			{
				//拆分temp_tail
				if(temp_tail->view_id != pos->control.view_id)
				{
					mask_divide_node(temp_tail,&(pos->control),contain_status);
				}
			}
		}
		else
		{
			//拆分temp_pre
			if(temp_pre->view_id != pos->control.view_id)
			{
				mask_divide_node(temp_pre,&(pos->control),contain_status);
			}
		}
    }
    else if(g_mask_control_node_num == 0)
    {
		mask_divide_add_tail(&(pos->control));
    }
}



/**
* @brief    初始化mask表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
int dns_mask_init()
{
	int size = sizeof(st_mask_control_array);

    INIT_LIST_HEAD(&g_mask_view_list);
	INIT_LIST_HEAD(&g_mask_delete_list);
	INIT_LIST_HEAD(&g_mask_temp_control_list);

    
    g_mask_control_array[0] = h_malloc(size);
    g_mask_control_array[1] = h_malloc(size);
    g_mask_control_array_select = 0;
    //mask_syn_from_mysql();
    return 0;
}

/**
* @brief    同步mysql到内存
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/22
**/
void dns_mask_syn()
{
	mask_syn_from_mysql();
}


/**
* @brief    清理mask表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
void dns_mask_destroy()
{
	//no implement
}

/**
* @brief   新建mask
* @param   ipaddr           掩码起始地址
* @param   mask       		掩码位数
* @param   view_id          视图id
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
int dns_mask_insert(char ipaddr[MAX_IP_LEN],int mask,ushort view_id)
{
	int result;
	
	result = mask_view_insert(ipaddr,mask,view_id);
	if(result == 0)
	{
		he_debug("insert mask[%s/%d] to view[%d]\n",ipaddr,mask,view_id);
	}
	return result;
}

/**
* @brief   删除mask
* @param   ipaddr           掩码起始地址
* @param   mask       		掩码位数
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
int dns_mask_remove(char ipaddr[MAX_IP_LEN],int mask)
{
	int result;
	
	result = mask_view_remove(ipaddr,mask);
	if(result == 0)
	{
		he_debug("remove mask[%s/%d]\n",ipaddr,mask);
	}
	return result;
}


/**
* @brief   设置已有mask的view
* @param   ipaddr           掩码起始地址
* @param   mask       		掩码位数
* @param   view_id          视图id
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
int dns_mask_set(char ipaddr[MAX_IP_LEN],int mask,ushort view_id)
{
	int result;
	
	result = mask_view_insert(ipaddr,mask,view_id);
	if(result == 0)
	{
		result = mask_view_remove(ipaddr,mask);
		return result;
	}
	return -1;
}



/**
* @brief   刷新更新到读结构
* @param   无
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
int dns_mask_fresh_view_to_control()
{
	st_mask_veiw_struct*pos =NULL;
    st_mask_veiw_struct*n =NULL;
    st_mask_control_struct*pos_control = NULL;
	st_mask_control_struct*n_control = NULL;
    int temp_control_list_count = 0;
    int control_struct_size = sizeof(st_mask_control_struct);

	st_mask_control_array * control_write_array = mask_get_write_control_array();
	st_mask_control_array * control_read_array = mask_get_read_control_array();

	if(control_write_array->mask_array != NULL)
	{
		//每次更新配置前，清空写数组内存
		while(time(0)<control_write_array->del_time);
		h_free(control_write_array->mask_array);
		control_write_array->size = 0;
	}
	
	//INIT_LIST_HEAD(&g_mask_temp_control_list);	2013/11/18 21:49:45

	mask_temp_list_free();
    g_mask_control_node_num = 0;
    
    list_for_each_entry_safe(pos,n,&g_mask_view_list,list)
    {
		control_add_node(pos);
    }

    control_write_array->mask_array = h_malloc(g_mask_control_node_num*sizeof(st_mask_control_struct));
    control_write_array->size = g_mask_control_node_num;
    list_for_each_entry_safe(pos_control,n_control,&g_mask_temp_control_list,list)
    {
    	memcpy(&(control_write_array->mask_array[temp_control_list_count]),pos_control,control_struct_size);
		temp_control_list_count++;
		if(temp_control_list_count > control_write_array->size)
		{
			goto FAILTURE;
		}
    }
	//mask_temp_list_free();
	if(control_read_array != NULL)
	{
		//先设置read数组的删除时间
		control_read_array->del_time = time(0) + 2;;
	}
	mask_control_rw_change();
	return 0;
    
FAILTURE:
	h_free(control_write_array->mask_array);
	control_write_array->size = 0;
	mask_temp_list_free();
	return -1;
}


/**
* @brief   刷新更新到读结构
* @param   无
*
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
int dns_mask_fresh_view_to_control_nowait()
{
	st_mask_veiw_struct*pos =NULL;
    st_mask_veiw_struct*n =NULL;
    st_mask_control_struct*pos_control = NULL;
	st_mask_control_struct*n_control = NULL;
    int temp_control_list_count = 0;
    int control_struct_size = sizeof(st_mask_control_struct);

	st_mask_control_array * control_write_array = mask_get_write_control_array();
	//st_mask_control_array * control_read_array = mask_get_read_control_array();

	if(control_write_array->mask_array != NULL)
	{
		h_free(control_write_array->mask_array);
		control_write_array->size = 0;
	}
	
	//INIT_LIST_HEAD(&g_mask_temp_control_list);	2013/11/19 17:02:03
	mask_temp_list_free();
    g_mask_control_node_num = 0;
    
    list_for_each_entry_safe(pos,n,&g_mask_view_list,list)
    {
		control_add_node(pos);
    }

    control_write_array->mask_array = h_malloc(g_mask_control_node_num*sizeof(st_mask_control_struct));
    control_write_array->size = g_mask_control_node_num;
    list_for_each_entry_safe(pos_control,n_control,&g_mask_temp_control_list,list)
    {
    	memcpy(&(control_write_array->mask_array[temp_control_list_count]),pos_control,control_struct_size);
		temp_control_list_count++;
		if(temp_control_list_count > control_write_array->size)
		{
			goto FAILTURE;
		}
    }
	
	mask_control_rw_change();
	return 0;
    
FAILTURE:
	h_free(control_write_array->mask_array);
	control_write_array->size = 0;
	mask_temp_list_free();
	return -1;
}


/**
* @brief   获取ip对应的view
* @param   ipaddr           域名
*
* @return  成功获取到的view_id，-1为失败
* @remark view id
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
ushort dns_mask_get_view(struct in_addr * addr)
{
	ushort view_id;
	ulong check;
	st_mask_control_array* use_array;
		
	use_array = mask_get_read_control_array();
	check = in_addr_to_long(addr);
	
	view_id = get_view_by_ipaddr_long(use_array,check);
	
	return view_id;
}


/**
* @brief   遍历内存中所有mask_view信息，计算node数目
* @return node 数目
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
int dns_mask_view_node_count()
{
	st_mask_veiw_struct*pos =NULL;
    st_mask_veiw_struct *n =NULL;
    int count = 0;
    
	list_for_each_entry_safe(pos,n,&g_mask_view_list,list)
    {
        count++;
    }
    return count;
}

/**
* @brief   遍历内存中所有mask_control信息，计算node数目
* @return node 数目
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
int dns_mask_control_node_count()
{
	return mask_get_read_control_array()->size;
}


/**
* @brief   遍历内存中所有mask_view信息，生成内存状态输出
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
int dns_mask_view_node_display(char*output,int max_len)
{
	st_mask_veiw_struct*pos =NULL;
    st_mask_veiw_struct *n =NULL;
	char node_output[MAX_NODE_OUTPUT_LEN] = {0};
	int len;
    
	list_for_each_entry_safe(pos,n,&g_mask_view_list,list)
    {
        len = sprintf_n(node_output,MAX_NODE_OUTPUT_LEN,"\n%s/\%d|%d|",pos->ipaddr,pos->mask,pos->control.view_id);
        if(len<=0)
		{
			goto FAILED;
		}
		len = strcat_n(output,max_len,node_output);
		if(len<0)
		{
			goto FAILED;
		}
    }
    return 0;
    
FAILED:
	return -1;
}

/**
* @brief   遍历内存中所有mask_control信息，生成内存状态输出
* @return 0 - 成功  /  -1 - 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
int dns_mask_control_node_display(char*output,int max_len)
{
	st_mask_control_struct*pos =NULL;
    st_mask_control_struct *n =NULL;
	char node_output[MAX_NODE_OUTPUT_LEN] = {0};
	int len;
	char ipaddr[MAX_IP_LEN];
    
	list_for_each_entry_safe(pos,n,&g_mask_temp_control_list,list)
    {
    	long_to_addr(ipaddr,pos->begin);
        len = sprintf_n(node_output,MAX_NODE_OUTPUT_LEN,"\n%s|%d|%d|",ipaddr,(int)pos->number,pos->view_id);
        if(len<=0)
		{
			goto FAILED;
		}
		len = strcat_n(output,max_len,node_output);
		if(len<0)
		{
			goto FAILED;
		}
    }
    return 0;
    
FAILED:
	return -1;
}


/**
* @brief    清空回收链表
* @return 
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/29
**/
void dns_mask_clear()
{
	mask_view_list_free();
}



