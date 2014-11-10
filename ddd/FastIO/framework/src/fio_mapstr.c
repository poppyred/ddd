//	$Id: fio_mapstr.c 2013-05-23 likunxiang$
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fio_mapstr.h"

static unsigned long fio_mapstr_str2id( const char* _str, unsigned long _len, unsigned long _hash_size )
{
    if( _len < 1 ) 
        return 0;

    unsigned long hash = 5381;
    unsigned long c;

    while ((c = *_str++) && _len--)
        hash = ((hash << 5) + hash) + c;

    return (hash & (_hash_size -1) );
}

struct fio_nodestr_t * fio_nodestr_create_array(unsigned long _count, int member_size)
{ 
    struct fio_nodestr_t *ns = calloc(_count, sizeof(struct fio_nodestr_t));
    int i;
    for (i = 0; i < _count; i++)
        ns[i].val = malloc(member_size);
    return ns;
}

struct fio_nodestr_t * fio_nodestr_create_nulldata(int member_size)
{ 
    struct fio_nodestr_t *n = malloc(sizeof(struct fio_nodestr_t));
    memset(n, 0, sizeof(struct fio_nodestr_t)); 
    n->val = NULL;
    return n;
}

struct fio_nodestr_t * fio_nodestr_create(int member_size)
{ 
    struct fio_nodestr_t *n = malloc(sizeof(struct fio_nodestr_t));
    memset(n, 0, sizeof(struct fio_nodestr_t)); 
    n->val = malloc(member_size);
    return n;
}

void fio_nodestr_destroy(struct fio_nodestr_t *n)
{
    if (n->val)
    {
        free(n->val);
        n->val = NULL;
    }
    free(n);
}

/*! member function of fio_mapstr_t */ 
struct fio_mapstr_t * fio_mapstr_create(int member_size)
{
    struct fio_mapstr_t *m = malloc(sizeof(struct fio_mapstr_t));
    memset(m, 0, sizeof(struct fio_mapstr_t));
    FIO_INIT_LIST_HEAD( &m->free_list );
    FIO_INIT_LIST_HEAD( &m->link );
    m->member_size = member_size;
    return m;
}

void fio_mapstr_init(struct fio_mapstr_t *_m, unsigned long _hash_size, unsigned long _max_size, const char* _obj_name )
{
    FIO_INIT_LIST_HEAD( &_m->link );

    unsigned long i = 0;
    for( i = 1; i < 32; ++i ) 
    {
        if( _hash_size <= (unsigned long)( 1 << i ) )
            break;
    }

    _m->num = 0;
    _m->pow2 = i;
    _m->hash_size = ( 1 << i );
    _m->hash_table = malloc(_m->hash_size * sizeof(struct fio_list_head));
    for(i = 0; i < _m->hash_size; ++i ) 
        FIO_INIT_LIST_HEAD( &_m->hash_table[i] );

    _m->mem = fio_nodestr_create_array(_max_size, _m->member_size);
    FIO_INIT_LIST_HEAD( &_m->free_list );
    for(i = 0; i < _max_size; ++i)
        fio_list_add_tail(&_m->mem[i].link, &_m->free_list);

    _m->max_size = _max_size;
    strncpy( _m->obj_name, _obj_name, MAX_OBJ_NAME );
    _m->iterator = &_m->link;
}

inline bool fio_mapstr_exist(struct fio_mapstr_t *_m, const char *_key)
{    
    int len;
    if (!_key || (len=strlen(_key)) < 1)
        return false;
    unsigned long index = fio_mapstr_str2id(_key, len, _m->hash_size);
    struct fio_list_head* pos;
    fio_list_for_each_safe(pos, &_m->hash_table[index])
    {
        struct fio_nodestr_t *node = fio_list_entry(pos, struct fio_nodestr_t, hash);
        if (node->len == len && !strncmp(node->key, _key, len))
            return true;
    } 
    return false;
}

inline bool fio_mapstr_find_cpy(struct fio_mapstr_t *_m, const char *_key, void *_val)
{
    int len;
    if (!_key || (len=strlen(_key)) < 1)
        return false;
    unsigned long index = fio_mapstr_str2id(_key, len, _m->hash_size);
    struct fio_list_head* pos;
    fio_list_for_each_safe(pos, &_m->hash_table[index]) 
    {
        struct fio_nodestr_t *node = fio_list_entry(pos, struct fio_nodestr_t, hash);
        if (node->len == len && !strncmp(node->key, _key, len))
        {
            memcpy(_val, node->val, _m->member_size);
            return true;
        }
    }
    return false;
}

inline bool fio_mapstr_find_ptr(struct fio_mapstr_t *_m, const char *_key, unsigned long *_val)
{
    int len;
    if (!_key || (len=strlen(_key)) < 1)
        return false;
    unsigned long index = fio_mapstr_str2id(_key, len, _m->hash_size);
    struct fio_list_head* pos;
    fio_list_for_each_safe( pos, &_m->hash_table[index] ) 
    {
        struct fio_nodestr_t *node = fio_list_entry(pos, struct fio_nodestr_t, hash);
        if (node->len == len && !strncmp(node->key, _key, len))
        {
            *_val = (unsigned long)node->val;
            return true;
        }
    } 
    return false;
}

bool fio_mapstr_set_cpy(struct fio_mapstr_t *_m, const char *_key, void *_val)
{
    int len;
    if (!_key || (len=strlen(_key)) < 1 || len >= MAX_NAME)
        return false;
    unsigned long index = fio_mapstr_str2id(_key, len, _m->hash_size);
    struct fio_nodestr_t *node = 0;
    struct fio_list_head* pos;
    fio_list_for_each( pos, &_m->hash_table[index] )
    {
        node = fio_list_entry( pos, struct fio_nodestr_t, hash );
        if (node->len == len && !strncmp(node->key, _key, len))
        {
            memcpy(node->val, _val, _m->member_size);
            return false;
        }
    } 

    if( fio_list_empty(&_m->free_list) ) 
        node = fio_nodestr_create(_m->member_size);
    else 
    {
        pos = _m->free_list.next;
        fio_list_del(pos);
        node = fio_list_entry( pos, struct fio_nodestr_t, link ); 
    }

    if( node ) 
    {
        strncpy( node->key, _key, len );
        node->key[len] = '\0';
        node->len = len;
        memcpy(node->val, _val, _m->member_size);
        fio_list_add( &node->hash, &_m->hash_table[index] );
        fio_list_add( &node->link, &_m->link );
        _m->num++;
    }
    return true;
}

bool fio_mapstr_set_ptr(struct fio_mapstr_t *_m, const char *_key, void *_val)
{
    int len;
    if (!_key || (len=strlen(_key)) < 1 || len >= MAX_NAME)
        return false;
    unsigned long index = fio_mapstr_str2id(_key, len, _m->hash_size);
    struct fio_nodestr_t *node = 0;
    struct fio_list_head* pos;
    fio_list_for_each( pos, &_m->hash_table[index] )
    {
        node = fio_list_entry( pos, struct fio_nodestr_t, hash );
        if (node->len == len && !strncmp(node->key, _key, len))
        {
            if (node->val)
            {
                free(node->val);
                node->val = NULL;
            }
            node->val = _val;
            return false;
        }
    } 

    if( fio_list_empty(&_m->free_list) ) 
        node = fio_nodestr_create_nulldata(_m->member_size);
    else 
    {
        pos = _m->free_list.next;
        fio_list_del(pos);
        node = fio_list_entry( pos, struct fio_nodestr_t, link ); 
    }

    if( node ) 
    {
        strncpy( node->key, _key, len );
        node->key[len] = '\0';
        node->len = len;
        if (node->val)
        {
            free(node->val);
            node->val = NULL;
        }
        node->val = _val;
        fio_list_add( &node->hash, &_m->hash_table[index] );
        fio_list_add( &node->link, &_m->link );
        _m->num++;
    }
    return true;
}

bool fio_mapstr_del(struct fio_mapstr_t *_m, const char *_key)
{
    int len;
    if (!_key || (len=strlen(_key)) < 1 || len >= MAX_NAME)
        return false;
    unsigned long index = fio_mapstr_str2id(_key, len, _m->hash_size);
    struct fio_list_head *pos;
    struct fio_nodestr_t *node;
    int n_index;
    fio_list_for_each_safe( pos, &_m->hash_table[index] )
    {
        node = fio_list_entry( pos, struct fio_nodestr_t, hash );
        if (node->len == len && !strncmp(node->key, _key, len))
        {
            if (_m->iterator == &node->link)
                _m->iterator = node->link.prev;
            fio_list_del( &node->hash );
            fio_list_del( &node->link );

            n_index = node - _m->mem;
            if(n_index >= 0 && (unsigned long)n_index < _m->max_size) 
                fio_list_add( &node->link, &_m->free_list );
            else 
                fio_nodestr_destroy(node);

            _m->num--;
            return true;
        }
    } 
    return false;
}

void fio_mapstr_clean(struct fio_mapstr_t *_m)
{
    int index = 0;
    struct fio_nodestr_t *node = NULL;
    struct fio_list_head *pos;
    while(!fio_list_empty(&_m->link)) {
        pos = _m->link.next;
        fio_list_del(pos);
        node = fio_list_entry(pos, struct fio_nodestr_t, link);
        fio_list_del(&node->hash);
        index = node - _m->mem;
        if(index >= 0 && (unsigned long)index < _m->max_size) {
            fio_list_add( &(node->link), &_m->free_list );
        } else {
            fio_nodestr_destroy(node);
        }
    }

    unsigned long i;
    for(i = 0; i < _m->hash_size; ++i) {
        FIO_INIT_LIST_HEAD(&_m->hash_table[i]);
    }

    _m->num = 0;
    FIO_INIT_LIST_HEAD(&_m->link);
    _m->iterator = &_m->link;
}

void fio_mapstr_destroy(struct fio_mapstr_t *_m)
{
	if (!_m)
		return;
    fio_mapstr_clean(_m);
    if( _m->hash_table ) 
    {
        free(_m->hash_table);
        _m->hash_table = NULL;
    }

    if( _m->mem ) 
    {
        int i = 0;
        for (i = 0; i < _m->max_size; i++)
        {
            if (_m->mem[i].val)
            {
                free(_m->mem[i].val);
                _m->mem[i].val = NULL;
            }
        }
        free(_m->mem);
        _m->mem = NULL;
    }
    _m->iterator = NULL;

    free(_m);
}

struct fio_nodestr_t * fio_mapstr_first(struct fio_mapstr_t *_m)
{
    if(!fio_list_empty( &_m->link ))
        return fio_list_entry( _m->link.next, struct fio_nodestr_t, link );
    return 0;
}

struct fio_nodestr_t * fio_mapstr_next(struct fio_mapstr_t *_m, struct fio_nodestr_t *_pre )
{
    struct fio_list_head* pos = _pre->link.next;
    if( pos != &_m->link )
        return fio_list_entry( pos, struct fio_nodestr_t, link );
    return 0;
}

inline bool fio_mapstr_isempty(struct fio_mapstr_t *_m)
{
    return fio_list_empty(&_m->link);
}

inline unsigned long fio_mapstr_count(struct fio_mapstr_t *_m)
{
    return _m->num;
}

inline struct fio_nodestr_t * fio_mapstr_iterator_put(struct fio_mapstr_t *_m)
{
    if (_m->num < 1)
        return NULL;

    _m->iterator = _m->iterator->next;
    if (_m->iterator == &_m->link)
        _m->iterator = _m->iterator->next;
    return (fio_list_entry(_m->iterator, struct fio_nodestr_t, link));
}

