/**
 * \file heap.h
 * 堆数据结构及程序接口
 * \author hyb
 * \defgroup base 基础函数库
 * \{
 */
 
#ifndef __LIBWNS_HEAP_H__
#define __LIBWNS_HEAP_H__

/**
 * 堆结构
 */
typedef struct heap_st h_heap_st;

/**
 * 堆节点比较函数
 * 需返回一个节点比较结果，一般来说返回一个对比差值（类似于qsort）
 * 返回a-b的值则会形成小堆(小值先出)，返回b-a则形成大堆
 * \param a 比较第一个值
 * \param b 比较第二个值
 * \return a与b的差值
 */
typedef intptr_t (*heap_comp_func_t)(void *a, void *b);



/**
 * 创建一个堆
 * \param cmp 自定义的比较函数，如果不指定，则默认比较的是push时的指针
 * \param fixed_size 固定长度，如果非0则指定固定长度，超出这个长度的堆push时会失败。为0表示动态增长。
 * \return 成功返回一个新创建的堆
 */
h_heap_st *h_heap_create(heap_comp_func_t cmp, uint32_t fixed_size);

/**
 * 销毁一个堆
 * \param hp 合法的堆
 */
void h_heap_destroy(h_heap_st *hp);

/**
 * 返回堆的数目
 */
uint32_t h_heap_count(h_heap_st *hp);

/**
 * 取回（但不删除）堆顶（最大/最小元素）
 * \note 堆push时可以是NULL，所以这里无法判断是取回NULL还是没有元素，需自行heap_count取得元素个数
 */
void * h_heap_gettop_simple(h_heap_st *hp);

/**
 * 取得（但不删除）堆顶
 * \note 这个是不安全版本，需要自行确保heap_count大于0
 */
void * h_heap_gettop_unsafe(h_heap_st * hp);

/**
 * 向堆中压入一个元素
 * \param hp 合法的堆
 * \param data 用户数据（将使用创建堆时的比较函数）
 * \return 如果堆指定了fixed_size，则在堆满时返回非0，其它情况都返回0
 */
int h_heap_push(h_heap_st *hp, void *data);

/**
 * 取出并删除堆顶
 * \param hp 合法的堆
 * \param ret 堆顶返回位置
 * \return 成功返回0，否则返回非0
 */
int h_heap_pop(h_heap_st *hp, void **ret);

/** \} */

#endif /* __LIBWNS_HEAP_H__ */
