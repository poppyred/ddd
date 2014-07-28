/**
 * 栈结构及其程序接口
 * LIFO 结构
 *
 */

#ifndef __LIBWNS_STACK_H__
#define __LIBWNS_STACK_H__

#include "libbase_type.h"

/**
 * 栈结构
 */
typedef struct stack_st h_stack_st;

/**
 * 创建一个栈
 * \param name       栈的名字
 * \param fixed_size 固定长度，如果非0则指定固定长度，超出这个长度的栈push时会失败，为0时动态增长
 * \param flags 特性标识
 *     暂时保留，以后可能加入无锁特性
 * \return 成功返回一个新创建的栈
 */
h_stack_st *h_stack_create(const char *name, uint32_t fixed_size, uint32_t flags);

/**
 * 销毁一个栈
 * \param st 合法的栈
 */
void h_stack_destroy(h_stack_st *st);

/**
 * 向栈中压入一个元素
 * \param st 合法的栈
 * \param data 用户数据
 * \return 如果栈指定了fixed_size，则在栈满时返回非0，其它情况都返回0。
 *         如果是动态增长的，在内存分配失败的时候返回非零。
 */
int h_stack_push(h_stack_st *st, void *data);

/**
 * 从栈上弹出一个数据
 * \param st 合法的栈
 * \param ret 弹出数据存放地址
 * \return 成功返回0，否则返回非0
 */
int h_stack_pop(h_stack_st *st, void **ret);

#endif /* __LIBWNS_STACK_H__ */
