/**
 * \file wns_chunk_ring.h
 * 块数据环形缓冲区实现，改造于rte_ring
 * \defgroup wns_chunk_ring 块数据环形缓冲区
 * \{
 */

#ifndef __LIBWNS_CHUNK_RING_H__
#define __LIBWNS_CHUNK_RING_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <stdint.h>

#include "libbase_config.h"

#include "libbase_atomic.h"

#define CHUNK_RING_NAMESIZE 32 ///

typedef struct h_chunk_ring_st h_chunk_ring_st;

/**
 * 在指定内存上创建环形缓冲区
 * \param name 缓冲区名称
 * \param size 缓冲区长度，必须为2的N次方（每一个元素需要一个管理结构，因此缓冲区的大小不是简单的元素大小乘以元素个数，而是略大）
 * \param flags 保留
 *     为了和wns_ring的接口统一
 * \return 正常情况返回mem，否则返回NULL（size不是2的N次方）
 *
 * chunk_ring 是多生产者多消费者线程安全的
 */
h_chunk_ring_st *h_chunk_ring_create(const char *name, uint32_t size, uint32_t flags);

/**
 * 销毁一个环状队列
 *
 * 必须保证在没有出队入队的操作时进行
 *
 * @param ring  环状队列指针
 *
 */
void h_chunk_ring_destroy(h_chunk_ring_st *r);

/**
 * 入队一个chunk
 * \param r 环形缓冲区
 * \param chunk 块数据指针
 * \param chunk_size 块数据长度
 * \return
 *   - 0: 成功
 *   - -EINVAL: 块大小超过ring限制(max_chunk指定)
 *   - -ENOBUFS: 空间不足
 */
int32_t h_chunk_ring_enqueue(h_chunk_ring_st *r, const char *chunk, uint32_t chunk_size);

/**
 * 获取一个可以出队的chunk, 但是并不真正出队
 * 注意出队缓冲要能够完整放入一个chunk
 * \param r 环形缓冲区
 * \param chunk 块数据缓冲区指针
 * \param chunk_size [in/out] 输入传入chunk缓冲区的大小，输出可以出队的chunk块的实际长度
 * \return
 *   - 0: 成功
 *   - -ENOENT:  队列空，没有出队
 *   - -ENOBUFS: 缓冲区空间不够
 */
int32_t h_chunk_ring_peek(h_chunk_ring_st *r, char *chunk, uint32_t *chunk_size);

/**
 * 出队一个chunk，注意出队缓冲要能够完整放入一个chunk
 * \param r 环形缓冲区
 * \param chunk 块数据缓冲区指针
 * \param chunk_size [in/out] 输入传入chunk缓冲区的大小，输出出队的chunk块的实际长度
 * \return
 *   - 0: 成功
 *   - -ENOENT:  队列空，没有出队
 *   - -ENOBUFS: 缓冲区空间不够
 */
int32_t h_chunk_ring_dequeue(h_chunk_ring_st *r, char *chunk, uint32_t *chunk_size);

/**
 * 环形缓冲区是否为空
 * \param r 环形缓冲区
 * \return 1为空，0非空
 */
int32_t h_chunk_ring_empty(const h_chunk_ring_st *r);

/**
 * 取得环形缓冲区空闲空间（以决定能否放入chunk）
 * \param r 环形缓冲区
 * \param 空闲空间数
 */
uint32_t h_chunk_ring_free_space(const h_chunk_ring_st *r);

/**
 * 增加写者数，用于判断该环形缓冲区没有人写入
 * \param r 环形缓冲区
 * \return 增加后的写者数目
 */
int32_t h_chunk_ring_inc_writter(h_chunk_ring_st *r);

/**
 * 结束写入，在所有写者全部结束写入后，缓冲区内容为空的情况下，总会取得0字节的数据，表示该缓冲区不会再有人写入
 * \param r 环形缓冲区
 */
void h_chunk_ring_done_enqueue(h_chunk_ring_st *r);

/**
 * 是否完成写入
 * @return 1:完成写入
 * @return 0:没有完成写入, 或者没有任何写者
 */
int32_t h_chunk_ring_is_done(h_chunk_ring_st *r);

#ifdef DEBUG_CHUNK_RING
/**
 * 打印环形缓冲区信息
 * \param r 环形缓冲区
 */
void h_chunk_ring_dump(const h_chunk_ring_st *r);
#endif

/** \} */

#endif /* __LIBWNS_CHUNK_RING_H__ */


/* copyright see rte_atomic.h */
