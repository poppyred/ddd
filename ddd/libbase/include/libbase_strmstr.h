/**
 * \file strmstr.h
 * \author hyb
 * \defgroup base 基础函数库
 * \{
 * \note 获得更最高速的多模式搜索: 添加-DMSTR_COMPILER 单独编译strmstr.c会得到一个状态生成器，并结合使用tools/inline-mstr-dfa工具，将生成的状态转换为c代码，可以获得最快速的多模式搜索静态代码。
 */

#ifndef __LIBWNS_STRMSTR_H__
#define __LIBWNS_STRMSTR_H__

struct mstr_ctrl_st;

/**
 * 多字串搜索控制结构
 */
typedef struct mstr_ctrl_st mstr_ctrl_st;

/**
 * 搜索状态
 */
typedef unsigned int mstr_state_st;

/**
 * 创建多字串控制结构时忽略大小写选项
 */
#define MSTR_OPT_IGNORE_CASE	0x1

/**
 * 不使用扩展，使用扩展是默认选项，扩展会使用DFA编译，在字串过多的情况下编译效率会成为问题
 */
#define MSTR_OPT_NO_EXTEND		0x2

/**
 * 创建一个多字串控制结构，该结构用于匹配字串
 * \param option 选项（MSTR_OPT_*）
 * \return 控制结构
 */
mstr_ctrl_st *mstr_ctrl_create(unsigned int option);

/**
 * 为控制结构添加一个搜索串
 * \param mc 未被编译的控制结构
 * \param str 要添加的模式串
 * \param len 添加模式串的长度
 * \param sid 在搜索回调时使用的模式串标识
 * \return 0成功，否则失败（重复添加或已经编译将会返回失败）
 */
int mstr_ctrl_add_pattern(mstr_ctrl_st *mc, const unsigned char *str, unsigned int len, void *sid);

/**
 * 编译控制结构，编译后方可用于匹配，无法再添加字串
 * \param mc 未编译的控制结构
 */
void mstr_ctrl_compile(mstr_ctrl_st *mc);

/**
 * 释放控制结构
 * \param mc 合法的控制结构
 */
void mstr_ctrl_destroy(mstr_ctrl_st *mc);

/**
 * 模式搜索回调函数类型，该函数将作为参数传递给mstr_search，在每搜到一个子串将调用一次
 * 返回非0值将结束搜索并导至mstr_search返回这个值
 * \param sid 匹配的子串sid(由add_pattern时指定)
 * \param at 子串出现的相对于搜索源串的位置，如果为负数，则表示该串处于上一次搜索的倒数位置
 * \param userdata 用户数据
 * \return 返回0继续搜索，返回其它值将导致search退出
 */
typedef int (*mstr_func_t)(void *sid, int at, void *userdata);

/**
 * 重置一个状态
 * \param ms 状态
 */
void mstr_state_reset(mstr_state_st *ms);

/**
 * 执行搜索，多次搜索使用同一个未被reset的状态时，认为这些src是连接着的。
 * \code
 mstr_state_st ms;

 mstr_state_reset(&ms);

 mstr_search(mc, &ms, "abcde", 5, NULL, match);		// 1
 mstr_search(mc, &ms, "fghij", 5, NULL, match);		// 2

 //上述两个语句等同于
 mstr_search(mc, &ms, "abcdefghij", 10, NULL, match);	// 3

 //假设mc中包含defgh模式，则在执行语句2时将调用match，at参数为-2
 //在3语句则at参数为3

 * \endcode
 * \param mc 编译的控制结构
 * \param ms 状态
 * \param src 要执行搜索的字串片段
 * \param len src的长度
 * \param userdata 执行callback的用户数据
 * \param callback 搜索到子串的回调
 * \return 如果未指定callback，返回是否搜索到子串，否则返回callback的返回值
 */
int mstr_search(const mstr_ctrl_st *mc, mstr_state_st *ms,
				const unsigned char *src, unsigned int len,
				void *userdata,
				mstr_func_t callback);


/** \} */

#endif /* __LIBWNS_STRMSTR_H__ */
