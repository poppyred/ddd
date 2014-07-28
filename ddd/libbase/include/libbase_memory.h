/*=============================================================================
* @brief                                                                      *
* 内存管理基础接口，方便内存信息跟踪                                          *
*                                                                             *
*                                                                             *
*                                         @作者:hyb      @date 2013/04/10     *
==============================================================================*/

#ifndef __LIBWNS_MEMORY_H__
#define __LIBWNS_MEMORY_H__

#include "libbase_comm.h"
#include "libbase_type.h"
#include "libbase_config.h"

/* 参考自protobuf-c的实现代码 */
/* --- memory management --- */
typedef struct _WnsAllocator WnsAllocator;
struct _WnsAllocator
{
    void *(*alloc)(void *allocator_data, size_t size);
    void (*free)(void *allocator_data, void *pointer);
    void *allocator_data;
};

/*
 * This is the default system allocator, meaning it uses malloc() and free().
 */
extern WnsAllocator wns_default_alloctor;

/*!
 * 设置自定义内存分配和内存释放函数
 * @allocator 内存分配和释放函数指针
 */
void h_set_alloctor(WnsAllocator *allocator);

/**
 * h_malloc - 和C库malloc一样，分配一段大小为 @size的内存
 * 参数：
 * @size: 要分配的内存的大小
 * 返回值:
 *     分配的内存块首地址 - 成功
 *     NULL - 失败
 * 注意：
 *     线程安全
 */
void *h_malloc(size_t size);

/**
 * h_realloc - 和C库realloc一样，改变分配的内存块大小
 * 参数：
 * @ptr:  已经分配的内存块指针
 * @size: 要分配的内存的大小
 * 返回值:
 *     分配的内存块首地址 - 成功
 *     NULL - 失败
 * 注意：
 *     线程安全
 *     当@ptr==NULL相当于调用了wns_malloc
 *     当@size==0相当于调用了wns_free，并返回NULL
 */
void *h_realloc(void *ptr, size_t size);

/**
 * h_calloc - 和C库calloc一样，分配@nmemb个大小为@size的连续内存块
 * 参数：
 * @nmemb:  成员数
 * @size:   成员大小
 * 返回值:
 *     分配的内存块首地址 - 成功
 *     NULL - 失败
 * 注意：
 *     calloc会对分配的内存清零
 *     线程安全
 */
void *h_calloc(size_t nmemb, size_t size);

/**
 * h_free - 和C库free一样，释放分配的内存
 * 参数：
 * @ptr:  分配的内存块首地址
 * 返回值:
 *     无
 * 注意：
 *     线程安全
 */
void h_free(void *ptr);

/**
 * zero_alloc - 分配一段内存，并将内存数据清零
 * 参数：
 * @size:  要分配的内存大小
 * 返回值:
 *     分配的内存块首地址 - 成功
 * 注意：
 *     危险接口。
 *     此接口内存分配失败会直接打印Out of memory 后挂掉，留在这里为了保持兼容性。建议使用wns_calloc然后判断返回值。
 *     因为以前是这么设计的，留在这类保持兼容性。
 */
void *zero_alloc(size_t size);

/**
 * h_mem_get_alloc_count - 获取wns_的所有内存分配调用次数
 * 参数：
 *     无
 * 返回值：
 *     返回内存分配调用次数
 * 注意：
 *     线程安全
 */
uint32_t h_mem_get_alloc_count();

/**
 * h_mem_clear_alloc_count - 清零alloc计数器
 * 参数：
 *     无
 * 返回值：
 *     无
 * 注意：
 *     线程安全
 */
void h_mem_clear_alloc_count();

/**
 * h_mem_get_free_count - 获取wns_的所有内存释放调用次数
 * 参数：
 *     无
 * 返回值：
 *     返回内存释放调用次数
 * 注意：
 *     线程安全
 */
uint32_t h_mem_get_free_count();

/**
 * h_mem_clear_free_count - 清零free计数器
 * 参数：
 *     无
 * 返回值：
 *     无
 * 注意：
 *     线程安全
 */
void h_mem_clear_free_count();

/**
 * h_mem_get_alloc_size_count - 获取内存分配的大小的统计
 * 参数：
 *     无
 * 返回值：
 *     返回内存释放调用次数
 * 注意：
 *     线程安全
 */
uint32_t h_mem_get_alloc_size_count();

/**
 * h_mem_clear_alloc_size_count - 清零alloc_size计数器
 * 参数：
 *     无
 * 返回值：
 *     无
 * 注意：
 *     线程安全
 */
void h_mem_clear_alloc_size_count();



typedef struct h_mem_pool h_mem_pool_st;

/**
 * h_mem_pool_create - 建立一个内存池
 * 参数:
 * @name:   内存池的名字
 * @nmemb:  内存池所含有的内存块数量
 * @size:   内存池每个内存块的大小
 * @flags:  内存池的特性
 *     暂时保留，以后可能加上无锁的特性
 * 返回值：
 *     返回内存池的描述符（指针）
 * 注意：
 *     非线程安全，为了内存池的分配效率，建议每个cpu有自己的内存池。
 */
h_mem_pool_st *h_mem_pool_create(const char *name, uint32_t nmemb, uint32_t size, uint32_t flags);

/**
 * h_mem_pool_destroy - 摧毁一个内存池
 * 参数:
 * @mpd:   内存池描述符
 * 返回值：
 *     无
 * 注意：
 *     非线程安全，为了内存池的分配效率，建议每个cpu有自己的内存池。
 */
void h_mem_pool_destroy(h_mem_pool_st *mpd);

/**
 * h_mem_pool_get - 从内存池取出一个内存块
 * 参数:
 * @mpd:   内存池描述符
 * 返回值：
 *     返回内存块的首地址
 * 注意：
 *     非线程安全，为了内存池的分配效率，建议每个cpu有自己的内存池。
 */
void *h_mem_pool_get(h_mem_pool_st *mpd);

/**
 * h_mem_pool_put - 把从内存池取出的内存块重新放回内存池
 * 参数:
 * @mpd:   内存池描述符
 * @ptr:   要释放的内存块首地址，不检验参数合法性，调用者必须保证传进来的地址是wns_mem_pool_get得到的。
 * 返回值：
 *     返回内存块的首地址
 * 注意：
 *     非线程安全，为了内存池的分配效率，建议每个cpu有自己的内存池。
 */
void h_mem_pool_put(h_mem_pool_st *mpd, void *ptr,int id);

#endif /* __LIBWNS_MEMORY_H__ */
