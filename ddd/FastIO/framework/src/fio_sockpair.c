#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/un.h> 
#include "fio_sockpair.h"
#include "fio_nm_util.h"
//#include "log_log.h"
#include "fio_sysconfig.h"

int fio_socketpair(int af, struct in_addr *if_ip, uint16_t sport, uint16_t delta, int type, int protocol, int fd[2])
{
    int listen_socket;
    struct sockaddr_in sin[2];
    int len;
    int itrue = 1;
    bzero(&sin[0], sizeof(struct sockaddr_in));
    bzero(&sin[1], sizeof(struct sockaddr_in));
    /* The following is only valid if type == SOCK_STREAM */
    if (type != SOCK_STREAM)
        return -1;
    /* Create a temporary listen socket; temporary, so any port is good */
    listen_socket = socket(af, type, protocol);
    if (listen_socket < 0)
    {
        OD( "creating listen_socket error");
        return -1;
    }
    itrue = 1;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&itrue, sizeof(int)) == -1) 
    {
        OD( "Setsockopt error");
        return -1;
    }
    sin[0].sin_family = af;
    sin[0].sin_port = htons(sport); /* 0; Use any port number */
    //sin[0].sin_addr.s_addr = INADDR_ANY;
    sin[0].sin_addr = *if_ip;
    if (bind(listen_socket, (struct sockaddr*)&sin[0], sizeof(sin[0])) < 0)
    {
        OD( "unable to bind");
        return -1;
    }
    len = sizeof(sin[0]);
    /* Read the port number we got, so that our client can connect to it */
    if (getsockname(listen_socket, (struct sockaddr*)&sin[0], (socklen_t*)&len) < 0)
    {
        OD( "getsockname error");
        return -1;
    }
    /* Put the listen socket in listening mode */
    if (listen(listen_socket, 5) < 0)
    {
        OD( "listen error");
        return -1;
    }
    /* Create the client socket */
    fd[1] = socket(af, type, protocol);
    if (fd[1] < 0)
    {
        OD( "creating client_socket error");
        return -1;
    }
    itrue = 1;
    if (setsockopt(fd[1], SOL_SOCKET, SO_REUSEADDR, (const char*)&itrue, sizeof(int)) == -1) 
    {
        OD( "Setsockopt error");
        return -1;
    }

    sin[1].sin_family = af;
    sin[1].sin_port = htons(sport+delta);
    sin[1].sin_addr = *if_ip;
    if (bind(fd[1], (struct sockaddr*)&sin[1], sizeof(sin[1]))<0)  
    {  
        OD( "bind to port %d failure!", sport+delta);  
        return -1;
    }  

    /* Put the client socket in non-blocking connecting mode */
    fcntl(fd[1], F_SETFL, fcntl(fd[1], F_GETFL, 0) | O_NONBLOCK);
    if (connect(fd[1], (struct sockaddr*)&sin[0], sizeof(sin[0])) < 0)
    {
        OD( "connect error");
        //return -1;
    }
    /* At the listen-side, accept the incoming connection we generated */
    len = sizeof(sin[1]);
    if ((fd[0] = accept(listen_socket, (struct sockaddr*)&sin[1], (socklen_t*)&len)) < 0)
    {
        OD( "accept error");
        return -1;
    }
    /* Reset the client socket to blocking mode */
    //fcntl(fd[1], F_SETFL, fcntl(fd[1], F_GETFL, 0) & ~O_NONBLOCK);
    itrue = 1;
    if (setsockopt(fd[0], SOL_SOCKET, SO_REUSEADDR, &itrue, sizeof(int)) == -1) 
    {
        OD( "Setsockopt error");
        return -1;
    }
    fcntl(fd[0], F_SETFL, fcntl(fd[0], F_GETFL, 0) | O_NONBLOCK);
    close(listen_socket);
    return 0;
}

#define NFIO_DOMAIN "/var/nfio.domain" 
int fio_usocketpair(struct in_addr *if_ip, uint16_t sport, int fd[2])
{
    int listen_socket;
    struct sockaddr_un sin[2];
    char sk_path[256];
    int len;
    bzero(&sin[0], sizeof(struct sockaddr_un));
    bzero(&sin[1], sizeof(struct sockaddr_un));
    listen_socket = socket(AF_UNIX,SOCK_STREAM,0); 
    if (listen_socket < 0)
    {
        OD( "creating listen_socket error");
        return -1;
    }
    sin[0].sun_family = AF_UNIX;
    snprintf(sk_path, sizeof(sk_path)-1, "%s.%d", NFIO_DOMAIN, sport);
    strncpy(sin[0].sun_path,sk_path,sizeof(sin[0].sun_path)-1); 
    unlink(sk_path); 
    if (bind(listen_socket, (struct sockaddr*)&sin[0], sizeof(sin[0])) < 0)
    {
        OD( "unable to bind");
        return -1;
    }
    len = sizeof(sin[0]);
    if (getsockname(listen_socket, (struct sockaddr*)&sin[0], (socklen_t*)&len) < 0)
    {
        OD( "getsockname error");
        return -1;
    }
    if (listen(listen_socket, 5) < 0)
    {
        OD( "listen error");
        return -1;
    }
    chmod(sk_path, 00777);//设置通信文件权限
    fd[1] = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd[1] < 0)
    {
        OD( "creating client_socket error");
        return -1;
    }
    sin[1].sun_family = AF_UNIX;
    strncpy(sin[1].sun_path,sk_path,sizeof(sin[1].sun_path)-1); 
    fcntl(fd[1], F_SETFL, fcntl(fd[1], F_GETFL, 0) | O_NONBLOCK);
    if (connect(fd[1], (struct sockaddr*)&sin[1], sizeof(sin[1])) < 0)
    {
        OD( "connect error");
    }
    len = sizeof(sin[1]);
    if ((fd[0] = accept(listen_socket, (struct sockaddr*)&sin[1], (socklen_t*)&len)) < 0)
    {
        OD( "accept error");
        return -1;
    }
    fcntl(fd[0], F_SETFL, fcntl(fd[0], F_GETFL, 0) | O_NONBLOCK);
    close(listen_socket);
    return 0;
}
