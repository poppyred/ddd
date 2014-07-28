#include <stdio.h>
#include "dns_port.h"
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>


#define hyb_debug(fmt, args ...) \
    do {printf("[%s][%d]:"fmt, __FILE__, __LINE__,##args);}while(0)


#define MAX_PORT_NUM    5000


int g_socket_queue[MAX_PORT_NUM] = {0};
unsigned short g_port_id = 1;

static pthread_mutex_t port_lock;


static unsigned short getport()
{
    unsigned short result = 0;
    pthread_mutex_lock(&port_lock);
    result = g_port_id++;
    if (result == 0)
    {
        result++;
    }
    pthread_mutex_unlock(&port_lock);
    return result;
}



int dns_port_init()
{
    int i = 0;
    struct sockaddr_in temp;

    for (i = 0;i < MAX_PORT_NUM; i++)
    {
        g_socket_queue[i] = socket(PF_INET, SOCK_DGRAM, 0);
        bzero(&temp, sizeof(struct sockaddr_in));
	    temp.sin_addr.s_addr = htonl(INADDR_ANY);
	    temp.sin_port = htons(i+MAX_PORT_NUM);
        temp.sin_family = AF_INET;

        if(bind(g_socket_queue[i],(struct sockaddr *)&temp,sizeof(struct sockaddr_in)) < 0) 
        {
            hyb_debug("Bind Error:%sn",strerror(errno));
            return -1;
        }
    }

    return 0;
}


unsigned short dns_get_port()
{
    unsigned short portid = getport();
    return (portid%MAX_PORT_NUM + MAX_PORT_NUM);
    //return (getport()%MAX_PORT_NUM + MAX_PORT_NUM);
}



int dns_port_to_socket(unsigned short port)
{
    if (port < 5000 || port > 9999)
    {
        return -1;
    }

    return g_socket_queue[port - MAX_PORT_NUM];
}
