//	$Id: fio_map32.h 2013-05-23 likunxiang$
//
#ifndef __FIO_MYMAP32_H__
#define __FIO_MYMAP32_H__

#include <stdbool.h>
#include "fio_list.h"

#define MAX_NAME 64
#define MAX_OBJ_NAME 100
#define MAX_NODE_SIZE 3000000

//24 bytes
struct fio_node_t
{
    struct fio_list_head link;
    struct fio_list_head hash;
    unsigned long key;
    void *val;
};

struct fio_map_t
{
    unsigned long num;
    unsigned long hash_size; 
    unsigned long max_size;
    int         pow2;
    char        obj_name[MAX_OBJ_NAME];
    int         member_size;

    struct fio_list_head*  hash_table;
    struct fio_list_head   link;
    struct fio_list_head   free_list;
    struct fio_list_head   *iterator;
    struct fio_node_t*       mem;
};

struct fio_map_t * fio_map_create(int member_size);
void fio_node_destroy(struct fio_node_t *n);
void fio_map_init( struct fio_map_t *_m, unsigned long _hash_size, unsigned long _max_size, const char* _obj_name);
bool fio_map_set_cpy(struct fio_map_t *_m, unsigned long _key, void *_val);
bool fio_map_set_ptr(struct fio_map_t *_m, unsigned long _key, void *_val);
bool fio_map_del(struct fio_map_t *_m, unsigned long _key );
inline bool fio_map_exist(struct fio_map_t *_m, unsigned long _key);
inline bool fio_map_find_cpy(struct fio_map_t *_m, unsigned long _key, void *_val);
inline bool fio_map_find_ptr(struct fio_map_t *_m, unsigned long _key, unsigned long *_val);
void fio_map_destroy(struct fio_map_t *_m);
void fio_map_clean(struct fio_map_t *_m);
struct fio_node_t* fio_map_first(struct fio_map_t *_m);
struct fio_node_t* fio_map_next(struct fio_map_t *_m, struct fio_node_t *_pre);
inline bool fio_map_isempty(struct fio_map_t *_m);
inline unsigned long fio_map_count(struct fio_map_t *_m);
inline struct fio_node_t * fio_map_iterator_put(struct fio_map_t *_m);

#endif

