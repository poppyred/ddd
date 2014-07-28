/**
 * \file rbtree.h
 * 红黑树实现
 * \author hyb
 * \defgroup base 基础函数库
 * \{
 */

#ifndef __LIBBASE_RBTREE_H__
#define __LIBBASE_RBTREE_H__

#include <stdint.h>

/**
 * 红黑树类型结构
 */
struct h_rbtree_st;
typedef struct h_rbtree_st h_rbtree_st;

/**
 * 红黑树节点比较函数，对于从小到大的顺序，当key1大于key2的情况下返回正数，否则返回负数
 * \param key1 比较的key1
 * \param ksize1 key1的长度
 * \param key2 比较的key2
 * \param ksize2 key2的长度
 * \return 返回一个数字以决定红黑树遍历的顺序
 */
typedef int (*rbtree_cmp_func_t)(const void *key1, uint32_t ksize1,
        const void *key2, uint32_t ksize2);

/**
 * 插入红黑树中数据的释放函数
 * \param data 数据
 */
typedef void (*rbtree_data_free_func_t)(void *data);

/**
 * 创建红黑树对像
 * \param data_free 插入数据的释放函数，会在红黑树销毁或节点被替换时自动调用释放该数据
 * \param cmp 节点key比较函数
 * \return 红黑树
 */
h_rbtree_st *h_rbtree_create(rbtree_data_free_func_t data_free, rbtree_cmp_func_t cmp);

/**
 * 释放红黑树
 * \param rb 红黑树
 */
void h_rbtree_destroy(h_rbtree_st *rb);

/**
 * 取得红黑树中的节点数
 * \param rb 红黑树
 * \return 节点数
 */
uint32_t h_rbtree_count(h_rbtree_st *rb);

/**
 * 红黑树遍历光标结构，使用该光标取得有序的前后元素(类似于iterator概念)
 */
struct h_rb_cursor_st;
typedef struct h_rb_cursor_st h_rb_cursor_st;

/**
 * 取得第一个元素，遍历不安全问题，如果对当前cursor的key进行删除操作，则当前cursor将会失效，不能再用于next/prev作为参数
 * 在这种情况下需要先取得下一个元素，再将本元素进行删除
 * \param rb 红黑树
 * \return 正常返回第一个元素的光标，没有元素返回NULL
 */
const h_rb_cursor_st *h_rbtree_first(const h_rbtree_st *rb);

/**
 * 取得最后一个元素
 * \param rb 红黑树
 * \return 正常返回最后一个元素的光标，没有元素返回NULL
 */
const h_rb_cursor_st *h_rbtree_last(const h_rbtree_st *rb);

/**
 * 取得当前光标的下一个元素
 * \param itor 当前光标
 * \return 正常返回下一个元素的光标，没有元素返回NULL
 */
const h_rb_cursor_st *h_rbtree_next(const h_rb_cursor_st *itor);

/**
 * 取得当前光标的上一个元素
 * \param itor 当前光标
 * \return 正常返回上一个元素的光标，没有元素返回NULL
 */
const h_rb_cursor_st *h_rbtree_prev(const h_rb_cursor_st *itor);

/**
 * 取得光标所指元素的key
 * \param itor 当前光标
 * \return 返回key的指针
 */
void *h_rb_cursor_get_key(const h_rb_cursor_st *itor);

/**
 * 取得光标所指元素的key size
 * \param itor 当前光标
 * \return 返回key的size
 */
uint32_t h_rb_cursor_get_ksize(const h_rb_cursor_st *itor);

/**
 * 取得光标所指元素的val
 * \param itor 当前光标
 * \return 返回元素的值
 */
void *h_rb_cursor_get_val(const h_rb_cursor_st *itor);

/**
 * 搜索key对应的值，与rbtree_get_cursor类同
 * \param rb 红黑树
 * \param key key
 * \param ksize key的长度
 * \param val [out] 如果非空，则从该位置返回对应的值
 * \return 找到元素返回0，否则返回-1
 */
int h_rbtree_search(const h_rbtree_st *rb, const void *key, uint32_t ksize, void **val);

/**
 * 搜索key对应的光标，与rbtree_search类似，但返回光标可用于遍历
 * \param rb 红黑树
 * \param key key
 * \param ksize key的长度
 * \return 找到元素返回光标，否则返回NULL
 */
const h_rb_cursor_st *h_rbtree_get_cursor(const h_rbtree_st *rb, const void *key, uint32_t ksize);

/**
 * 搜索大于等于key第一个元素光标(逻辑上，取决于红黑树对比函数)
 * \param rb 红黑树
 * \param key key
 * \param ksize key的长度
 * \return 找到元素返回光标，否则返回NULL
 */
const h_rb_cursor_st *h_rbtree_lower_bound(const h_rbtree_st *rb, const void *key, uint32_t ksize);

/**
 * 搜索小于等于key第一个元素光标(逻辑上，取决于红黑树对比函数)
 * \param rb 红黑树
 * \param key key
 * \param ksize key的长度
 * \return 找到元素返回光标，否则返回NULL
 */
const h_rb_cursor_st *h_rbtree_upper_bound(const h_rbtree_st *rb, const void *key, uint32_t ksize);

/**
 * 插入一个元素，如果key存在，则替换掉对应的值，被替换的旧值自动使用create时传入的析构函数进行释放
 * \param rb 红黑树
 * \param key key
 * \param ksize key的长度
 * \param val 对应的值，该值会自动使用create时传入的析构函数进行释放
 * \return 0成功，非0失败。
 */
int h_rbtree_insert(h_rbtree_st *rb, const void *key, uint32_t ksize, void *val);

/**
 * 删除一个元素，如果key存在，对应的值将自动使用create时传入的析构函数进行释放
 * \param rb 红黑树
 * \param key key
 * \param ksize key的长度
 * \return 0成功，非0没有这个key
 */
int h_rbtree_delete(h_rbtree_st *rb, const void *key, uint32_t ksize);

/**
 * 红黑树遍历函数类型，通常创建这样的函数供rbtree_walk作为参数调用
 * \param key 键
 * \param ksize 键的长度
 * \param val 值
 * \param userdata 用户数据
 * \return 返回非0值将结束遍历并在rbtree_walk中返回该值
 */
typedef int (*rbtree_walk_func_t)(const void *key, int ksize, void *val, void *userdata);

/**
 * 遍历红黑树
 * \param rb 红黑树
 * \param userdata 传递给处理函数的用户数据
 * \param fn 每个键值对的处理函数
 * \return 正常返回0，如果处理函数返回非0值，将立即结束遍历过程并返回该值
 */
int h_rbtree_walk(h_rbtree_st *rb, void *userdata, rbtree_walk_func_t fn);

/** \} */

#endif /* __LIBBASE_RBTREE_H__ */
