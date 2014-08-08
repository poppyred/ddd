#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "libbase.h"
#include "dns_lcllog.h"
#include "dns_pktlog.h"
#include "dns_comdef.h"


#define GENERAL_LCLLOG_FORMAT   "%ld\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n"
#define VIEW_LCLLOG_FORMAT   "%ld\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n"

#define ANSWER_LCLLOG_FORMAT    "%ld\t%d\t%s\t%s\n"
#define ERRDST_LCLLOG_FORMAT    "%ld\t%s\t%d\t%s\n"

#define REQCNT_LCLLOG_FORMAT    "%ld\t%s\t%d\t%d\n"


int g_cache_conut = 0;
int g_record_conut = 0;
int g_answer_conut = 0;
int g_errdst_conut = 0;
int g_allreq_conut = 0;
int g_illreq_conut = 0;
int g_dnsreq_conut = 0;


int g_stop = 0;

#define PTHREAD_NUMBER 24

typedef struct st_lcldst_path
{
    char path[256];
    int count;
    FILE *fp;
    
}st_lcldst_path;

typedef struct st_lclview
{
    int is_print;
    int cache;
    int record;
    int answer;
    int errdst;
    int dnsreq;
    int allreq;
    int illreq;
    
}st_lclview;


typedef struct st_logserver
{
    h_hash_st *count_table;
    //int count; 
    
}st_logserver;


typedef struct st_reqcnt_node
{
    int cnt[MAX_VIEM_NUM];
    
}st_reqcnt_node;



st_logserver g_reqcnt_loger[PTHREAD_NUMBER];


st_lcldst_path *g_general_path = NULL;
st_lcldst_path *g_answer_path = NULL;
st_lcldst_path *g_errdst_path = NULL;
st_lcldst_path *g_view_path = NULL;
st_lcldst_path *g_reqcnt_path = NULL;


st_lclview *g_view_data = NULL;


int g_pnum = 0;



extern struct tm *g_tm;
extern time_t global_now;


void dns_lcllog_cache_count()
{
    g_cache_conut++;
}

void dns_lcllog_record_count()
{
    g_record_conut++;
}

void dns_lcllog_answer_count()
{
    g_answer_conut++;
}

void dns_lcllog_errdst_count()
{
    g_errdst_conut++;
}

void dns_lcllog_allreq_count()
{
    g_allreq_conut++;
}

void dns_lcllog_illreq_count()
{
    g_illreq_conut++;
}

void dns_lcllog_dnsreq_count()
{
    g_dnsreq_conut++;
}




static void dns_lcllog_cache_reset()
{
    g_cache_conut = 0;
}

static void dns_lcllog_record_reset()
{
    g_record_conut = 0;
}

static void dns_lcllog_answer_reset()
{
    g_answer_conut = 0;
}

static void dns_lcllog_errdst_reset()
{
    g_errdst_conut = 0;
}

static void dns_lcllog_allreq_reset()
{
    g_allreq_conut = 0;
}

static void dns_lcllog_illreq_reset()
{
    g_illreq_conut = 0;
}

static void dns_lcllog_dnsreq_reset()
{
    g_dnsreq_conut = 0;
}


static void make_path()
{
    char dir_path[256] = {0};
    int i = 0;

    g_stop = 1;
    
    mkdir("./log/",0777);
    mkdir("./log/answer/",0777);
    mkdir("./log/errdst/",0777);

    sprintf_n(dir_path,256,"./log/answer/%04d-%02d-%02d/",g_tm->tm_year+1900, g_tm->tm_mon+1,g_tm->tm_mday);

    mkdir(dir_path,0777);

    memset(dir_path,0,256);

    sprintf_n(dir_path,256,"./log/errdst/%04d-%02d-%02d/",g_tm->tm_year+1900, g_tm->tm_mon+1,g_tm->tm_mday);

    mkdir(dir_path,0777);
    
    for(i = 0; i < g_pnum; i++)
    {
        sprintf_n(g_answer_path[i].path,256,"./log/answer/%04d-%02d-%02d/%02d",
            g_tm->tm_year+1900, g_tm->tm_mon+1,g_tm->tm_mday,i);

        if (g_answer_path[i].fp)
        {
            fclose(g_answer_path[i].fp);
            g_answer_path[i].fp = NULL;
        }
        g_answer_path[i].fp = fopen(g_answer_path[i].path,"a+");

        
        sprintf_n(g_errdst_path[i].path,256,"./log/errdst/%04d-%02d-%02d/%02d",
            g_tm->tm_year+1900, g_tm->tm_mon+1,g_tm->tm_mday,i);
        
        if (g_errdst_path[i].fp)
        {
            fclose(g_errdst_path[i].fp);
            g_errdst_path[i].fp = NULL;
        }

        g_errdst_path[i].fp = fopen(g_errdst_path[i].path,"a+");

        
    }

    mkdir("./log/general/",0777);
    sprintf_n(g_general_path[0].path,256,"./log/general/%04d-%02d-%02d",
            g_tm->tm_year+1900, g_tm->tm_mon+1,g_tm->tm_mday);

    mkdir("./log/view/",0777);
    sprintf_n(g_view_path[0].path,256,"./log/view/%04d-%02d-%02d",
            g_tm->tm_year+1900, g_tm->tm_mon+1,g_tm->tm_mday);


    mkdir("./log/reqcnt/",0777);
    sprintf_n(g_reqcnt_path[0].path,256,"./log/reqcnt/%04d-%02d-%02d",
            g_tm->tm_year+1900, g_tm->tm_mon+1,g_tm->tm_mday);

    g_stop = 0;
}

static void reqcnt_loger_table_desteoy(void *data)
{
    st_reqcnt_node*node = NULL;
    node = (st_reqcnt_node *)data;

    h_free(node);
}



int dns_lcllog_init(int pnum)
{
    g_general_path = (st_lcldst_path *)h_malloc(sizeof(st_lcldst_path));
    if (!g_general_path)
    {
        return -1;
    }

    g_answer_path = (st_lcldst_path *)h_malloc(pnum*sizeof(st_lcldst_path));
    if (!g_answer_path)
    {
        return -1;
    }

    g_errdst_path = (st_lcldst_path *)h_malloc(pnum*sizeof(st_lcldst_path));
    if (!g_errdst_path)
    {
        return -1;
    }

    g_view_path = (st_lcldst_path *)h_malloc(sizeof(st_lcldst_path));
    if (!g_view_path)
    {
        return -1;
    }


    g_reqcnt_path = (st_lcldst_path *)h_malloc(sizeof(st_lcldst_path));
    if (!g_reqcnt_path)
    {
        return -1;
    }
    

    g_view_data = (st_lclview *)h_malloc(100 *sizeof(st_lclview));
    if (!g_view_data)
    {
        return -1;
    }

    int i =0;
    for ( i = 0; i< pnum ; i++)
    {
        g_reqcnt_loger[i].count_table = h_hash_create(reqcnt_loger_table_desteoy,NULL,0);
        if(!g_reqcnt_loger[i].count_table)
        {
            return -1;
        }
    }
    
    g_pnum = pnum;

    make_path();

    return 0;
}


void dns_lcllog_remake_path()
{
    make_path();
}


void dns_lcllog_general()
{
    if (g_stop)
    {
        return;
    }
    FILE *fp = fopen(g_general_path[0].path,"a+");
    if (!fp)
    {
        return;
    }

    char msg[128] = {0};


    sprintf_n(msg,128,GENERAL_LCLLOG_FORMAT,global_now,g_allreq_conut,g_cache_conut,g_record_conut,
                g_errdst_conut,g_answer_conut,g_illreq_conut,g_dnsreq_conut);
    fwrite(msg,strlen(msg),1,fp);
    memset(msg,0,128);

    fclose(fp);

    dns_lcllog_cache_reset();
    dns_lcllog_record_reset();
    dns_lcllog_answer_reset();
    dns_lcllog_errdst_reset();    
    dns_lcllog_allreq_reset();
    dns_lcllog_illreq_reset();
    dns_lcllog_dnsreq_reset();
}



void dns_lcllog_answer(int pid,int view,char *domain,int server)
{
    if (g_stop)
    {
        return;
    }
    FILE *fp = g_answer_path[pid].fp;
    if (!fp)
    {
        return;
    }

    char msg[256] = {0};

    sprintf_n(msg,256,ANSWER_LCLLOG_FORMAT,global_now,view,domain,inet_ntoa(*(struct in_addr *)&server));

//    hyb_debug("dns_lcllog_answer to [%s]: %s",g_answer_path[pid].path,msg);
    g_answer_path[pid].count ++;
    fwrite(msg,strlen(msg),1,fp);
    fflush(fp);

}

void dns_lcllog_errdst(int pid,int view,char *domain,int sip)
{
    if (g_stop)
    {
        return;
    }
    FILE *fp = g_errdst_path[pid].fp;
    if (!fp)
    {
        return;
    }

    
    char msg[256] = {0};

    sprintf_n(msg,256,ERRDST_LCLLOG_FORMAT,global_now,inet_ntoa(*(struct in_addr *)&sip),view,domain);

//    hyb_debug("dns_lcllog_errdst to [%s]: %s",g_errdst_path[pid].path,msg);
    g_errdst_path[pid].count ++;
    fwrite(msg,strlen(msg),1,fp);
    fflush(fp);

}


/******************************************************************************/



void dns_lcllog_cache_viewcount(int view_id)
{
    if(!g_view_data[view_id].is_print)
    {
        g_view_data[view_id].is_print = 1;
    }
    g_view_data[view_id].cache++;
}

void dns_lcllog_record_viewcount(int view_id)
{
    if(!g_view_data[view_id].is_print)
    {
        g_view_data[view_id].is_print = 1;
    }
    g_view_data[view_id].record++;
}

void dns_lcllog_answer_viewcount(int view_id)
{
    if(!g_view_data[view_id].is_print)
    {
        g_view_data[view_id].is_print = 1;
    }
    g_view_data[view_id].answer++;
}

void dns_lcllog_errdst_viewcount(int view_id)
{
    if(!g_view_data[view_id].is_print)
    {
        g_view_data[view_id].is_print = 1;
    }
    g_view_data[view_id].errdst++;
}

void dns_lcllog_dnsreq_viewcount(int view_id)
{
    if(!g_view_data[view_id].is_print)
    {
        g_view_data[view_id].is_print = 1;
    }
    g_view_data[view_id].dnsreq++;
}

void dns_lcllog_allreq_viewcount(int view_id)
{
    if(!g_view_data[view_id].is_print)
    {
        g_view_data[view_id].is_print = 1;
    }
    g_view_data[view_id].allreq++;
}

void dns_lcllog_illreq_viewcount(int view_id)
{
    if(!g_view_data[view_id].is_print)
    {
        g_view_data[view_id].is_print = 1;
    }
    g_view_data[view_id].illreq++;
}



static void view_cache_reset(int view_id)
{
    g_view_data[view_id].cache = 0;
}

static void view_record_reset(int view_id)
{
    g_view_data[view_id].record= 0;
}

static void view_answer_reset(int view_id)
{
    g_view_data[view_id].answer= 0;
}

static void view_errdst_reset(int view_id)
{
    g_view_data[view_id].errdst= 0;
}

static void view_dnsreq_reset(int view_id)
{
    g_view_data[view_id].dnsreq= 0;
}

static void view_allreq_reset(int view_id)
{
    g_view_data[view_id].allreq= 0;
}

static void view_illreq_reset(int view_id)
{
    g_view_data[view_id].illreq= 0;
}



void dns_lcllog_view()
{
    if (g_stop)
    {
        return;
    }
    char msg[128] = {0};
    FILE *fp = fopen(g_view_path[0].path,"a+");
    if (!fp)
    {
        return;
    }

    int i = 0;
    for (i = 0;i < 100;i++)
    {
        if (!g_view_data[i].is_print)
        {
            continue;
        }

        sprintf_n(msg,128,VIEW_LCLLOG_FORMAT,global_now,i,g_view_data[i].allreq,g_view_data[i].cache,g_view_data[i].record,
                g_view_data[i].errdst,g_view_data[i].answer,g_view_data[i].illreq,g_view_data[i].dnsreq);
        fwrite(msg,strlen(msg),1,fp);
        memset(msg,0,128);

      
        view_cache_reset(i);
        view_record_reset(i);
        view_answer_reset(i);
        view_errdst_reset(i);
        view_dnsreq_reset(i);
        view_allreq_reset(i);
        view_illreq_reset(i);
        g_view_data[i].is_print = 0;
    }

    fclose(fp);

}



/******************************************************************************/



void dns_lcllog_reqcnt_log(char *domain,int dlen,int view_id,int pid)
{

    h_hash_st *tar = g_reqcnt_loger[pid].count_table;
    st_reqcnt_node *node = NULL;

    hyb_debug("dns_lcllog_reqcnt_log domain[%d]:%s\n",view_id,domain);

    if (h_hash_search(tar,domain,dlen,(void **)&node) == 0)
    {
        node->cnt[view_id] ++;
    }
    else
    {
        node = (st_reqcnt_node*)h_malloc(sizeof(st_reqcnt_node));

        node->cnt[view_id] ++;
        h_hash_insert(tar,domain,dlen,node);
    }


}


static int reqcnt_log_hash_walk(const void *key, int klen, void *val, void *data)
{

    char name[128] = {0};
    char msg[256] = {0};
    FILE *fp = (FILE *)data;
    char *domain = (char *)key;
    st_reqcnt_node *node = (st_reqcnt_node *)val;

    memcpy(name,domain,klen);

    int i = 0;
    for (i = 0; i < MAX_VIEM_NUM; i ++)
    {
        if (node->cnt[i])
        {
            sprintf_n(msg,256,REQCNT_LCLLOG_FORMAT,global_now, name , i, node->cnt[i]);
            fwrite(msg,strlen(msg),1,fp);

            node->cnt[i] = 0;
        }

    }
     
    return 0;

    
}



void dns_lcllog_reqcnt()
{
    int i = 0;

    FILE *fp = fopen(g_reqcnt_path[0].path,"a+");
    if (!fp)
    {
        return ;
    }

    for(i = 0; i < g_pnum; i++)
    {
        h_hash_walk(g_reqcnt_loger[i].count_table, fp, reqcnt_log_hash_walk);
    }

    fclose(fp);
    
}



