/**
 * \file binsearch.h
 * 二分法查找
 * \author hyb
 * \defgroup base 基础函数库
 * \{
 */
#ifndef __LIBWNS_BINSEARCH_H__
#define __LIBWNS_BINSEARCH_H__

#include "libbase_type.h"
/**
 * 二分法查找比较函数类型
 * \param elem 数组元素指针
 * \param key 由binsearch传入的key类型
 * \return 如果elem > key应该返回一个正数，如果elem < key，则返回负数，相等则返回0
 */
typedef int32_t (*h_binsearch_cmp_func_t)(const void *elem, const void *key);

/**
 * 对有序数组执行两分法查找
 * \param base 数组起始地址
 * \param nelem 数组元素个数
 * \param esize 数组元素大小
 * \param key 查找目标key
 * \param compar 比较函数
 * \return 数组元素下标，-1为未找到
 */
int32_t h_binsearch(void *base, uint32_t nelem, uint32_t esize, void *key,
        h_binsearch_cmp_func_t compar);


/**
 * 取得最接近key并且小于等于key的元素
 * \param base 数组起始地址
 * \param nelem 数组元素个数
 * \param esize 数组元素大小
 * \param key 查找目标key
 * \param compar 比较函数
 * \return 数组元素下标，-1为未找到
 */
int32_t h_binsearch_lower_bound(void *base, uint32_t nelem, uint32_t esize, void *key,
        h_binsearch_cmp_func_t compar);

/** \} */

#endif /* __LIBWNS_BINSEARCH_H__ */
