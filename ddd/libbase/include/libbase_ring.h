/*=============================================================================
* @brief                                                                      *
* 无锁环状队列                                                                *
*                                                                             *
*                                                                             *
*                                         @作者:hyb      @date 2013/04/10     *
==============================================================================*/


#ifndef __LIBBASE_RING_H__
#define __LIBBASE_RING_H__

struct h_ring;
typedef struct h_ring h_ring_st;

/* @flags of wns_ring_create define as follow, OR */
#define WNS_RING_F_SP_EN (1 << 0) /* "single-producer" 单生产者使能 */
#define WNS_RING_F_SC_EN (1 << 1) /* "single-consumer" 单消费者使能 */

#define WNS_RING_F_MORE_CORE (1 << 2) /* 带上这个标志，表示cpu核心数比生产者/消费者多，*/
									  /* 否者表示生产者/消费者比cpu核心数多。此标志对单生产者单消费者无效 */

/**
 * 创建一个环状队列
 *
 * 在flags指定多生产者和多消费者的模式下，这个队列也是线程安全的
 *
 * @param name  环状队列的名称
 * @param count 环状队列的大小，容纳的元素个数（必须为2的N次方），非0。实际ring中只有count-1个可用空间。
 * @param flags 单生产者和单消费者标识
 *     可以是 WNS_RING_F_SP_EN 单消费者使能
 *     WNS_RING_F_SC_EN 单消费者使能
 *     或者这两者取或 WNS_RING_F_SP_EN | WNS_RING_F_SC_EN
 *     默认是多生产者多消费者
 *
 * @return
 *     成功返回一个新创建的环状队列，不成功返回NULL
 */
h_ring_st * h_ring_create(const char *name, uint32_t count, uint32_t flags);

/**
 * 销毁一个环状队列
 * 
 * 必须保证在没有出队入队的操作时进行
 *
 * @param ring  环状队列指针
 *
 */
void h_ring_destroy(h_ring_st *ring);

/**
 * 从环状队列中出队
 *
 * 这个函数能不能线程安全，依赖于创建ring是传入的flags
 *
 * @param ring  环状队列指针
 * @param obj_p 存放出队数据的变量的地址
 * 
 * @return
 *     0:  成功，元素出队
 *   非0:  失败，队列没有元素出队，队列为空
 */
int32_t h_ring_dequeue(h_ring_st *ring, void **obj_p);

/**
 * 环状队列入队
 *
 * 这个函数能不能线程安全，依赖于创建ring是传入的flags
 *
 * @param ring  环状队列指针
 * @param obj   入队的数据
 * 
 * @return
 *     0:  成功
 *   非0:  失败
 */
int32_t h_ring_enqueue(h_ring_st *ring, void *obj);

/**
 * 一次出队多个元素，或者理解为一次出队一块内存
 *
 * 这个函数能不能线程安全，依赖于创建ring是传入的flags
 *
 * @param ring  环状队列指针
 * @param obj_table 一个指针指向void *（元素）表的指针
 * @param n  需要出队存入元素表中的出队元素的个数
 *
 * @return
 *     0:  成功
 *   非0:  失败
 */
int32_t h_ring_dequeue_bulk(h_ring_st *ring, void **obj_table, uint32_t n);

/**
 * 一次入队多个元素，或者理解为一次入队一块内存
 * 
 * 这个函数能不能线程安全，依赖于创建ring是传入的flags
 * 
 * @param ring  环状队列指针
 * @param obj_table 一个指针指向void *（元素）表的指针
 * @param n  需要出队存入元素表中的出队元素的个数
 *
 * @return
 *     0:  成功
 *   非0:  失败
 */
int32_t h_ring_enqueue_bulk(h_ring_st *ring, void * const *obj_table, uint32_t n);


/**
 * 队列是否为空
 *
 * 非线程安全，在多线程的时候，不一定能实时反映出ring的状态
 *
 * @param ring  环状队列指针
 * 
 * @return
 *     0: 非空
 *   非0: 空
 */
int32_t h_ring_empty(const h_ring_st *ring);

/**
 * 队列是否为满
 *
 * 非线程安全，在多线程的时候，不一定能实时反映出ring的状态
 *
 * @param ring  环状队列指针
 * 
 * @return
 *     0: 不满
 *   非0: 满
 */
int32_t h_ring_full(const h_ring_st *ring);

/**
 * 队列已经存在元素个数
 *
 * 非线程安全，在多线程的时候，不一定能实时反映出ring的状态
 *
 * @param ring  环状队列指针
 * 
 * @return
 *     个数
 */
uint32_t h_ring_count(const h_ring_st *ring);

/**
 * 队列空闲元素个数
 *
 * 非线程安全，在多线程的时候，不一定能实时反映出ring的状态
 *
 * @param ring  环状队列指针
 * 
 * @return
 *     个数
 */
uint32_t h_ring_free_count(const h_ring_st *ring);

#endif /* __LIBBASE_RING_H__ */
