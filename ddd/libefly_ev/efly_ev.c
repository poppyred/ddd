#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>


#define hyb_debug(fmt, args ...) \
    do {fprintf(stderr,"[%s][%d]:"fmt, __FILE__, __LINE__,##args);}while(0)
        

/* -------system -------- */

#include "event.h"
#include "libbase.h"
#include "efly_ev.h"

#define MAX_PATH_LEN    (25)
#define LEN_OF_LISTEN   (100)
#define RECV_BUF_LEN    (30000)
#define ANSWER_BUF_LEN  (20)


struct ev_timer
{
    struct list_head list;
    struct timeval timeout;
    struct event *event;
    void *user_data;
    event_timer_callback_fun user_func;
    
};


typedef struct efly_ipc_cb
{
    event_ipc_callback_fun cb;
    
}efly_ipc_cb;


struct efly_ipc_svr
{
    h_hash_st *hash;
    struct event *ev;
    
};


struct client
{
    int fd;
    struct event ev_read;
    h_hash_st *hash;
    char srcname[MAX_PATH_LEN];
    
};



struct event_base *g_srv_base;

struct list_head g_member_list;


int efly_event_init()
{
    INIT_LIST_HEAD(&g_member_list);
    g_srv_base = event_init();
    if (!g_srv_base)
    {   
        hyb_debug("event init failed!\n");
        return -1;
    }

    return 0;
}



int32_t efly_event_loop()
{
    return event_loop(0);
}



int32_t efly_event_loop_once()
{
    return event_loop(EVLOOP_ONCE);
}




static void Func(int fd, short n, void *arg)
{
    assert(arg);
    ev_timer *ev_t = NULL;

    ev_t = (ev_timer *)arg;

    ev_t->user_func(ev_t->user_data);

    if (ev_t->timeout.tv_sec != 0 || ev_t->timeout.tv_usec != 0)
    {
        
        evtimer_set(ev_t->event, Func, ev_t);
        evtimer_add(ev_t->event, &ev_t->timeout);
    }
    else
    {
        list_del_init(&ev_t->list);
        efly_timer_unset(ev_t);
        hyb_debug("del timer!\n");
    }
    
}



ev_timer *efly_timer_set(int32_t f_timeout, int32_t timeout,
        event_timer_callback_fun cb, void *arg)
{
    ev_timer *ev_t = NULL;

    ev_t = (ev_timer *)h_malloc(sizeof(ev_timer));
    if (!ev_t)
    {
        return NULL;
    }

    ev_t->event = (struct event *)h_malloc(sizeof(struct event));
    if (!ev_t->event)
    {
        return NULL;
    }

    if (f_timeout >= 1000)
    {
        ev_t->timeout.tv_sec = f_timeout/1000;
    }
    else
    {
        ev_t->timeout.tv_usec = f_timeout*1000;
    }
    ev_t->user_data = arg;
    ev_t->user_func = cb;
    
    
    evtimer_set(ev_t->event, Func, ev_t);
    evtimer_add(ev_t->event, &ev_t->timeout);

    ev_t->timeout.tv_sec = 0;
    ev_t->timeout.tv_usec = 0;
    
    if (timeout >= 1000)
    {
        ev_t->timeout.tv_sec = timeout/1000;
    }
    else
    {
        ev_t->timeout.tv_usec = timeout*1000;
    }

    INIT_LIST_HEAD(&ev_t->list);
    list_add_tail(&ev_t->list,&g_member_list);

    return ev_t;
}


void efly_timer_unset(ev_timer *ev_t)
{
    if (!ev_t)
        return;

    if (ev_t->event)
    {
        evtimer_del(ev_t->event);
        h_free(ev_t->event);
        ev_t->event = NULL;
        list_del(&ev_t->list);
    }

    h_free(ev_t);
    
}

void efly_timer_clear()
{
    ev_timer *pos,*n = NULL;

    list_for_each_entry_safe(pos,n,&g_member_list,list)
    {
        efly_timer_unset(pos);
    }
    
}


static int32_t read_sock(int32_t fd, void *buffer, uint32_t length)
{
    //assert(fd >= 0);
    //assert(buffer);
    uint32_t nleft = 0;
    int32_t  nread = 0;
    char *ptr;

    if (fd < 0)
    {
        return -1;
    }
    
    ptr = (char *)buffer;
    nleft = length;
    while(nleft > 0)
    {
        if((nread = read(fd, ptr, nleft)) < 0)
        {
            if(errno == EAGAIN)
            {
                nread = 0;
                continue;
            }
            else
            {
                return length - nleft;
            }
        }
        else if(nread == 0)
        {
            break;
        }

        nleft -= nread;
        ptr += nread;
    }
    
    return length - nleft;
}


static int32_t write_sock(int32_t fd, void *buffer, uint32_t length)
{
    //assert(fd >= 0);
    //assert(buffer);
    
    uint32_t nleft;
    int32_t nwritten;
    int32_t nwrite;
    char *ptr;

    if (fd < 0)
    {
        return -1;
    }

    ptr = (char *)buffer;
    nleft = length;
    while(nleft > 0)
    {
        if((nwrite = write(fd, ptr, nleft))<=0)
        {
            if(errno == EAGAIN)
            {
                nwrite = 0;
            }
            else
            {
                return -1;
            }
        }

        nleft -= nwrite;
        ptr += nwrite;
        nwritten += nwrite; 
    }

    return nwritten;

}



//回显数据
static void on_read(int fd, short ev, void *arg)
{
    assert(arg);
    struct client *cli = (struct client *)arg;
    char req[RECV_BUF_LEN] = {0};
    char answer[ANSWER_BUF_LEN] = {0};
    int32_t len = 0;
    unsigned short id = 1;
    efly_ipc_cb *cb = NULL;

    len = read_sock(fd, req, RECV_BUF_LEN);
    if (len == 0) 
    {
        /* 客户端断开连接，在这里移除读事件并且释放客户数据结构 */
        //hyb_debug("Client disconnected from socket:%s.\n",cli->srcname);
        evutil_closesocket(fd);
        event_del(&cli->ev_read);
        h_free(cli);
        return;
    }

    hyb_debug("received len:%d\n", len);

    
    memcpy(&id,req,sizeof(unsigned short));

    if (h_hash_search(cli->hash ,&id ,sizeof(id),(void **)&cb) == 0)
    {
        cb->cb(req+sizeof(unsigned short), len ,cli->srcname,answer);
    }

    (void)write_sock(fd, answer, ANSWER_BUF_LEN);

    //hyb_debug("send msg: %s to %s\n", answer, cli->srcname);
    
}




static void accept_cb(int fd, short n, void *arg)
{
    assert(arg);

    //hyb_debug("callback!\n");
    
    int client_fd;
    struct sockaddr_in client_addr = {0};
    size_t client_len = sizeof(client_addr);
    struct client *cli = NULL;
    //char *name = NULL;
    
    efly_ipc_svr *svr = (efly_ipc_svr *)arg;
    if (!svr->hash)
    {
        return ;
    }
    
    client_fd = accept(fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
    if (client_fd < 0) 
    {
        hyb_debug("Accept Failed\n");
        return;
    }

    /*非堵塞*/
    evutil_make_socket_nonblocking(client_fd);
    
    //hyb_debug("Accepted connection from %s\n",client_addr.sun_path);

    cli = (struct client*)h_malloc(sizeof(*cli));
    if (!cli) 
    {
        hyb_debug("alloc client failed\n");
        evutil_closesocket(client_fd);
        return;
    }
    
    cli->hash = svr->hash;
    //name = (char *)&client_addr.sun_path;
    strcpy_n(cli->srcname, MAX_PATH_LEN, inet_ntoa(client_addr.sin_addr));
    event_set(&cli->ev_read, client_fd, EV_READ|EV_PERSIST, on_read, cli);
    event_add(&cli->ev_read, NULL);
}


static void ipc_table_free(void *data)
{
    efly_ipc_cb* cb = (efly_ipc_cb *)data;
    h_free(cb);
}



efly_ipc_svr *efly_ipc_init(char *pname)
{

    int reuse = 1;
    //char path[MAX_PATH_LEN] = {0};
    //struct sockaddr_un addr = {0};
    struct sockaddr_in addr = {0};
    struct event *accept_event = NULL;
    struct efly_ipc_svr *svr = NULL;

    int socketlisten = socket(PF_INET, SOCK_STREAM, 0);
    if (0 > socketlisten)
    {
        hyb_debug("failed to create listen socket\n");
        return NULL;
    }
    
    addr.sin_family = AF_INET;
    //sprintf_n(path, MAX_PATH_LEN, "/tmp/%s.sock",pname);  
    //strcpy_n(addr.sun_path, MAX_PATH_LEN, path);
    //unlink(path);
    //umask(0);

    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(12345);
    
    if (bind(socketlisten, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        hyb_debug("bind listen socket failed\n");
        goto FAILED;
    }

    if (listen(socketlisten, LEN_OF_LISTEN) < 0) 
    {
        hyb_debug("listen socket failed\n");
        goto FAILED;
    }

    svr = (struct efly_ipc_svr *)h_malloc(sizeof(struct efly_ipc_svr));
    if (!svr)
    {
        goto FAILED;
    }  

    accept_event = (struct event *)h_malloc(sizeof(struct event));
    if (!accept_event)
    {
        goto FAILED;
    }

    svr->hash = h_hash_create(ipc_table_free, NULL, 0);
    if (!svr->hash)
    {
        goto FAILED;
    }

    svr->ev = accept_event;
    evutil_make_socket_nonblocking(socketlisten);
    setsockopt(socketlisten, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, 
            sizeof(reuse));

    
    event_assign(accept_event, g_srv_base, socketlisten, EV_READ | EV_PERSIST, 
            accept_cb, svr);

    event_add(accept_event, NULL);

    return svr;

FAILED:
    if (accept_event)
        h_free(accept_event);
    if (svr)
        h_free(svr);
    evutil_closesocket(socketlisten);
    
    return NULL;
}



void efly_ipc_destroy(efly_ipc_svr *svr)
{
    if (!svr)
        return;

    if (svr->hash)
    {
        h_hash_destroy(svr->hash);
    }
    
    if (svr->ev)
    {
        h_free(svr->ev);
    }

    h_free(svr);
    
}




int32_t efly_ipc_reg_func(efly_ipc_svr *svr, unsigned short id, 
        event_ipc_callback_fun cb)
{
    efly_ipc_cb *ipc_cb = NULL;
    
    if (!svr)
    {
        return -1;
    } 

    ipc_cb = (efly_ipc_cb *)h_malloc(sizeof(efly_ipc_cb));
    if(!ipc_cb)
    {
        return -1;
    }

    ipc_cb->cb = cb;
    
    return h_hash_insert(svr->hash, &id, sizeof(id), ipc_cb);

}


