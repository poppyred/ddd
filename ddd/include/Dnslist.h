//defination of hash table

#ifndef _DNSLIST_H
#define _DNSLIST_H

#include "Utils.h"
#include <netinet/in.h>
#include <ctype.h>
#include <netinet/in.h>

#define	USE_MASK_HALF	1	//’€∞Î≤È’“

//rfc 2817
#define MAX_TTL (7 * 86400)
//no rfc
#define MIN_TTL (10)
#define MAX_LINE_SIZE 	102400
#define MAX_MARK_SIZE	10240
#define MAX_ACTION_SIZE	4

#define MAX_MASK_COUNT	10000000

////////////////////////memory chunk/////////////////////////
struct msgcache
{
 int head,tail;
 uint size;
 pthread_mutex_t lock; //protect head and tail
 uchar data[0];
};


//struct msgcache* init_msgcache(int n);
//void free_msgcache(struct msgcache*);
/////////////////////////////////////////////////////////////


enum
{
 MAX_MSG_SEG = 15,
 MAX_MSG_SIZE = 1500,
};


//used by memory hash and disk db
struct mvalue
{
 uint16_t len;
 uint16_t num;
 uint32_t ttl;
 uint32_t hits;
 uint16_t seg; //when there is no memory segment, seg == 0
 //uint16_t off.
 //something...
};


///////////////////////memory hash///////////////////////////
typedef hashval_t (hashfunc) (void *data);
typedef int (comparefunc) (void*,void*);
typedef int (delkeyfunc) (void*);
typedef int (delvalfunc) (void*);


//we can hold at least HASH_TABLE_SIZE * MULTI_HASH elements
//slot size

struct hentry
{
 void *val;
 struct hentry *next;
 uchar key[0];
};


struct hdata
{
 struct hentry *list;
 pthread_rwlock_t lock;
};


struct htable
{
 pthread_mutex_t lock;//protect now
 struct hdata *table;
 uchar *edata;
 uint size,mask,now;
 hashfunc *h;
 comparefunc *c;
};

struct view_link
{
	int id;
	char * view_ip;
	char * view_action_str;
	struct sockaddr_in view_action[MAX_ACTION_SIZE];
	int action_size;
	struct view_link * next;
	pthread_rwlock_t rwlock;
};

struct domain_ip_send
{
	char ipaddr[MAX_IPS_SIZE];
	char msg[4][1024];
	unsigned int msg_len[4];
	unsigned int msg_size;
	unsigned int rand;
	pthread_rwlock_t rwlock;
};

struct domain_ip_temp
{
	char ipaddr[MAX_IPS_SIZE];
	char ip[4][16];
	unsigned int ip_len[4];
	unsigned int ip_size;
	unsigned int rand;
	pthread_rwlock_t rwlock;
};

struct htable* htable_create(hashfunc *h,comparefunc *c,int size);
int htable_insert(struct htable*,char*,void*,int);
int htable_set(struct htable *ht,char *key,void *val);
struct hentry* htable_delete(struct htable *ht,char *key);
void* htable_find(struct htable *ht,char *key,int len);
int htable_reset(struct htable*ht);
uint get_pre_mem_hash(void*);
int htable_for_each_do(struct htable *ht, struct in_addr ip, int(func)(struct hentry *,struct in_addr));
int htable_for_each_view(struct htable *ht,int(func)(struct hentry *));
int htable_cmpr_view(struct htable *ht, struct in_addr ip);
int htable_free(struct htable *ht);
hashval_t default_char_hash_function(void *argv,int len,int size);

/*
int view_file_to_struct(struct view_update*result,int size,char * filename);
struct view_link * view_link_init();
int view_link_add(struct view_link * vl ,struct view_update * vu);
int view_link_del(struct view_link * vl ,struct view_update * vu);
int view_link_update(struct view_update * vu);
int view_link_check_ip(struct view_link * vl,struct in_addr ip);
int view_link_info(struct view_link * vl);
int view_display(struct view_link * vl,char * view_output);
int view_count(struct view_link * vl);
*/

//*********************mask
/*
int getMask(int marknum,struct in_addr* ipaddr);
int mask_link_init();
int mask_link_set(struct view_update * vu);
int mask_link_unset(struct view_update * vu);
int mask_link_check_ip(struct in_addr ip);
int mask_print(struct hentry * ht);
int mask_display(char * mssk_output);
int getMaskNum(char*markstr);
int find_min_mask_num(struct hentry * ht);
*/

/*
int domain_init(struct htable**hl,int id);
int domain_del(struct htable**hl,struct view_update*vu);
int domain_add(struct htable**hl,struct view_update*vu);
int domain_update(struct htable**hl,struct view_update*vu);
int  domain_find(struct htable**hl,int id,char * domain,int len,char*msg,int max_size);
int domain_display(struct htable**hl);
int domain_count(struct htable**hl);

int make_dns_answers(struct domain_ip_send * dis,struct view_update*vu);
int make_dns_pre_answers(struct domain_ip_temp * dis,struct view_update*vu);
*/
#endif
