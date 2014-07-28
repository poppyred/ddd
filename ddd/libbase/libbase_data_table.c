/**
 * 数据表的实现
 * 提供一个存放记录集合的容器
 * 并且提供对多个域的快速索引的能力
 *
 * 基本实现思路是动态添加多张hash表索引数据
 *
 *
 *
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#define NDEBUG
#include <assert.h>

#include "libbase_data_table.h"


/* 内部调试接口 */
#ifdef HH_DEBUG
#  ifndef ERR_OUT_OF_MEMORY
#    define ERR_OUT_OF_MEMORY() ERR_OUT("Out of Memory!\n")
#  endif

#  ifndef ERR_OUT
#    define ERR_OUT(...) \
        do { \
            fprintf(stderr, "file:[%s] line:[%d] fun:[%s] ", \
                    __FILE__, __LINE__, __FUNCTION__); \
            fprintf(stderr, __VA_ARGS__); \
        } while(0)
#  endif
#  ifndef INFO_OUT
#    define INFO_OUT(...) printf(__VA_ARGS__)
#  endif
#else
#  ifndef ERR_OUT_OF_MEMORY
#    define ERR_OUT_OF_MEMORY()
#  endif
#  ifndef ERR_OUT
#    define ERR_OUT(...)
#  endif
#  ifndef INFO_OUT
#    define INFO_OUT(...)
#  endif
#endif

#ifndef POWEROF2
/* true if x is a power of 2 */
#define POWEROF2(x) ((((x)-1) & (x)) == 0)
#endif

/* 取得最靠近x并且小于x，的2的N次方 */
static
unsigned int get_nearly_power_of2_lt(unsigned int x) {
    unsigned int i = 1;
    while (!POWEROF2(x)) {
        x = (x >> i) << i;
        i++;
    }
    return x;
}

/* 记录结构，所有的hash表，都指向同一个这个结构，一个记录只有一份 */
struct record_st {
    void *data;

    unsigned int hval[DT_MAX_INDEX_FIELD];
};

/* hash表节点，每个记录，每张hash表都有一个的节点结构 */
struct hash_node_st {
    struct hash_node_st *next;

    struct record_st *record;
};

/* 搜索域管理结构 */
struct search_field_st {
    struct hash_node_st **slot;
    unsigned int (*hash)(const void *key, int klen);

    unsigned int size;
    h_data_table_get_field_cb_t get_field;
};

/* 数据表 */
struct h_data_table_st {
    unsigned int record_max; /* 最大记录数，这里也是哈希槽数 */

    unsigned int record_size; /* 每个用户记录的大小 */
    unsigned int record_num; /* 已经添加的记录数 */

    unsigned int slot_num;
    unsigned int slot_mask; /* slot_num - 1 */

    h_data_table_free_cb_t free_cb;

    void *(*malloc)(size_t size);
    void (*free)(void *);
    void *(*calloc)(size_t nmemb, size_t size);

    struct search_field_st *field[DT_MAX_INDEX_FIELD];
};

/* 默认time33 hash函数 */
static unsigned int hash_default_time33(const void *key, int klen)
{
    unsigned int h = 5318;
    const unsigned char *p = (const unsigned char *)key;

    while (klen > 0) {
        h = h * 33 + (*p);
        p++;
        klen--;
    }
    return h;
}

static
void dummy_free(void *ptr)
{
    (void)ptr;
}

h_data_table_t h_data_table_create(h_data_table_free_cb_t free_cb,
        unsigned int record_size, unsigned int max_record,
        h_dt_memory_allocer_st *allocer)
{
    h_data_table_t table;
    void *(*my_calloc)(size_t nmemb, size_t size);

    if (!max_record)
        return NULL;

    if (allocer == NULL)
        my_calloc = calloc;
    else
        my_calloc = allocer->calloc;

    if ((table = my_calloc(1, sizeof(struct h_data_table_st))) == NULL) {
        ERR_OUT_OF_MEMORY();
        return NULL;
    }
    table->record_max = max_record;
    table->slot_num = get_nearly_power_of2_lt(max_record);
    table->slot_mask = table->slot_num - 1;
    table->record_num = 0;
    table->record_size = record_size;
    table->free_cb = free_cb ? free_cb : dummy_free;

    if (allocer == NULL) {
        table->malloc = malloc;
        table->free = free;
        table->calloc = calloc;
    } else {
        table->malloc = allocer->malloc;
        table->free = allocer->free;
        table->calloc = allocer->calloc;
    }

    return table;
}


int h_data_table_add_index(h_data_table_t table, unsigned field_id,
        unsigned field_size, h_data_table_get_field_cb_t get_field)
{
    struct search_field_st *field;
    if (!table || field_id >= DT_MAX_INDEX_FIELD || !get_field)
        return -DT_ERR_INPUT;

    if (table->record_num != 0)
        return -DT_ERR_LOGIC;

    if (table->field[field_id]) {
        return -DT_ERR_LOGIC;
    }

    if ((field = table->calloc(1, sizeof(struct search_field_st))) == NULL) {
        ERR_OUT_OF_MEMORY();
        return -DT_ERR_MALLOC;
    }
    field->size = field_size;
    field->get_field = get_field;

    /* 到底要不要让用户可以提供自己的hash函数呢，
     * 暂时接口简洁点好 */
    field->hash = hash_default_time33;

    if ((field->slot = table->calloc(1, table->slot_num
                    * sizeof(struct hash_node_st *))) == NULL) {
        ERR_OUT_OF_MEMORY();
        table->free(field);
        return -DT_ERR_MALLOC;
    }

    table->field[field_id] = field;

    return 0;
}

/* 检测是否有添加的记录中有没有搜索域和现有记录的搜索域冲突 */
#if 0
static
int is_field_conflict(wns_data_table_t table, void *record)
{
    assert(table);
    int i;
    for (i = 0; i < DT_MAX_INDEX_FIELD; i++) {
        if (table->field[i] && (wns_data_table_search(table, i,
                    table->field[i]->get_field(record)) != NULL))
            return 1;
    }
    return 0;
}
#endif

static
int __record_insert(h_data_table_t table, void *record,
        unsigned int *in_hval /* 如果给出这个参数，直接用已经计算过的hash */ )
{
    struct search_field_st *field;
    struct record_st *r;
    struct hash_node_st *node[DT_MAX_INDEX_FIELD];
    int i;
    unsigned hval;
    unsigned idx;

    if (!table || !record)
        return -DT_ERR_INPUT;

    if (table->record_num == table->record_max)
        return -DT_ERR_FULL;

#if 0  /* 不做冲突检测 */
    if (is_field_conflict(table, record))
        return -DT_ERR_EXITE;
#endif

    if ((r = table->calloc(1, sizeof(struct record_st))) == NULL) {
        ERR_OUT_OF_MEMORY();
        return -DT_ERR_MALLOC;
    }
    bzero(node, sizeof(node));
    /* 先预分配内存，防止在插入过程中，分配不到内存，不好做错误处理 */
    for (i = 0; i < DT_MAX_INDEX_FIELD; i++) {
        if ((node[i] = table->calloc(1, sizeof(struct hash_node_st))) == NULL) {
            ERR_OUT_OF_MEMORY();
            goto err;
        }
    }
    r->data = record;
    /* 所有的hash表，对域建立索引 */
    for (i = 0; i < DT_MAX_INDEX_FIELD; i++) {
        field = table->field[i];
        if (!field) {
            table->free(node[i]);
            node[i] = NULL;
            continue;
        }
        if (in_hval)
            hval = in_hval[i];
        else
            hval = field->hash(field->get_field(record), field->size);

        r->hval[i] = hval;

        node[i]->record = r;
        node[i]->next = NULL;

        idx = hval & table->slot_mask;

        node[i]->next = field->slot[idx];
        field->slot[idx] = node[i];
    }
    table->record_num++;
    return 0;
err:
    for (i = 0; i < DT_MAX_INDEX_FIELD; i++)
        table->free(node[i]);

    table->free(r);
    return -DT_ERR_MALLOC;
}

int h_data_table_insert(h_data_table_t table, void *record)
{
    return __record_insert(table, record, NULL);
}

void* h_data_table_search(h_data_table_t table,
        unsigned int field_id, void *key)
{
    struct search_field_st *field;
    unsigned hval;
    unsigned idx;
    struct hash_node_st *p;

    if (!table ||field_id >= DT_MAX_INDEX_FIELD || !table->field[field_id])
        return NULL;

    field = table->field[field_id];

    hval = field->hash(key, field->size);

    idx = hval & table->slot_mask;
    p = field->slot[idx];
    while (p) {
        if (memcmp(field->get_field(p->record->data), key, field->size) == 0)
            return p->record->data;
        p = p->next;
    }
    return NULL;
}

int __del_by_field(h_data_table_t table, unsigned field_id,
        void *key, int free_flag)
{
    struct search_field_st *field;
    unsigned hval, idx;
    struct hash_node_st *p;
    struct hash_node_st *last;
    struct record_st *record;
    int i;
    if (!table ||field_id >= DT_MAX_INDEX_FIELD || !table->field[field_id])
        return -DT_ERR_INPUT;

    field = table->field[field_id];

    hval = field->hash(key, field->size);

    idx = hval & table->slot_mask;
    p = field->slot[idx];
    /* 先找到记录的地址 */
    while (p) {
        if (memcmp(field->get_field(p->record->data), key, field->size) == 0) {
            record = p->record;
            break;
        }
        p = p->next;
    }
    if (p == NULL)
        return -DT_ERR_NO_EXITE;
    /* 然后按照记录里面保存的hash，对所有hash表的节点进行删除 */
    for (i = 0; i < DT_MAX_INDEX_FIELD; i++) {
        field = table->field[i];
        if (!field)
            continue;
        hval = record->hval[i];
        idx = hval & table->slot_mask;
        p = field->slot[idx];
        last = NULL;
        while(p) {
            if (p->record == record) {
                if (last)
                    last->next = p->next;
                else
                    field->slot[idx] = p->next;

                table->free(p);
                break;
            }
            last = p;
            p = p->next;
        }
    }
    if (free_flag)
        table->free_cb(record->data);
    table->free(record);
    table->record_num--;
    return 0;
}

int h_data_table_remove(h_data_table_t table, void* record)
{
    int i;

    if (!table || !record)
        return -DT_ERR_INPUT;

    for (i = 0; i < DT_MAX_INDEX_FIELD; i++) {
        if (table->field[i])
            break;
    }
    if (i == DT_MAX_INDEX_FIELD)
        return -DT_ERR_LOGIC;

    return __del_by_field(table, i,
            table->field[i]->get_field(record), 0);
}

int h_data_table_delete(h_data_table_t table, void *record)
{
    int ret;
    if ((ret = h_data_table_remove(table, record)) == 0)
        table->free_cb(record);

    return ret;
}

int
h_data_table_del_by_field(h_data_table_t table, unsigned int field_id,
        void *key)
{
    return __del_by_field(table, field_id, key, 1 /* free record */);
}
int
h_data_table_rmv_by_field(h_data_table_t table, unsigned int field_id,
        void *key)
{
    return __del_by_field(table, field_id, key, 0 /* not free record */);
}

/* 清除表中所有的记录，但是表和表的配置还在 */
static
void clean_data_table(h_data_table_t table)
{
    struct search_field_st *field;
    struct hash_node_st *node;
    int i, j, first_time = 1;

    if (!table)
        return;

    for (i = 0; i < DT_MAX_INDEX_FIELD; i++) {
        field = table->field[i];
        if (!field)
            continue;
        for (j = 0; j < table->slot_num; j++) {
            while(field->slot[j]) {
                node = field->slot[j];
                field->slot[j] = field->slot[j]->next;

                if (first_time) {
                    /* 只有第一张hash表需要正在释放里面的数据，
                     * 其他的hash表只要释放管理节点 */
                    table->free_cb(node->record->data);
                    table->free(node->record);
                }
                table->free(node);
            }
        }
        first_time = 0;
    }
}

void h_data_table_destory(h_data_table_t table)
{
    struct search_field_st *field;
    int i;

    if (!table)
        return;

    clean_data_table(table);

    for (i = 0; i < DT_MAX_INDEX_FIELD; i++) {
        field = table->field[i];
        if (!field)
            continue;
        table->free(field->slot);
        table->free(field);
    }
    table->free(table); /* it's will be OK to free self */
}

/* 找到数据表的第一个记录，默认以第一张hash表顺序为标准
 * 这套内部函数，在操作期间有其他插入删除操作会带来不确定的后果 */
static struct hash_node_st *__walk_get_first(h_data_table_t table)
{
    struct search_field_st *field;
    int i;
    assert(table);

    field = table->field[0]; /* 数据表用户必须保证0号域有效 */
    if (!field)
        return NULL;
    for (i = 0; i < table->slot_num; i++) {
        if (!field->slot[i])
            continue;
        return field->slot[i];
    }
    return NULL;
}
/* 当node == NULL，取第一个记录 */
static struct hash_node_st *
__walk_get_next(h_data_table_t table, struct hash_node_st *node)
{
    struct search_field_st *field;
    int slot;
    int i;

    if (node == NULL)
        return __walk_get_first(table);

    field = table->field[0]; /* 数据表用户必须保证0号域有效 */

    if (node->next != NULL)
        return node->next;

    slot = node->record->hval[0] & table->slot_mask;
    for (i = slot + 1; i < table->slot_num; i++) {
        if (!field->slot[i])
            continue;
        return field->slot[i];
    }
    return NULL;
}

int h_data_table_to_file(h_data_table_t table, const char *path)
{
    FILE *fp;
    struct hash_node_st *node = NULL;
    struct record_st *record;
    //const unsigned int size = DT_MAX_INDEX_FIELD * sizeof(record->hval[0]);

    if (!path || !table)
        return -DT_ERR_INPUT;

    if ((fp = fopen(path, "w")) == NULL)
        return -DT_ERR_FILE;

    while ((node = __walk_get_next(table, node))) {
        record = node->record;
        //if (fwrite(record->hval, size, 1, fp) != 1)
        //    goto err;
        if (fwrite(record->data, table->record_size, 1, fp) != 1)
            goto err;
    }
    fclose(fp);

    return 0;
err:
    fclose(fp);
    return -DT_ERR_FILE;
}

int h_data_table_from_file(h_data_table_t table, const char *path)
{
    FILE *fp;
    void *data;
    //unsigned int hval[DT_MAX_INDEX_FIELD];

    if (!path || !table)
        return -DT_ERR_INPUT;

    if ((fp = fopen(path, "r")) == NULL)
        return -DT_ERR_FILE;

    while (1) {
        if ((data = table->calloc(1, table->record_size)) == NULL) {
            ERR_OUT_OF_MEMORY();
            goto err;
        }

/*
        if (fread(hval, sizeof(hval), 1, fp) != 1) {
            if (feof(fp))
                break;
            ERR_OUT("fread hval err\n");
            goto err2;
        }
*/

        if (fread(data, table->record_size, 1, fp) != 1) {
            if (feof(fp))
                break;
            ERR_OUT("fread data err\n");
            goto err2;
        }

        //__record_insert(table, data, hval);
        __record_insert(table, data, NULL);
    }
    table->free(data);
    fclose(fp);

    return 0;
err2:
    table->free(data);
err:
    clean_data_table(table);
    fclose(fp);
    return -DT_ERR_FILE;
}

int h_data_table_walk(h_data_table_t table,
        h_data_table_walk_cb_t walk_cb, void *data)
{
    struct hash_node_st *node;
    struct hash_node_st *node_next;
    int ret;

    if (!table || !walk_cb)
        return -DT_ERR_INPUT;

    /* 为了能在遍历的时候删除当前节点，我们必须先找好下一个节点 */
    node = __walk_get_first(table);
    while (node) {
        node_next = __walk_get_next(table, node);
        if ((ret = walk_cb(node->record->data, data)) != 0)
            return ret;
        node = node_next;
    }
    return 0;
}


int h_data_table_unit_walk(h_data_table_t table, unsigned int unit_id,
        h_data_table_walk_cb_t walk_cb, void *data)
{
    struct search_field_st *field;
    struct hash_node_st *node, *node_tmp;
    int ret;
    if (!table)
        return -DT_ERR_INPUT;

    field = table->field[0]; /* 数据表用户必须保证0号域有效 */
    if (!field)
        return -DT_ERR_INPUT;

    if (unit_id >= table->slot_num)
        return -DT_ERR_INPUT;

    node = field->slot[unit_id];

    while (node != NULL) {
        node_tmp = node->next;
        if ((ret = walk_cb(node->record->data, data)) != 0)
            return ret;
        node = node_tmp;
    }
    return 0;
}

/* @breif 返回数据表的遍历单元数 */
unsigned int h_data_table_get_foreach_unit_num(h_data_table_t table)
{
    return table->slot_num;
}

unsigned int h_data_table_get_record_num(h_data_table_t table)
{
    return table->record_num;
}

#ifdef HH_DEBUG
void h_data_table_state_show(h_data_table_t table)
{
    int i;
    int count = 0;
    printf("record_max: %u\n", table->record_max);
    printf("record_size: %u\n", table->record_size);
    printf("record_num: %u\n", table->record_num);
    printf("slot_num: %u\n", table->slot_num);
    printf("slot_mask: %#x\n", table->slot_mask);

    for (i = 0; i < DT_MAX_INDEX_FIELD; i++) {
        if (table->field[i])
            count++;
    }
    printf("field index num:%d\n", count);
}
#endif /* HH_DEBUG */
