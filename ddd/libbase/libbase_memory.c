/*=============================================================================
* @brief                                                                      *
* 内存管理基础接口，方便内存信息跟踪                                          *
*                                                                             *
*                                                                             *
*                                         @作者:hyb     @date 2013/04/10      *
==============================================================================*/


#include <stdlib.h>
#include <string.h>

#include "libbase_error.h"
#include "libbase_memory.h"
#include "libbase_stack.h"
#include "libbase_atomic.h"
#include "libbase_strutils.h"
#include "libbase_config.h"
#include "libbase_comm.h"

/* 默认的内存分配接口, 使用free */
static void *system_alloc(void *allocator_data, size_t size)
{
    (void)allocator_data;
    if (size == 0) {
        return NULL;
    }
    return malloc(size);
}

/* 默认的内存释放接口, 使用free */
static void system_free (void *allocator_data, void *data)
{
    (void)allocator_data;
    if (data) {
        free(data);
    }
}

/* use malloc, free etc */
WnsAllocator wns_default_alloctor =
{
    system_alloc,
    system_free,
    NULL
};

static WnsAllocator *s_allocator = &wns_default_alloctor;

static wns_atomic32_t alloc_cnt = WNS_ATOMIC32_INIT(0);
static wns_atomic32_t free_cnt = WNS_ATOMIC32_INIT(0);
static wns_atomic32_t alloc_size = WNS_ATOMIC32_INIT(0);

/* 内存分配跟踪层
 *
 * /```` malloc 分配一段内存 `````\
 * |------|------------------------|
 * ^      ^
 * |      |__ 从这里返回给用户
 * |
 * |_ 记录分配信息
 *
 * free的时候，根据用户传过来的指针往前移动算出内存大小
 *
 * */
/* parasoft unsuppress item * struct mem_block */
struct mem_block {
    uint32_t size;    /* 内存块大小 */
    uint32_t pad[3];  /* 用来128bit 对齐，对32/64bit系统通用 */
    uint8_t space[0]; /* 真正传给用户的空间 */
};

/* 内部结构指针转换为用户指针 */
#define P2USERP(ptr) (ptr == NULL ? NULL : (void *)ptr->space)

/* 用户指针转换为内部结构指针 */
#define USERP2P(ptr) \
    (ptr == NULL ? NULL : container_of(ptr, struct mem_block, space))

void h_set_alloctor(WnsAllocator *allocator)
{
    if (allocator
            && allocator->alloc
            && allocator->free) {
        s_allocator = allocator;
    }
}

void *h_malloc(size_t size)
{
    struct mem_block *ret = s_allocator->alloc(s_allocator->allocator_data,
            sizeof(struct mem_block) + size);
    if (likely(ret)) {
        memset(ret, 0, sizeof(struct mem_block) + size);
        ret->size = size;
        wns_atomic32_inc(&alloc_cnt);
        wns_atomic32_add(&alloc_size, (int32_t)size);
    }
    return P2USERP(ret);
}

void *h_realloc(void *ptr, size_t size)
{
    struct mem_block *ret;
    uint32_t oldsize;
    /* 这里的判断NULL和size == 0的代码，其实是realloc函数里面实现的逻辑，这里做判断，是为了跟踪内存分配计数 */
    if (unlikely(ptr == NULL))
        return h_malloc(size);

    if (unlikely(size == 0)) {
        h_free(ptr);
        return NULL;
    }
    ret = USERP2P(ptr);
    oldsize = ret->size;

    void *new = h_malloc(size);
    if (likely(new)) {
        memcpy(new, ptr, size > oldsize ? oldsize : size);
        h_free(ptr);
        USERP2P(new)->size = size;
    }
    return new;
}

void *h_calloc(size_t nmemb, size_t size)
{
    size_t bytes = nmemb * size;
    return h_malloc(bytes);
}

void h_free(void *ptr)
{
    struct mem_block *ret;
    if (likely(ptr)) {
        ret = USERP2P(ptr);
        wns_atomic32_inc(&free_cnt);
        wns_atomic32_sub(&alloc_size, ret->size);
        s_allocator->free(s_allocator->allocator_data, ret);
    }
}

void *zero_alloc(size_t size)
{
    void *r = h_malloc(size);
    if (!r) {
        wns_die("Out of memory\n");
    }
    memset(r, 0, size);
    return r;
}

uint32_t h_mem_get_alloc_count()
{
    return wns_atomic32_read(&alloc_cnt);
}

void h_mem_clear_alloc_count()
{
    wns_atomic32_set(&alloc_cnt, 0);
}

uint32_t h_mem_get_free_count()
{
    return wns_atomic32_read(&free_cnt);
}

void h_mem_clear_free_count()
{
    wns_atomic32_set(&free_cnt, 0);
}

uint32_t h_mem_get_alloc_size_count()
{
    return wns_atomic32_read(&alloc_size);
}

void h_mem_clear_alloc_size_count()
{
    wns_atomic32_set(&alloc_size, 0);
}

#define WNS_MEM_POOL_NAMESIZE 32
struct h_mem_pool {
    char name[WNS_MEM_POOL_NAMESIZE];
    h_stack_st *free_stack; /* 空闲内存，首地址用栈管理 */
    uint8_t data[0] __wns_arch_cache_aligned;
};

#ifdef SF_ARCH_X86_64
/* 补齐8的倍数 */
#define PAD_ALIGNED(x) (((x) & 0x7) ? (((x) | 0x7) + 0x1) : (x))
#else
/* 补齐4的倍数 */
#define PAD_ALIGNED(x) (((x) & 0x3) ? (((x) | 0x3) + 0x1) : (x))
#endif

h_mem_pool_st *h_mem_pool_create(const char *name, uint32_t nmemb,
        uint32_t size, uint32_t flags)
{
    h_mem_pool_st *mpd;
    int i;
    size = PAD_ALIGNED(size);
    if ((mpd = h_malloc(sizeof(h_mem_pool_st) + nmemb * size)) == NULL)
        return NULL;

    if ((mpd->free_stack = h_stack_create(name, nmemb, 0)) == NULL)
        goto err;

    for (i = 0; i < nmemb; i++) {
        if (h_stack_push(mpd->free_stack,
                    (void *)((uintptr_t)(mpd->data) + i * size)) != 0)
            goto err2;
    }
    sprintf_n(mpd->name, WNS_MEM_POOL_NAMESIZE, "%s", name);

    return mpd;

err2:
    h_stack_destroy(mpd->free_stack);
err:
    h_free(mpd);
    return NULL;
}

void h_mem_pool_destroy(h_mem_pool_st *mpd)
{
    h_stack_destroy(mpd->free_stack);
    h_free(mpd);
    return;
}
void *h_mem_pool_get(h_mem_pool_st *mpd)
{
    void *ret;
    if (h_stack_pop(mpd->free_stack, &ret) != 0)
        return NULL;

    return ret;
}

void h_mem_pool_put(h_mem_pool_st *mpd, void *ptr,int id)
{
    if (h_stack_push(mpd->free_stack, ptr) != 0)
    {
        printf("die in id:%d\n",id);
        wns_die("memory pool stack error\n");
    }
    return;
}
