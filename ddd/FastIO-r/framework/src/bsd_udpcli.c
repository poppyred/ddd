//	$Id: bsd_udpcli.c 2014-12-18 likunxiang$

#include "bsd_udpcli.h"
#include "fio_sysconfig.h"
#include "fio_nm_util.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

int buc_write(int sockfd, char *data, int len)
{
    int ret = 0;
    if (( ret = send(sockfd, data, len, 0)) < 0){}
    return ret;
}

int buc_open(char *svr_ip, uint16_t svr_port)
{
    int sockfd;
    struct sockaddr_in servaddr;

    /* init servaddr */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(svr_port);
    if(inet_pton(AF_INET, svr_ip, &servaddr.sin_addr) <= 0)
    {
        OD("[%s] is not a valid IPaddress", svr_ip);
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* connect to server */
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        OD("connect to [%s:%d] error", svr_ip, svr_port);
        //return -1; //这里不要返回，这里不要这么严谨
    }

    //do_cli(stdin, sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    return sockfd;
}
