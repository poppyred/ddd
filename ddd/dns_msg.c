#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "dns_msg.h"

#include "dns_domain.h"
#include "dns_extend.h"
#include "dns_view.h"
#include "dns_mask.h"
#include "dns_comdef.h"
#include "dns_tool.h"
#include "dns_pkt.h"
#include "dns_cache.h"
#include "dns_extche.h"
#include "dns_pack.h"

#include <assert.h>
#include "libbase.h"
#include "cJSON.h"


int get_type_from_msg(char *msg)
{
    if (!msg)
    {
        return -1;
    }
    
    unsigned char type = 0;
    memcpy(&type,msg,1);

    return (int)type;
    
}

int get_vid_from_msg(char *msg)
{    

    if (!msg)
    {
        return -1;
    }
    
    unsigned short vid = 0;
    memcpy(&vid,msg+1,2);

    return (int)vid;
    
}

int get_opt_from_msg(char *msg)
{
    if (!msg)
    {
        return -1;
    }
    
    unsigned char opt = 0;
    memcpy(&opt,msg+3,1);

    return (int)opt;
    
}


int get_domain_from_msg(char *msg, char *domain, int *len)
{
    if (!msg || !len || !domain)
    {
        return -1;
    }
    unsigned short domain_len = 0;
    
    
    memcpy(&domain_len,msg+4,2);
    *len = domain_len;

    memcpy(domain,msg+6,*len);

    return 0;

}



int get_ip_from_msg(char *msg, struct in_addr *ips, int *num)
{
    if (!msg || !ips || !num)
    {
        return -1;
    }
    int d_len = 0;
    unsigned short ip_num = 0;
    unsigned short domain_len = 0;

    memcpy(&domain_len,msg+4,2);
    d_len = domain_len;
    
    memcpy(&ip_num,msg+6+d_len,2);
    *num = ip_num;

    memcpy(ips, msg+8+ d_len, *num * (sizeof(struct in_addr)));

    return 0;

}

void answer_display(void *buf)
{
    char domain[256] = {0};
    int len = 0;
    int ip_num = 0;   
    struct in_addr ips[10];
    int i = 0;


    
    printf("\n ======================\n");
    printf("*   --- Message In ---   *\n");
    printf(" ======================\n");
    printf("  Type - %d\n",get_type_from_msg(buf));
    printf("  Vid - %d\n",get_vid_from_msg(buf));
    printf("  Opt - %d\n",get_opt_from_msg(buf));
    get_domain_from_msg(buf,domain,&len);
    printf("  Domain - %s(%d)\n",domain,len);
    get_ip_from_msg(buf,ips,&ip_num);

    printf("  Ip_Num - (%d)\n",ip_num);
    for (i = 0; i < ip_num ; i ++)
    {
        printf("  Ip - %s\n",inet_ntoa(ips[i]));
    }
    printf(" ======================\n\n");

    
}


void answer_sucess(char *buf)
{
    memcpy(buf,"01",2);
}


void answer_failure(char *buf)
{
    memcpy(buf,"02",2);
}



void deal_domain_type(void* buf, char* answer)
{
    int i = 0;
    char domain[256] = {0};
    char ip[MAX_IP_LEN] = {0};
    int len = 0;
    int ip_num = 0;
    int vid = 0;
    struct in_addr ips[32];
    
    get_ip_from_msg(buf,ips,&ip_num);
    get_domain_from_msg(buf,domain,&len);
    
    vid = get_vid_from_msg(buf);

    switch(get_opt_from_msg(buf))
    {
    case CREATE_OPT:
        (void)dns_domain_create(domain, len, vid);

        for (i = 0; i < ip_num ; i ++)        
        {
            memset(ip,0,MAX_IP_LEN);
            strcpy(ip,inet_ntoa(ips[i]));
            if (dns_domain_insert(domain, len, vid, ip))
            {
                dns_domain_drop(domain, len, vid);
                goto FAILED;
            }
        }
        
        break;
        
    case INSERT_OPT:

        if (ip_num != 1)
        {
            goto FAILED;
        }
        
        (void)dns_domain_create(domain, len, vid);
        
        memset(ip, 0, MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_domain_insert(domain, len, vid, ip))
        {
            goto FAILED;
        }
        break;
        
        /*
        if (ip_num != 1)
        {
            goto FAILED;
        }
        memset(ip, 0, MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_domain_insert(domain, len, vid, ip))
        {
            goto FAILED;
        }
        break;
        */

    case REMOVE_OPT:
        
        if (ip_num != 1)
        {
            goto FAILED;
        }
        memset(ip, 0, MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_domain_remove(domain, len, vid, ip))
        {
            goto FAILED;
        }
        break;
        
    case MODIFY_OPT:
        
        if (ip_num != 2)
        {
            goto FAILED;
        }

        memset(ip,0,MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_domain_remove(domain, len, vid, ip))
        {
            goto FAILED;
        }

        (void)dns_domain_create(domain, len, vid);
        memset(ip,0,MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[1]));
        if (dns_domain_insert(domain, len, vid, ip))
        {
            goto FAILED;
        }
        
        break;

        /*
        if (dns_domain_drop(domain, len, vid))
        {
            goto FAILED;
        }
        */

    case RESTART_OPT:
        
        if (ip_num != 1)
        {
            goto FAILED;
        }
        
        (void)dns_domain_create(domain, len, vid);


        memset(ip, 0, MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_domain_insert(domain, len, vid, ip))
        {
            goto FAILED;
        }
        break;

    case STOP_OPT:
        
        if (ip_num != 1)
        {
            goto FAILED;
        }
        memset(ip, 0, MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_domain_remove(domain, len, vid, ip))
        {
            goto FAILED;
        }
        break;

    case DROPALL_OPT:
        
        if (dns_domain_del(domain,len))
        {
            goto FAILED;
        }
        break;

    case DROP_OPT:
    case STOPDOMAIN_OPT:
        if (dns_domain_drop(domain,len,vid))
        {
            goto FAILED;
        }
        break;
        
    default:
        hyb_debug("Wrong deal domain message!\n");
        goto FAILED;
      
    }
    
    answer_sucess(answer);
    return;

FAILED:
    answer_failure(answer);
    return;

}

void deal_extend_type(void* buf, char* answer)
{
    int i = 0;
    char domain[256] = {0};
    char ip[MAX_IP_LEN] = {0};
    int len = 0;
    int ip_num = 0;
    int vid = 0;
    struct in_addr ips[32];
    
    get_ip_from_msg(buf,ips,&ip_num);
    get_domain_from_msg(buf,domain,&len);
    
    vid = get_vid_from_msg(buf);

    switch(get_opt_from_msg(buf))
    {
    case CREATE_OPT:
        (void)dns_extend_create(domain, len, vid);

        for (i = 0; i < ip_num ; i ++)        
        {
            memset(ip,0,MAX_IP_LEN);
            strcpy(ip,inet_ntoa(ips[i]));
            if (dns_extend_insert(domain, len, vid, ip))
            {
                dns_extend_drop(domain, len, vid);
                goto FAILED;
            }
        }
        
        break;
        
    case INSERT_OPT:
        
        if (ip_num != 1)
        {
            goto FAILED;
        }
        
        (void)dns_extend_create(domain, len, vid);

        memset(ip,0,MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_extend_insert(domain, len, vid, ip))
        {
            goto FAILED;
        }

        
        break;
        


        /*
        if (ip_num != 1)
        {
            goto FAILED;
        }
        memset(ip, 0, MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_extend_insert(domain, len, vid, ip))
        {
            goto FAILED;
        }
        break;
        */

    case REMOVE_OPT:
        
        if (ip_num != 1)
        {
            goto FAILED;
        }
        memset(ip, 0, MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_extend_remove(domain, len, vid, ip))
        {
            goto FAILED;
        }
        break;
        
    case MODIFY_OPT:
        
        if (ip_num != 2)
        {
            goto FAILED;
        }

        memset(ip,0,MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_extend_remove(domain, len, vid, ip))
        {
            goto FAILED;
        }

        (void)dns_extend_create(domain, len, vid);
        memset(ip,0,MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[1]));
        if (dns_extend_insert(domain, len, vid, ip))
        {
            goto FAILED;
        }
        
        break;
        
        /*
        if (dns_extend_drop(domain, len, vid))
        {
            goto FAILED;
        }
        break;*/
        
    case RESTART_OPT:
        
        if (ip_num != 1)
        {
            goto FAILED;
        }
        
        (void)dns_extend_create(domain, len, vid);

        memset(ip,0,MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_extend_insert(domain, len, vid, ip))
        {
            goto FAILED;
        }

        
        break;

        
    case STOP_OPT:
        
        if (ip_num != 1)
        {
            goto FAILED;
        }
        memset(ip, 0, MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_extend_remove(domain, len, vid, ip))
        {
            goto FAILED;
        }
        
        break;

    case DROPALL_OPT:
        
        if (dns_extend_del(domain,len))
        {
            goto FAILED;
        }
        break;

    case DROP_OPT:
    case STOPDOMAIN_OPT:
        if (dns_extend_drop(domain,len,vid))
        {
            goto FAILED;
        }
        break;

        
    default:
        hyb_debug("Wrong deal extend message!\n");
        goto FAILED;
      
    }
    
    answer_sucess(answer);
    return;

FAILED:
    answer_failure(answer);
    return;

}

void deal_view_type(void* buf, char* answer)
{
    int i = 0;
    char ip[MAX_IP_LEN] = {0};
    int ip_num = 0;
    int vid = 0;
    struct in_addr ips[32];
    
    get_ip_from_msg(buf,ips,&ip_num);
    
    vid = get_vid_from_msg(buf);

    switch(get_opt_from_msg(buf))
    {
    case VIEW_CREATE_OPT:
    case VIEW_RESTART_OPT:
        if (dns_view_create(vid))
        {
            goto FAILED;
        }
        for (i = 0; i < ip_num ; i ++)        
        {
            memset(ip,0,MAX_IP_LEN);
            strcpy(ip,inet_ntoa(ips[i]));
            if (dns_view_insert(vid, ip))
            {
                dns_view_drop(vid);
                goto FAILED;
            }
        }
        
        break;

    case VIEW_DELETE_OPT:
        dns_view_drop(vid);
        break;

    case VIEW_STOP_OPT:
        /*
        if (dns_view_drop(vid))
        {
            goto FAILED;
        }
        */
        dns_view_drop(vid);
        break;

    case IP_INSERT_OPT:
        if (ip_num != 1)
        {
            goto FAILED;
        }
        memset(ip, 0, MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_view_insert(vid, ip))
        {
            goto FAILED;
        }
        break;

    case IP_REMOVE_OPT:
        if (ip_num != 1)
        {
            goto FAILED;
        }
        memset(ip, 0, MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_view_remove(vid, ip))
        {
            goto FAILED;
        }
        break;

    case IP_MODIFY_OPT:
        if (ip_num != 2)
        {
            goto FAILED;
        }
        
        memset(ip, 0, MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_view_remove(vid, ip))
        {
            goto FAILED;
        }
        
        memset(ip, 0, MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[1]));
        if (dns_view_insert(vid, ip))
        {
            goto FAILED;
        }
        break;

    case IP_RESTART_OPT:
        if (ip_num != 1)
        {
            goto FAILED;
        }
        memset(ip, 0, MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_view_insert(vid, ip))
        {
            goto FAILED;
        }
        break;


    case IP_STOP_OPT:
        if (ip_num != 1)
        {
            goto FAILED;
        }
        memset(ip, 0, MAX_IP_LEN);
        strcpy(ip,inet_ntoa(ips[0]));
        if (dns_view_remove(vid, ip))
        {
            goto FAILED;
        }
        break;
        
    default:
        hyb_debug("Wrong deal view message!\n");
        goto FAILED;
      
    }
    
    answer_sucess(answer);
    return;

FAILED:
    answer_failure(answer);
    return;

}


static int deal_mask_str(char *buf,int *mask, char *ip)
{
    char *temp = NULL;
    int len  = 0;

    temp = strstr(buf,"/");
    if (!temp)
    {
        return -1;
    }

    temp++;
    *mask = atoi(temp);

    len = strcspn(buf,"/");
    if (len == 0)
    {
        return -1;
    }

    memcpy_safe(ip,MAX_IP_LEN,buf,len);

    return 0;
    
}

void deal_mask_type(void* buf, char* answer)
{
    char domain[256] = {0};
    char ip[MAX_IP_LEN] = {0};
    int len = 0;
    int mask = 0;
    int ip_num = 0;
    int vid = 0;
    struct in_addr ips[32];
    
    get_ip_from_msg(buf,ips,&ip_num);
    get_domain_from_msg(buf,domain,&len);

    if (deal_mask_str(domain,&mask,ip))
    {
        goto FAILED;
    }

    //hyb_debug("ip:%s   mask:%d\n",ip,mask);
    vid = get_vid_from_msg(buf);

    switch(get_opt_from_msg(buf))
    {
    
    case MASK_CREATE_OPT:
        if (dns_mask_insert(ip, mask, vid))
        {
            goto FAILED;
        }
        else
        {
            dns_mask_fresh_view_to_control_nowait();
        }
        break;

    case MASK_STOP_OPT:
        if (dns_mask_remove(ip, mask))
        {
            goto FAILED;
        }
        else
        {
            dns_mask_fresh_view_to_control_nowait();
        }
                
        break;
        
    case MASK_RESTART_OPT:
        if (dns_mask_insert(ip, mask, vid))
        {
            goto FAILED;
        }
        else
        {
            dns_mask_fresh_view_to_control_nowait();
        }
        break;


        
    case MASK_DELETE_OPT:
        if (dns_mask_remove(ip, mask))
        {
            goto FAILED;
        }
        else
        {
            dns_mask_fresh_view_to_control_nowait();
        }
        break;
        
    default:
        hyb_debug("Wrong deal mask message!\n");
        goto FAILED;
      
    }
    
    answer_sucess(answer);
    return;

FAILED:
    answer_failure(answer);
    return;

}

/****************************JSON-MSG**************************************/


int dns_msg_pack_init(char *msg)
{
    assert(msg);

    cJSON *root = NULL;
    root = cJSON_CreateObject();
    if (root)
    {

        cJSON_AddStringToObject(root, "class", "init");

        char *msg_js = cJSON_Print(root);
        //printf("Pack a answer :%s\n",msg_js);
        strcpy(msg,msg_js);
        free(msg_js);
        cJSON_Delete(root);

        return 0;
    }

    return -1;

}


int dns_msg_pack_register(char *msg)
{
    assert(msg);

    cJSON *root = NULL;
    root = cJSON_CreateObject();
    if (root)
    {

        cJSON_AddStringToObject(root, "class", "register");

        char *msg_js = cJSON_Print(root);
        //printf("Pack a answer :%s\n",msg_js);
        strcpy(msg,msg_js);
        free(msg_js);
        cJSON_Delete(root);

        return 0;
    }

    return -1;

}



int dns_msg_pack_answer(char *nameclass,int opt,unsigned int msg_id,int type,int view,char *data, int result,char *msg)
{
    assert(msg);
    assert(nameclass);
    assert(data);

    cJSON *root = NULL;
    root = cJSON_CreateObject();
    if (root)
    {

        cJSON_AddStringToObject(root, "class", nameclass);
        cJSON_AddNumberToObject(root, "opt", opt);
        cJSON_AddNumberToObject(root, "id", msg_id);
        cJSON_AddNumberToObject(root, "type", type);
        cJSON_AddNumberToObject(root, "viewid", view);
        cJSON_AddStringToObject(root, "data", data);
        cJSON_AddNumberToObject(root, "result", result);

        char *msg_js = cJSON_Print(root);
        //printf("Pack a answer :%s\n",msg_js);
        strcpy(msg,msg_js);
        free(msg_js);
        cJSON_Delete(root);

        return 0;
    }

    return -1;

}


void dns_msg_cache_request(char *domain,int view_id,int type,unsigned int msg_id)
{
    
    request_to_core(domain,view_id,type,msg_id);
    
}


void dns_msg_cache_delete(char *domain,int view_id,int type,unsigned int msg_id)
{
    int ret = 0;
    char inaddr[512] = {0};
    char *target = domain;

    if (type == 0X000C)  
    {
        char origin[512] = {0};
        
        strcpy_n(origin,512,domain);
            
        if(ip_to_q_name(origin, strlen(origin), inaddr) < 0)
        {
            return;
        }

        target = inaddr;
    }

    //hyb_debug("Delete Target:%s\n");
    
    if (*target == '*')
    {

        ret = dns_ext_cache_drop(target,strlen(target),view_id,type);
        
    }
    else
    {
           
        ret = dns_cache_delete(target,strlen(target),view_id,type);
        
    }

    
    if (ret)
    {
        /*answer fail*/
        answer_to_mgr("dns_reply",CACHE_OPTION_DEL,msg_id,type,view_id,target,MGR_ANSWER_FAILED);
    }
    else
    {
        /*answer sucess*/
        answer_to_mgr("dns_reply",CACHE_OPTION_DEL,msg_id,type,view_id,target,MGR_ANSWER_SUCCESS);
    }

    
}


static void deal_json_str(char *str)
{
    int len = strlen(str);
    memcpy_s(str,str+1,len-1);
    str[len-2] = '\0';
}


int dns_msg_cache_analyze(char *msg)
{
    int i = 0;
   // char *test = 
   // "[{\"opt\": 1, \"domain\": \"www.he.com\", \"type\": 1, \"view\": 2}, {\"opt\": 1, \"domain\": \"he.com\", \"type\": 2, \"view\": 2}]";

    cJSON *json_arr = NULL;
   unsigned int msg_id = 0;
   int opt_id = 0;
   int type_id= 0;
   int view_id = 0;
    
	json_arr = cJSON_Parse(msg);
    if (!json_arr)
    {
        hyb_debug("dns_msg_cache_analyze failed!reason:json parse failed!\n");
        goto FAILED;
    }

    hyb_debug("%s\n",cJSON_Print(json_arr));
    int num = cJSON_GetArraySize(json_arr);

    for (i = 0; i < num; i++)
    {
        char domain_str[256] = {0};
        cJSON *json = cJSON_GetArrayItem(json_arr, i);

        cJSON *id = cJSON_GetObjectItem(json,"id");
        if (id)
        {
            char *id_js = cJSON_Print(id);
            if (id_js)
            {
                msg_id = atoi(id_js);
                //printf("msgid %u %s\n",msg_id, id_js);
                free(id_js);
            }
        }
        
        cJSON *opt = cJSON_GetObjectItem(json,"opt");
        if (opt)
        {
            char *opt_js = cJSON_Print(opt);
            if (opt_js)
            {
                opt_id = atoi(opt_js);
                free(opt_js);
            }
        }
        
        cJSON *domain = cJSON_GetObjectItem(json,"domain");
        if (domain)
        {
            char *domain_js = cJSON_Print(domain);
            strcpy_n(domain_str,256,domain_js);
            deal_json_str(domain_str);
            
            free(domain_js);
            
        }
        else
        {
            goto FAILED;
        }
        
        cJSON *type = cJSON_GetObjectItem(json,"type");
        if (type)
        {
            char *type_js = cJSON_Print(type);
            if (type_js)
            {
                type_id = atoi(type_js);
                free(type_js);
            }
        }
        
        cJSON *view = cJSON_GetObjectItem(json,"view");
        if (view)
        {
            char *view_js = cJSON_Print(view);
            if (view_js)
            {
                view_id = atoi(view_js);
                free(view_js);
            }
        }


        hyb_debug("Receive a cache-set msg[opt:%d domain:%s type:%d view:%d]\n",opt_id,domain_str,type_id,view_id);

        switch(opt_id)
        {
        case CACHE_OPTION_REF:
            dns_msg_cache_request(domain_str,view_id,type_id, msg_id);
            break;
            
        case CACHE_OPTION_DEL:
            dns_msg_cache_delete(domain_str,view_id,type_id, msg_id);
            break;
            
        default:
            hyb_debug("receive uncorrect opt msg!\n");
            break;
        }
        //cJSON_Delete(json);
        
    }
    
    cJSON_Delete(json_arr);

    return 0;
    
FAILED:
    
    cJSON_Delete(json_arr);

    return -1;
  
}


int dns_msg_view_analyze(char *msg)
{
    int i = 0;

    unsigned int msg_id = 0;
    int opt_id = 0;
    int type_id= 0;
    int view_id = 0;

    cJSON *json_arr = NULL;
	json_arr = cJSON_Parse(msg);
    if (!json_arr)
    {
        hyb_debug("dns_msg_view_analyze failed!reason:json parse failed!\n");
        return 0;
    }

    int num = cJSON_GetArraySize(json_arr);

    for (i = 0; i < num; i++)
    {
        char mask_str[64] = {0};
        char ip[MAX_IP_LEN] = {0};
        int mask_num = 0;
        
        cJSON *json = cJSON_GetArrayItem(json_arr, i);

        
        cJSON *opt = cJSON_GetObjectItem(json,"opt");
        if (opt)
        {
            char *opt_js = cJSON_Print(opt);
            if (opt_js)
            {
                opt_id = atoi(opt_js);
                free(opt_js);
            }
        }

        cJSON *id = cJSON_GetObjectItem(json,"id");
        if (id)
        {
            char *id_js = cJSON_Print(id);
            if (id_js)
            {
                msg_id = atoi(id_js);
                free(id_js);
            }
        }
        
        cJSON *view = cJSON_GetObjectItem(json,"view");
        if (view)
        {
            char *view_js = cJSON_Print(view);
            if (view_js)
            {
                view_id = atoi(view_js);
                free(view_js);
            }
        }

        
        cJSON *mask = cJSON_GetObjectItem(json,"mask");
        if (mask)
        {
            char *mask_js = cJSON_Print(mask);
            if (mask_js)
            {
                strcpy_n(mask_str, 64 ,mask_js);
                deal_json_str(mask_str);
                free(mask_js);
            }
        }


        if (deal_mask_str(mask_str,&mask_num,ip))
        {
            answer_to_mgr("view_reply",opt_id,msg_id,0,view_id,mask_str,MGR_ANSWER_FAILED);
        }
          
        //hyb_debug("Receive a view-set msg[opt:%d mask:%s view:%d]\n", opt_id, mask_str, view_id);
        
        switch(opt_id)
        {
            
        case VIEW_OPTION_ADD:

            /*先看看是否存在*/
            dns_view_create(view_id);
            
            if (dns_mask_insert(ip, mask_num, view_id))
            {
                answer_to_mgr("view_reply",VIEW_OPTION_ADD,msg_id,0,view_id,mask_str,MGR_ANSWER_NOEXIST);
            }
            else
            {
                dns_mask_fresh_view_to_control_nowait();
                answer_to_mgr("view_reply",VIEW_OPTION_ADD,msg_id,0,view_id,mask_str,MGR_ANSWER_SUCCESS);
            }
            break;
            
        case VIEW_OPTION_DEL:
            if (dns_mask_remove(ip, mask_num))
            {
                answer_to_mgr("view_reply",VIEW_OPTION_DEL,msg_id,0,view_id,mask_str,MGR_ANSWER_FAILED);
            }
            else
            {
                dns_mask_fresh_view_to_control_nowait();
                answer_to_mgr("view_reply",VIEW_OPTION_DEL,msg_id,0,view_id,mask_str,MGR_ANSWER_SUCCESS);
            }
            break;
            
        default:
            hyb_debug("receive uncorrect opt msg!\n");
            break;
        }

        //cJSON_Delete(json);
        
    }
    cJSON_Delete(json_arr);

    return 0;
  
}


