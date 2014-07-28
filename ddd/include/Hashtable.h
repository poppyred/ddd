#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef __USE_ISOC99
#define inline
#endif

#define create_hashtable(hsize) \
         htable_create(lh_strhash, equal_str, hsize)

typedef unsigned int (keyfunc) (void *);
typedef int (comparefunc) (void*,void*);

unsigned int lh_strhash(void *src);
int equal_str(void *k1, void *k2);

struct hashentry
{
    void             *key;
    void             *data;
    struct hashentry *next;
};

struct hdata
{
    struct hashentry *entry;
    pthread_mutex_t lock;
};

struct hashtable
{
    keyfunc           *kf;
    comparefunc       *cf;
    unsigned int      hashsize;
    unsigned int      count;
    struct hdata      *hlist;
    pthread_mutex_t   lock;//protect now
};

struct hashtable* htable_create(keyfunc *kf, comparefunc *cf, int size);
void htable_free(struct hashtable *tab);
void htable_insert(void *key, void *data, struct hashtable *tab);
void htable_remove(void *key, struct hashtable *tab);
void* htable_value(void *key, struct hashtable *tab);
void htable_for_each_do(struct hashtable *tab, int (cb)(void *, void *));
int htable_count(struct hashtable *tab);

int cmpr_hash_view(struct hashtable *tab, struct in_addr ip);
int markChange(int marknum,struct in_addr *ipaddr);
int check_ip_view(struct in_addr ip,char* ipmark);
void arr_free(int num, char ** marks);
#endif
