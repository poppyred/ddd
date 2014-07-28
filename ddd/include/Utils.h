//defination of some useful tool function.
#ifndef _UTILS_H
#define _UTILS_H

//standard c headers
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h> //for uint32_t

//unix system headers
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef uint32_t hashval_t;

#define MAX_IPS_SIZE 1024
#define MAX_IP_SIZE 16
#define MAX_DOMAIN_SIZE 256

#define MAX_VIEW_SIZE	(100)
#define HASH_TABLE_SIZE  (100)
#define MULTI_HASH (1)
#define MAX_RECORD_SIZE (100000)
#define MAX_MASK_SIZE	(102400)

#define CLIENT_DOMAIN_CHAR_SIZE	256
#define	CLIENT_DOMAIN_CACHE_NUM	1024

#define CLIENT_DOMAIN_DIRECTORY	"/var/log/dns_client_domain"
#define	CLIENT_DOMAIN_MAX_FILE_SIZE	6000000

enum utils_numberic
{
	DEBUG_TIMES = 500,
};


struct list_node
{
	void *data;
	struct list_node *next;
};


//list header
struct list
{
	pthread_mutex_t lock;
	struct list_node *head;
};


struct ttlnode
{
	uint exp; //expired time
	ushort dlen; //data len
	char *data; //
};

struct view_update
{
	int type;	//0:del|1:add|2:update
	int id;
	char domain[MAX_DOMAIN_SIZE];
	char ip[MAX_IPS_SIZE];
};

struct view_update_link
{
	struct view_update vu;
	struct view_update_link * next;
};

int trig_signals(int);
void drop_privilege(char*);

char* get_str(char *str,int len);
void put_str(char*);

int dict_comp_uint_equ(void *a,void *b);
int dict_comp_str_equ(void *a,void *b);
int rbt_comp_uint_gt(void *v1,void *v2,void *argv);
int rbt_comp_ttl_gt(void *v1,void *v2,void *argv);

void dns_error(int,char*);
int dbg(const char *format, ...);
void print_hex(char *val,int n);

int str_to_uchar4(const char *addr,char *val);
int str_to_uchar6(char *addr,char *val);
int to_uppercase(char *buf,int n);
int to_lowercase(char *buf,int n);
int fix_tail(char *domain);

int empty_function(int);
void insert_mem_bar(void);
int test_lock(pthread_mutex_t *lock);

int set_bit(ushort*,int);
int clr_bit(ushort*,int);
int tst_bit(const ushort,int);


int get_random_data(char*,int);

int get_time_usage(struct timeval *tv,int isbegin);
int is_uppercase(int c);
int is_lowercase(int c);

hashval_t uint_hash_function(void *ptr);
hashval_t nocase_char_hash_function(void *argv);

int slog(char *msg,int fd,pthread_mutex_t *lock);

int getTime(char*time);
int setTime();
int getIps(char* str,char ** result,int max_size);

int file_log(char*filenaem,char*content);
int file_log_all(FILE*file,char*content);
int efly_log(char*format);
//int resource_log();
int domain_to_q_name(char*domain,int len,char*qname);

void arr_free(int num, char ** marks);

void printftime(char*head);

int set_non_block(int sockfd);

int get_file_line(char * file);

#endif
