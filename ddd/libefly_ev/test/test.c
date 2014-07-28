#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "efly_ev.h"

void cb(void *user_data)
{
    char *str = (char *)user_data;
    printf("str:%s\n",str);
}

void cb2(void *user_data)
{
    printf("gay!\n");
    
}


int get_view_from_msg(char *msg)
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
    int d_len = 0;
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




void fun(void *buf, int32_t b_len, char *src,char *answer)
{
    char domain[256] = {0};
    int len = 0;
    int num = 0;   
    struct in_addr ips[10];
    int i = 0;


    
    printf("get type - %d\n",get_view_from_msg(buf));
    printf("get vid - %d\n",get_vid_from_msg(buf));
    printf("get opt - %d\n",get_opt_from_msg(buf));
    get_domain_from_msg(buf,domain,&len);
    printf("get domain - %s(%d)\n",domain,len);
    get_ip_from_msg(buf,ips,&num);

    printf("get ip - (%d)\n",num);
    for (i = 0; i < num ; i ++)
    {
        printf("get ip - %s\n",inet_ntoa(ips[i]));
    }

    memcpy(answer,"01",2);
    
}



int main()
{
    char *str = "hello swen!";
    char *str2 = "i am fine!";
    signal(SIGPIPE, SIG_IGN);
    
    efly_event_init();

    //efly_timer_set(1000,3000, cb, str);

    efly_ipc_svr *svr = NULL;

    //efly_timer_set(4000,4000, cb, str2);
    //efly_timer_set(6000,1000, cb2, str2);
    
    
    svr = efly_ipc_init("dns");

    if (svr)
    {
        efly_ipc_reg_func(svr, 1, fun);
        efly_event_loop();
    }

    else
        return 0;
}


