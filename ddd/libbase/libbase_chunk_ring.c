#include <stdlib.h>
#include <assert.h>
#include <sched.h>

#include "libbase_chunk_ring.h"
#include "libbase_memory.h"
#include "libbase_strutils.h"
#include "libbase_comm.h"
//#include "libbase_log.h"

/**
 * 块数据环形缓冲区共享区结构
 */
struct h_chunk_ring_st {
    char        name[CHUNK_RING_NAMESIZE];  /**< 缓冲区名称 */

    wns_atomic32_t  writter;                    /**< 写者 */
    wns_atomic32_t  done_writter;               /**< 已完成的写者 */

    /* 生产者状态 */
    struct {
        uint32_t size;                      /**< 长度 */
        uint32_t mask;                      /**< 掩码 (size-1) */
        volatile uint32_t head;             /**< 生产者头部 */
        volatile uint32_t tail;             /**< 生产者尾部 */
    } prod __wns_arch_cache_aligned;                /**< 生产者 */

    /* 消费者状态 */
    struct {
        uint32_t size;                      /**< 长度 */
        uint32_t mask;                      /**< 掩码 (size-1) */
        volatile uint32_t head;             /**< 消费者头部 */
        volatile uint32_t tail;             /**< 消费者尾部 */
    } cons __wns_arch_cache_aligned;                /**< 消费者 */

    char ring[0];                           /**< 缓冲区 */
};


/* 在指定内存上创建环形缓冲区 */
h_chunk_ring_st *h_chunk_ring_create(const char *name, uint32_t size,
        uint32_t flags)
{
    h_chunk_ring_st *r;
    uint32_t ring_size;

    if (size == 0
            || !POWEROF2(size))
        return NULL;

    ring_size = size + sizeof(h_chunk_ring_st);

    r = (h_chunk_ring_st *)h_malloc(ring_size);
    if (r == NULL) {
        printf("Cannot alloc memory\n");
        return NULL;
    }

    memset(r, 0, sizeof(*r));
    sprintf_n(r->name, sizeof(r->name), "%s", name);

    // modified by adam - 2012-10-25 --------------------------------
    // 避免cpptest的嵌套赋值声明警告
//  r->prod.size = r->cons.size = size;
//  r->prod.mask = r->cons.mask = size - 1;
//  r->prod.head = r->cons.head = 0;
//  r->prod.tail = r->cons.tail = 0;
    r->prod.size = size;
    r->cons.size = size;
    r->prod.mask = size - 1;
    r->cons.mask = size - 1;
    r->prod.head = 0;
    r->cons.head = 0;
    r->prod.tail = 0;
    r->cons.tail = 0;
    // end of modified by adam --------------------------------------

    return r;
}

/* 多生产者入队 */
int32_t h_chunk_ring_enqueue(h_chunk_ring_st *r, const char *buffer,
        uint32_t n)
{
    uint32_t prod_head, prod_next;
    uint32_t cons_tail, free_entries;
    int32_t success;
    uint32_t mask = r->prod.mask;
    uint32_t entlen = n + sizeof(uint32_t);
    uint32_t pos;

    if (!buffer)
        return -ENOBUFS;

    /* 自动移动 prod.head */
    do {
        prod_head = r->prod.head;
        cons_tail = r->cons.tail;
        free_entries = (mask + cons_tail - prod_head);

        /* 检查是否有空间放入chunk */
        if (free_entries < entlen) {
            return -ENOBUFS;
        }

        prod_next = prod_head + entlen;
        success = wns_atomic32_cmpset(&r->prod.head, prod_head, prod_next);
    } while (success == 0);

    /* 写入长度 */
    pos = (prod_head & mask);
    if (r->prod.size - pos >= sizeof(uint32_t)) {
        *((uint32_t *)(r->ring + pos)) = n;
    } else {
        uint32_t l = r->prod.size - pos;
        memcpy_s(r->ring + pos, &n, l);
        memcpy_s(r->ring, ((char *)&n) + l, sizeof(uint32_t) - l);
    }

    /* 写入数据 */
    pos = ((prod_head + sizeof(uint32_t)) & mask);
    if (r->prod.size - pos >= n) {
        memcpy_s(r->ring + pos, buffer, n);
    } else {
        uint32_t l = r->prod.size - pos;
        memcpy_s(r->ring + pos, buffer, l);
        memcpy_s(r->ring, buffer + l, n - l);
    }

    wns_wmb();

    /* 等待其它入队者完成操作 */
    while (r->prod.tail != prod_head)
        wns_yield();

    r->prod.tail = prod_next;
    return 0;
}

// added by adam 2012-10-22 ------------------------------------------------
int32_t h_chunk_ring_peek(h_chunk_ring_st *r, char *buffer,
        uint32_t *chunk_size)
{
    uint32_t cons_head, prod_tail;
    uint32_t entries;
    // deleted by adam 2012-10-22 ------------------------------------------
//  uint32_t cons_next, entries;
//  int32_t success;
    // end of deleted by adam 2012-10-22 -----------------------------------
    uint32_t pos;
    uint32_t mask = r->prod.mask;
    uint32_t n;

    if (!buffer)
        return -ENOBUFS;

    /* 移动 cons.head */
    do {
        cons_head = r->cons.head;
        prod_tail = r->prod.tail;
        entries = (prod_tail - cons_head);

        /* 检查缓冲区中是否有数据 */
        if (entries < sizeof(uint32_t)) {
            return -ENOENT;
        }

        /* 读取chunk长度 */
        pos = (cons_head & mask);
        if (r->cons.size - pos >= sizeof(uint32_t)) {
            n = *((uint32_t *)(r->ring + pos));
        } else {
            uint32_t l = r->cons.size - pos;
            memcpy_s(&n, r->ring + pos, l);
            memcpy_s(((char *)&n) + l, r->ring, sizeof(uint32_t) - l);
        }
        if (*chunk_size < n)
            return -ENOBUFS;

    // modified by adam 2012-10-22 -----------------------------------------
//      cons_next = cons_head + n + sizeof(uint32_t);
//      success = wns_atomic32_cmpset(&r->cons.head, cons_head, cons_next);
//  } while (success == 0);
    } while (0);
    // end of modified by adam 2012-10-22 ----------------------------------

    wns_rmb();

    *chunk_size = n;

    /* 读取chunk数据 */
    pos = ((cons_head + sizeof(uint32_t)) & mask);
    if (r->cons.size - pos >= n) {
        memcpy_s(buffer, r->ring + pos, n);
    } else {
        uint32_t l = r->cons.size - pos;
        memcpy_s(buffer, r->ring + pos, l);
        memcpy_s(buffer + l, r->ring, n - l);
    }

    // deleted by adam 2012-10-22 ------------------------------------------
//  /* 等待其它出队者完成操作 */
//  while (r->cons.tail != cons_head)
//      wns_yield();
//
//  r->cons.tail = cons_next;
    // end of deleted by adam 2012-10-22 -----------------------------------
    return 0;
}
// end of added by adam 2012-10-22 -----------------------------------------

/* 多消费者出队 */
int32_t h_chunk_ring_dequeue(h_chunk_ring_st *r, char *buffer,
        uint32_t *chunk_size)
{
    uint32_t cons_head, prod_tail;
    uint32_t cons_next, entries;
    int32_t success;
    uint32_t pos;
    uint32_t mask = r->prod.mask;
    uint32_t n;

    if (!buffer)
        return -ENOBUFS;

    /* 移动 cons.head */
    do {
        cons_head = r->cons.head;
        prod_tail = r->prod.tail;
        entries = (prod_tail - cons_head);

        /* 检查缓冲区中是否有数据 */
        if (entries < sizeof(uint32_t)) {
            return -ENOENT;
        }

        /* 读取chunk长度 */
        pos = (cons_head & mask);
        if (r->cons.size - pos >= sizeof(uint32_t)) {
            n = *((uint32_t *)(r->ring + pos));
        } else {
            uint32_t l = r->cons.size - pos;
            memcpy_s(&n, r->ring + pos, l);
            memcpy_s(((char *)&n) + l, r->ring, sizeof(uint32_t) - l);
        }
        if (*chunk_size < n)
            return -ENOBUFS;

        cons_next = cons_head + n + sizeof(uint32_t);
        success = wns_atomic32_cmpset(&r->cons.head, cons_head, cons_next);
    } while (success == 0);

    wns_rmb();

    *chunk_size = n;

    /* 读取chunk数据 */
    pos = ((cons_head + sizeof(uint32_t)) & mask);
    if (r->cons.size - pos >= n) {
        memcpy_s(buffer, r->ring + pos, n);
    } else {
        uint32_t l = r->cons.size - pos;
        memcpy_s(buffer, r->ring + pos, l);
        memcpy_s(buffer + l, r->ring, n - l);
    }

    /* 等待其它出队者完成操作 */
    while (r->cons.tail != cons_head)
        wns_yield();

    r->cons.tail = cons_next;
    return 0;
}

/* 是否为空 */
int32_t h_chunk_ring_empty(const h_chunk_ring_st *r)
{
    uint32_t prod_tail = r->prod.tail;
    uint32_t cons_tail = r->cons.tail;
    return !!(cons_tail == prod_tail);
}

/* 检查空间 */
uint32_t h_chunk_ring_free_space(const h_chunk_ring_st *r)
{
    uint32_t prod_tail = r->prod.tail;
    uint32_t cons_tail = r->cons.tail;
    uint32_t ret = ((cons_tail - prod_tail - 1) & r->prod.mask);
    return ret > sizeof(uint32_t) ? ret - sizeof(uint32_t) : 0;
}

/* 增加写者数，用于判断该环形缓冲区没有人写入 */
int32_t h_chunk_ring_inc_writter(h_chunk_ring_st *r)
{
    return wns_atomic32_add_return(&r->writter, 1);
}

/* 完成写入 */
void h_chunk_ring_done_enqueue(h_chunk_ring_st *r)
{
    wns_atomic32_inc(&r->done_writter);
}

/* 是否完成写入 */
int32_t h_chunk_ring_is_done(h_chunk_ring_st *r)
{
    int32_t v = wns_atomic32_read(&r->writter);
    if (!v)
        return 0;

    return (v == wns_atomic32_read(&r->done_writter)) ? 1 : 0;
}

void h_chunk_ring_destroy(h_chunk_ring_st *r)
{
    h_free(r);
}

#ifdef DEBUG_CHUNK_RING
/* 输出信息 */
void h_chunk_ring_dump(const h_chunk_ring_st *r)
{
#define PRIu32  "u"
    printf("ring <%s>@%p\n", r->name, r);
    printf("  size=%"PRIu32"\n", r->prod.size);
    printf("  ct=%"PRIu32"\n", r->cons.tail);
    printf("  ch=%"PRIu32"\n", r->cons.head);
    printf("  pt=%"PRIu32"\n", r->prod.tail);
    printf("  ph=%"PRIu32"\n", r->prod.head);
    printf("  avail=%"PRIu32"\n", h_chunk_ring_free_space(r));
}
#endif

/* copyright see wns_atomic.h */
