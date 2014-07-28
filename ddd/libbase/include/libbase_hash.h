/*=============================================================================
* @brief                                                                      *
* 哈希表(哈希算法为time33)                                                    *
*                                                                             *
*                                                                             *
*                                         @作者:hyb      @date 2013/04/10     *
==============================================================================*/

#ifndef __LIBBASE_HASH_H__
#define __LIBBASE_HASH_H__

typedef struct h_hash_st h_hash_st;

/**
 * 释放哈希表中data字段的函数类型
 * 释放函数不应该对当前hash进行增删操作，否则会出现不必要的问题
 * \param data 数据指针
 */
typedef void (*hash_data_free_func_t)(void *data);

/** 
 * 哈希函数类型
 * \param key 哈希表的key
 * \param klen key的长度
 */
typedef unsigned int (*hash_key_func_t)(const void *key, int klen);

/**
 * 取得哈希表项数
 */
unsigned int h_hash_count(h_hash_st *ht);

/**
 * 创建一个哈希表
 * 使用hash_destroy释放创建的哈希表
 * \param del 释放data的回调函数，data释放将在哈希表销毁或key被替换时执行，可以为空（默认不释放）
 * \param keyf 使用自定义的哈希函数，可以为空（默认使用time33对字节进行计算）
 * \param soltnum 哈希表的哈希桶数量, 哈希桶数量为0时哈希桶会动态增长，哈希桶的数量必须是2的n次方。
 * \retval NULL 创建失败
 * \retval !NULL 哈希表结构指针
 * 注意：
 *     内存在slotnum == 0的模式下将会随着插入内存膨胀，并且不会随着元素删除收缩，但会随着destory调用销毁。
 */
h_hash_st *h_hash_create(hash_data_free_func_t del, hash_key_func_t keyf, unsigned int slotnum);

/**
 * 哈希表插入
 * \note 如果插入的key存在，新的val将替换已存在的数据，被替换的数据将调用哈希表data释放回调进行释放
 * \param ht 哈希表结构
 * \param key key字段
 * \param klen key的长度
 * \param val 插入的数据
 * \retval 0 插入成功
 * \retval !0 插入失败，内存分配不到等等原因
 */
int h_hash_insert(h_hash_st *ht, const void *key, int klen, void *val);

/**
 * 哈希表查找
 * \param ht 哈希表结构
 * \param key key字段
 * \param klen key的长度
 * \param [out] val 返回数据指针
 * \retval 0 成功查找
 * \retval !0 无法找到
 */
int h_hash_search(h_hash_st *ht, const void *key, int klen, void **val);

/**
 * 哈希表删除键
 * \note 删除的数据将调用data释放回调进行释放
 * \param ht 哈希表结构
 * \param key key字段
 * \param len key的长度
 * \retval 0 成功删除
 * \retval !0 无法找到键值
 */
int h_hash_delete(h_hash_st *ht, const void *key, int len);

/**
 * 哈希表释放
 * \note 所有数据将调用data释放回调进行释放
 * \param ht 哈希表结构
 */
void h_hash_destroy(h_hash_st *ht);

/**
 * 哈希表遍历函数类型，遍历时将在每个key-value上调用该函数
 * \param key 索引字段
 * \param klen 索引字段长度
 * \param val 值指针
 * \param data 自定义数据
 * \return 正常必须返回0，否则将引发遍历函数返回该值
 */
typedef int (*hash_walk_func_t)(const void *key, int klen, void *val, void *data);

/**
 * 哈希表遍历，对所有的键值对调用fn回调
 * \param ht 哈希表结构
 * \param data 用户回调数据
 * \param fn 对每个键值对调用该回调，如果回调返回非0值，立即结束遍历并将该值返回
 * \retval 0 成功进行遍历操作
 * \retval !0 回调返回了非0值
 */
int h_hash_walk(h_hash_st *ht, void *data, hash_walk_func_t fn);

/**
 * 哈希表部分遍历的中间状态结构
 */
typedef struct h_hcookie_st h_hcookie_st;

/**
 * 创建一个遍历cookie
 * \param percent 要遍历的百分比(0-1)
 * \return 成功返回一个cookie，否则为NULL
 */
h_hcookie_st *h_hash_cookie_create(float percent);

/**
 * 释放cookie
 * \param cookie 合法的cookie
 */
void h_hash_cookie_destroy(h_hcookie_st *cookie);

/**
 * 重新配置遍历的百分比
 * \param cookie 合法的cookie
 * \param percent 要遍历的百分比(0-1)
 */
void h_hash_cookie_set_percent(h_hcookie_st *cookie, float percent);

/**
 * 哈希表部分遍历，由cookie指定百分比，并记录当前位置
 * \note 百分比仅代表大概数目，并不是准确数目，与wns_hash_walk不同的是，该函数不会理会回调的返回值
 * \param ht 哈希表结构
 * \param cookie cookie控制结构
 * \param data 用户数据
 * \param fn 回调函数
 */
void h_hash_walk_partial(h_hash_st *ht, h_hcookie_st *cookie, void *data, hash_walk_func_t fn);

void h_hash_slotinfo(h_hash_st *ht, int *counts, int ncount);



/** \} */
#endif /* __LIBBASE_HASH_H__ */
