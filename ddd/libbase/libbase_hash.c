/*=============================================================================
* @brief                                                                      *
* 哈希表(哈希算法为time33)                                                    *
*                                                                             *
*                                                                             *
*                                         @作者:hyb      @date 2013/04/10     *
==============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libbase_hash.h"
#include "libbase_memory.h"
#include "libbase_error.h"
#include "libbase_comm.h"
#include "libbase_strutils.h"

struct hash_node_st {
    int klen;                   /* key and len */

    unsigned int __hval;        /* hash value of this node, for rehashing */
    void *val;                  /* value */

    struct hash_node_st *next;
    char key[0];
};

/**
 * 哈希表结构
 */
struct h_hash_st {
    struct hash_node_st **slots;
    unsigned int nslot;         /* number of bucket */
    unsigned int max_element;   /* max element allowed for next rehashing */
    unsigned int min_element;   /* min element allowed for next rehashing */

//  int __fixed;       /* 哈希桶的数量是否固定 0为动态增长，非0为固定大小 */

    volatile unsigned int nelement;      /* hash entry count */

    hash_data_free_func_t hdel;
    hash_key_func_t hkey;
};

/**
 * 哈希表部分遍历的中间状态结构
 */
struct h_hcookie_st {
    float percent;          /* 百分比 */
    unsigned int iter;      /* 当前状态 */
};

/* rehash, if max_node reached */
static const struct hash_spec_st {
    unsigned int nslot;
    unsigned int min_node;
    unsigned int max_node;
} hash_specs[] = {
    { (1<<3), 0, (1<<5) },
    { (1<<5), (1<<4), (1<<7) },
    { (1<<7), (1<<6), (1<<9) },
    { (1<<9), (1<<8), (1<<11) },
    { (1<<11), (1<<10), (1<<13) },
    { (1<<13), (1<<12), MAX_SIGNED_32INT }
};

/* the default key function, famous time33 alg */
#if 0
static unsigned int hash_default_key_time33(const void *key, int klen)
{
    unsigned int h = 5381;
    const unsigned char *p = (const unsigned char *)key;

    while (klen > 0) {
        h = h * 33 + (*p);
        p++;
        klen--;
    }
    return h;
}
#endif

/*add after*/
static unsigned int str2id( const void* pstr, int _len)
{
    if( _len < 1 )   
         return 0;    
 
    unsigned long hash = 5381;
    
    unsigned long c; 
    char *_str = (char*)pstr;
 
    while ((c = *_str++) && _len--)
         hash = ((hash << 5) + hash) + c;

     return hash;
}


/* create an hash, with data function and key generate function, all can be NULL */
h_hash_st *h_hash_create(hash_data_free_func_t del, hash_key_func_t keyf,
        unsigned int slotnum)
{
    h_hash_st *h;

    if (!POWEROF2(slotnum))
        return NULL;

    if ((h = h_calloc(1, sizeof(h_hash_st))) == NULL)
        return NULL;

    if (slotnum == 0) { /* 自动增长 */
        h->nslot = hash_specs[0].nslot;
        h->max_element = hash_specs[0].max_node;
        h->min_element = hash_specs[0].min_node;
    } else { /* 固定大小 */
        h->nslot = slotnum;
        h->max_element = (MAX_UNSIGNED_32INT);
        h->min_element = 0;
    }

    h->slots = h_calloc(1, h->nslot * sizeof(struct hash_node_st *));
    if (h->slots == NULL) {
        h_free(h);
        return NULL;
    }

    h->hdel = del;

    if (keyf)
        h->hkey = keyf;
    else
        h->hkey = str2id;

    return h;
}

unsigned int h_hash_count(h_hash_st *ha)
{
    /*add by hyb 20140217*/
    if (ha->nelement < 0)
    {
        ha->nelement = 0;
    }
    return ha->nelement;
}

/* extern/shrink the slots of hash and rearrange all nodes */
/* flag小于0表示收缩，flag大于0扩张，等于0什么都不干 */
static void hash_rehash(h_hash_st *h, int flag)
{
    unsigned int new_nslot = 0, new_max = 0, new_min = 0;
    struct hash_node_st **new_slots;
    unsigned int i;
    if (flag == 0)
        return;

    if (flag > 0) {
        /* finding the next slot size and max/min element */
        for (i = 0;
                i < sizeof(hash_specs)/sizeof(struct hash_spec_st) - 1;
                ++i) {
            if (hash_specs[i].nslot == h->nslot) {
                new_nslot = hash_specs[i+1].nslot;
                new_max = hash_specs[i+1].max_node;
                new_min = hash_specs[i+1].min_node;
                break;
            }
        }
    } else {
        /* finding the prev slot size and max/min element */
        for (i = 1; i < sizeof(hash_specs)/sizeof(struct hash_spec_st); ++i) {
            if (hash_specs[i].nslot == h->nslot) {
                new_nslot = hash_specs[i-1].nslot;
                new_max = hash_specs[i-1].max_node;
                new_min = hash_specs[i-1].min_node;
                break;
            }
        }
    }

    if (!new_nslot) {
        fprintf(stderr, "[BUG] rehashing, can not get next_mask\n");
        return;
    }

    /* allocate new slots and move every node to new slots */
    new_slots = h_calloc(1, new_nslot * sizeof(struct hash_node_st *));
    if (new_slots == NULL) {
        fprintf(stderr, "[BUG] rehashing, Out of memory\n");
        return;
    }

    for (i = 0; i < h->nslot; ++i) {
        struct hash_node_st *p = h->slots[i];
        while (p) {
            struct hash_node_st *next = p->next;
            unsigned int newindex = p->__hval & (new_nslot - 1);

            p->next = new_slots[newindex];
            new_slots[newindex] = p;

            p = next;
        }
    }
    h_free(h->slots);
    h->slots = new_slots;
    h->nslot = new_nslot;
    h->max_element = new_max;
    h->min_element = new_min;
}

/* insert, if key is exist, an del function will be call on the old data, and replace with new data */
int h_hash_insert(h_hash_st *ht, const void *key, int len, void *val)
{
    unsigned int hval = ht->hkey(key, len);
    unsigned int idx = hval & (ht->nslot - 1);
    struct hash_node_st *tmp;
    struct hash_node_st *p = ht->slots[idx];

    /* lookup whether the node exist, replace then */
    while (p) {
        if (hval == p->__hval
                && p->klen == len && memcmp(p->key, key, len) == 0) {
            if ((void *)ht->hdel)
                ht->hdel(p->val);

            p->val = val;
            return 0;
        }
        p = p->next;
    }

    /* new node for insert */
    tmp = h_malloc(sizeof(struct hash_node_st) + len);
    if (!tmp)
        return -1;

    tmp->klen = len;
    memcpy_s(tmp->key, key, len);

    tmp->val = val;
    tmp->__hval = hval;

    tmp->next = ht->slots[idx];
    ht->slots[idx] = tmp;
    ht->nelement++;

    if (ht->nelement > ht->max_element)         /* rehash after newly node inserted */
        hash_rehash(ht, 1);

    return 0;
}

/* search the key, return at void **val return 0 if key founded */
int h_hash_search(h_hash_st *ht, const void *key, int len, void **val)
{
    unsigned int hval = ht->hkey(key, len);
    unsigned int idx = hval & (ht->nslot - 1);
    struct hash_node_st *p = ht->slots[idx];

    while(p) 
    {
        if ( p->__hval == hval && p->klen == len
        && memcmp(p->key, key, len) == 0) {
            if (val)
                *val = p->val;

            return 0;
        }
        p = p->next;
    }
    return -1;
}

/* delete the key of the hash, del function will be called on this element */
int h_hash_delete(h_hash_st *ht, const void *key, int len)
{
    unsigned int hval = ht->hkey(key, len);
    unsigned int idx = hval & (ht->nslot - 1);
    struct hash_node_st *p = ht->slots[idx];
    struct hash_node_st *last = NULL;

    while (p) {
        if (hval == p->__hval
                && p->klen == len && memcmp(p->key, key, len) == 0) {
            if (last)
                last->next = p->next;
            else
                ht->slots[idx] = p->next;

            ht->nelement--;

            if ((void *)ht->hdel)
                ht->hdel(p->val);

            h_free(p);

            if (ht->nelement < ht->min_element)         /* rehash after node delete */
                hash_rehash(ht, -1);

            return 0;
        }
        last = p;
        p = p->next;
    }
    return -1;
}

/* destroy every thing */
void h_hash_destroy(h_hash_st *ht)
{
    unsigned int i;
    struct hash_node_st *t;

    for(i = 0; i < ht->nslot; i++) {
        while(ht->slots[i]) {
            t = ht->slots[i];
            ht->slots[i] = ht->slots[i]->next;

            if ((void *)ht->hdel)
                ht->hdel(t->val);

            h_free(t);
        }
    }
    h_free(ht->slots);
    h_free(ht);
}

/* walk the hash with callback on every element */
int h_hash_walk(h_hash_st *ht, void *udata, hash_walk_func_t fn)
{
    struct hash_node_st *p;
    unsigned int i;
    int ret;

    if (!(void *)fn)
        return -1;

    for (i = 0; i < ht->nslot; ++i) {
        p = ht->slots[i];
        while (p) {
            struct hash_node_st *next = p->next;

            ret = fn(p->key, p->klen, p->val, udata);
            if (ret)
                return ret;

            p = next;
        }
    }
    return 0;
}

#define list_for_each_hash_val_safe(i, pos, table)          \
    for (i = 0, pos = table->slots[i]->val; \
            i < table->nslot;                    \
            i++, pos = table->slots[i]->val)



/* create a cookie struct for partial walk */
h_hcookie_st *h_hash_cookie_create(float percent)
{
    h_hcookie_st *c = h_calloc(1, sizeof(h_hcookie_st));
    if (c == NULL)
        return NULL;

    c->percent = percent > 1.0f ? 1.0f : percent;
    return c;
}

void h_hash_cookie_destroy(h_hcookie_st *cookie)
{
    h_free(cookie);
}

/* setup percent of a cookie */
void h_hash_cookie_set_percent(h_hcookie_st *cookie, float percent)
{
    cookie->percent = percent;
}

/* partially walk a hash */
void h_hash_walk_partial(h_hash_st *ht, h_hcookie_st *cookie,
        void *udata, hash_walk_func_t fn)
{
    struct hash_node_st *p;
    int towalk;

    if (!(void *)fn)
        return;

    towalk = (int)((float)ht->nslot * cookie->percent);
    if (towalk < 1) {
        towalk = 1;
    }

    while (towalk > 0) {
        if (cookie->iter >= ht->nslot) {
            cookie->iter = 0;
        }

        p = ht->slots[cookie->iter];
        while (p) {
            struct hash_node_st *next = p->next;
            fn(p->key, p->klen, p->val, udata);
            p = next;
        }

        cookie->iter++;
        towalk--;
    }
}

/* get slot info */
void h_hash_slotinfo(h_hash_st *ht, int *counts, int ncount)
{
    unsigned int i;
    for (i = 0; i < ht->nslot; ++i) {
        struct hash_node_st *p = ht->slots[i];
        int cnt = 0;

        while (p) {
            cnt++;
            p = p->next;
        }

        if (cnt >= ncount)
            continue;

        counts[cnt]++;
    }
}
