//	$Id: fio_mapstr.h 2013-05-23 likunxiang$
//
#ifndef __FIO_MYMAPSTR_H__
#define __FIO_MYMAPSTR_H__

#include <stdbool.h>
#include "fio_list.h"

#ifndef MAX_NAME
#define MAX_NAME 64
#endif
#ifndef MAX_OBJ_NAME
#define MAX_OBJ_NAME 100
#endif
#ifndef MAX_NODE_SIZE
#define MAX_NODE_SIZE 3000000
#endif

//88 bytes
struct fio_nodestr_t
{
    struct fio_list_head link;
    struct fio_list_head hash;
    char key[MAX_NAME];
    void *val;
    int len;
};

struct fio_mapstr_t
{
    unsigned long  num;
    unsigned long  hash_size; 
    unsigned long  max_size;
    int         pow2;
    char        obj_name[MAX_OBJ_NAME];
    int         member_size;

    struct fio_list_head  *hash_table;
    struct fio_list_head   link;
    struct fio_list_head   free_list;
    struct fio_list_head  *iterator;
    struct fio_nodestr_t  *mem;
};

struct fio_mapstr_t * fio_mapstr_create(int member_size);
void fio_mapstr_init(struct fio_mapstr_t *_m, unsigned long _hash_size, unsigned long _max_size, const char* _obj_name );
void fio_mapstr_clean(struct fio_mapstr_t *_m);
bool fio_mapstr_set_cpy(struct fio_mapstr_t *_m, const char *_key, void *_val);
void fio_mapstr_destroy(struct fio_mapstr_t *_m);
inline bool fio_mapstr_exist(struct fio_mapstr_t *_m, const char *_key);
#endif

