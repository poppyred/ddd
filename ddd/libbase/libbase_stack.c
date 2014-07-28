/**
 * author hyb
 */
#include <string.h>
#include <stdio.h>
#include "libbase_stack.h"

#include "libbase_type.h"
#include "libbase_memory.h"

/**
 * 栈结构
 */

#define DEFAULT_STACK_SIZE 8

struct stack_st {
    void **base;
    volatile uint32_t sp; /* 栈顶index 指向栈顶第一个可用的元素 */
    uint32_t size;
    uint32_t __fixed;
};

/**
 * 创建一个栈
 * \param name       栈的名字
 * \param fixed_size 固定长度，如果非0则指定固定长度，超出这个长度的栈push时会失败
 * \param flags 特性标识
 *     暂时保留，以后可能加入无锁特性
 * \return 成功返回一个新创建的堆
 */
h_stack_st *h_stack_create(const char *name, uint32_t fixed_size,
        uint32_t flags)
{
    h_stack_st *stack;
    if ((stack = h_malloc(sizeof(*stack))) == NULL)
        return NULL;

    bzero(stack, sizeof(*stack));

    stack->sp = 0;
    if (fixed_size != 0) {
        if ((stack->base = h_malloc(sizeof(void *) * fixed_size)) == NULL)
            goto err;
        stack->size = fixed_size;
        stack->__fixed = 1;
    } else {
        stack->base = h_malloc(sizeof(void *) * DEFAULT_STACK_SIZE);
        if (stack->base == NULL)
            goto err;
        stack->size = DEFAULT_STACK_SIZE;
        stack->__fixed = 0;
    }

    return stack;

err:
    h_free(stack);
    return NULL;
}

/**
 * 销毁一个栈
 * \param st 合法的栈
 */
void h_stack_destroy(h_stack_st *stack)
{
    h_free(stack->base);
    h_free(stack);
}

/**
 * 向栈中压入一个元素
 * \param st 合法的堆
 * \param data 用户数据
 * \return 如果堆指定了fixed_size，则在堆满时返回非0，其它情况都返回0。
 *         如果是动态增长的，在内存分配失败的时候返回非零。
 */
int h_stack_push(h_stack_st *stack, void *data)
{
    void *tmp;

    if (stack->sp == stack->size)
    {
        printf("--------- stack->sp:%d stack->size:%d! -----------\n",stack->sp,stack->size);
    }
    
    /* 堆栈满是处理异常和动态增长 */
    if (stack->sp > stack->size) {
        
        if (stack->__fixed)
        {
            printf("stack->sp:%d stack->size:%d!\n",stack->sp,stack->size);
            return -1;
        }
        
        printf("enlarge!!\n");
        stack->size = stack->size << 1;
        tmp = h_realloc(stack->base, stack->size * sizeof(void *));
        if (!tmp) {
            printf("die there!\n");
            return -1;
        }
        stack->base = tmp;
    }
    stack->base[stack->sp] = data;
    stack->sp++;
    return 0;
}

/**
 * 从栈上弹出一个数据
 * \param st 合法的堆
 * \param ret 弹出数据存放地址
 * \return 成功返回0，否则返回非0
 */
int h_stack_pop(h_stack_st *stack, void **ret)
{
    void *tmp;
    unsigned int ssize, size;

    if (stack->sp == 0)
        return -1;
    //stack->sp--;
    int temp = --stack->sp;

    if (!stack->__fixed) {
        ssize = stack->size >> 2; /* 触发收缩的大小，这里是到四分之一的时候收缩 */
        size = stack->size >> 1;  /* 收缩之后的大小,这里是收缩到原大小一半 */
        if (temp < ssize && size >= DEFAULT_STACK_SIZE) {
            tmp = h_realloc(stack->base, size * sizeof(void *));
            if (tmp != NULL) {
                stack->size = size;
                stack->base = tmp;
            }
        }
    }
    *ret = stack->base[temp];
    return 0;
}
