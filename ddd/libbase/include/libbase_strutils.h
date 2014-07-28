/**
 * \file strutils.h
 * 字串辅助函数
 * \author hyb
 * \defgroup base 基础函数库
 * \{
 */

#ifndef __LIBWNS_STRUTILS_H__
#define __LIBWNS_STRUTILS_H__

#include "libbase_comm.h"
#include <stdarg.h>

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef SANGFOR_DEBUG
    #define    STRSAFE_ASSERT(expression) do{ \
            if (!(expression)) { \
                fprintf(stderr, \
                    "assertion \"%s\" failed: file \"%s\", line %d\n", \
                    #expression, \
                    __FILE__, __LINE__); \
                abort(); \
            } \
        }while(0)
#else  /* Release version */
    #define    STRSAFE_ASSERT(expression)   do{}while(0)
#endif

/**
 * 字符串子串查找函数
 * \param str 源字串
 * \param slen 源字串长度
 * \param match 要查找的子字串
 * \param mlen 子串的长度
 * \param start 查找的起点
 * \return 子串在源字串中的起始偏移，找不到子串则返回-1
 */
int str_find(const char *str, unsigned int slen,
        const char *match, unsigned int mlen,
        unsigned int start);

/**
 * 代替系统strstr的子串查找函数
 * \param str 源字串
 * \param len 源字串长度
 * \param match 要查找的子字串
 * \param mlen 子串的长度
 * \return 子串在源串的起始指针，找不到子串返回NULL
 */
const char *str_nstr(const char *str, unsigned int len, const char *match, unsigned int mlen);

/**
 * 代替系统strchr函数查找字符，字串允许出现\0字符
 * \param str 源字串
 * \param len 源字串长度
 * \param c 要查找的字符
 * \return 字符出现的起始指针，找不到或超出长度返回NULL
 */
const char *str_nchr(const char *str, unsigned int len, int c);


#define SANGFOR_TRUNC ((size_t)-1)

/**
 *    字符串连接。时间复杂度O(m + n), m和n分别为目的串和源串长度
 *    strcat_n保证缓冲区内字符串以0结尾，除非size == 0
 *    @dst:     目的字符串地址
 *    @size:    目的字符串缓冲区总长度
 *    @src:     源字符串地址
 *    @return    < 0 时表示缓冲区长度不够用，已作截断处理
 */
int strcat_n(char *dst, size_t size, const char *src);

/**
 *    字符串连接，防截断版本。时间复杂度O(m + 2n), m和n分别为目的串和源串长度
 *    strcat_ns保证缓冲区内字符串以0结尾，除非size == 0
 *    @dst:     目的字符串地址
 *    @size:    目的字符串缓冲区总长度
 *    @src:     源字符串地址
 *    @count:   最多连接count个字符到目的串，count为SANGFOR_TRUNC这个特殊值表示缓冲区长度不足时可以截断
 *    @return    < 0 时表示缓冲区长度不够用，count为SANGFOR_TRUNC时截断，否则目的串不做任何修改
 */
int strcat_ns(char *dst, size_t size, const char *src, size_t count);

/**
 *    字符串拷贝
 *    strcpy_n保证缓冲区内字符串以0结尾，除非 size == 0
 *    @dst:     目的字符串地址
 *    @size:    目的字符串缓冲区总长度
 *    @src:     源字符串地址
 *    @return    < 0 时表示缓冲区长度不够用，已作截断处理
 */
int strcpy_n(char *dst, size_t size, const char *src);

/**
 *    字符串拷贝，防截断版本
 *    strcpy_ns保证缓冲区内字符串以0结尾，除非 size == 0
 *    @dst:     目的字符串地址
 *    @size:    目的字符串缓冲区总长度
 *    @src:     源字符串地址
 *    @count:   最多拷贝count个字符到目的串，count为SANGFOR_TRUNC这个特殊值表示缓冲区长度不足时可以截断
 *    @return    < 0 时表示缓冲区长度不够用，count为SANGFOR_TRUNC时截断，否则目的缓冲区不做任何修改
 */
int strcpy_ns(char *dst, size_t size, const char *src, size_t count);

/**
 *    格式化输出到字符串缓冲区
 *    vsprintf_n保证缓冲区内字符串以0结尾，除非 size == 0
 *    @buf:     输出缓冲区地址
 *    @size:    输出缓冲区总长度
 *    @fmt:     输出格式控制字符串
 *    @ap:      可变参数列表
 *    @return    < 0 时表示缓冲区长度不够用，已截断处理；否则返回实际输出的字符个数
 */
int vsprintf_n(char* buf, size_t size, const char* fmt, va_list ap);

/**
 *    格式化输出到字符串缓冲区，防截断版本
 *    vsprintf_ns保证缓冲区内字符串以0结尾，除非 size == 0
 *    @buf:     输出缓冲区地址
 *    @size:    输出缓冲区总长度
 *    @count:   最多输出count个字符到目的缓冲区，count为SANGFOR_TRUNC这个特殊值表示缓冲区长度不足时可以截断
 *    @fmt:     输出格式控制字符串
 *    @ap:      可变参数列表
 *    @return    <  0 时表示缓冲区长度不够用，count为SANGFOR_TRUNC时截断，否则返回空串
 *                 >= 0 时表示实际输出的字符个数
 */
int vsprintf_ns(char* buf, size_t size, size_t count, const char* fmt, va_list ap);

/**
 *    格式化输出到字符串缓冲区
 *    sprintf_n保证缓冲区内字符串以0结尾，除非 size == 0
 *    @buf:     输出缓冲区地址
 *    @size:    输出缓冲区总长度
 *    @fmt:     输出格式控制字符串
 *    @...:     可变参数
 *    @return    < 0 时表示缓冲区长度不够用，已截断处理；否则返回实际输出的字符个数
 */
int sprintf_n(char* buf, size_t size, const char* fmt, ...) __attribute__((format(printf, 3, 4)));

/**
 *    格式化输出到字符串缓冲区
 *    sprintf_ns保证缓冲区内字符串以0结尾，除非 size == 0
 *    @buf:     输出缓冲区地址
 *    @size:    输出缓冲区总长度
 *    @count:   最多输出count个字符到目的缓冲区，count为SANGFOR_TRUNC这个特殊值表示缓冲区长度不足时可以截断
 *    @fmt:     输出格式控制字符串
 *    @...:     可变参数
 *    @return    <  0 时表示缓冲区长度不够用，count为SANGFOR_TRUNC时截断，否则返回空串
 *                 >= 0 时表示实际输出的字符个数
 */
int sprintf_ns(char* buf, size_t size, size_t count, const char* fmt, ...) __attribute__((format(printf, 4, 5)));

/**
 *    从格式化输出中解析获取参数，防截断版本
 *    对于%c和%s必须附加参数指定缓冲区长度，长度不足时中止解析。%s会加入'/0'，而%c不会加入
 *    例如： sscanf_ns("%d, %s", &i, buf, 5);
 *    @buf:      输入字符串
 *    @fmt:      格式字符
 *    @...:      结果参数
 *    @return  成功解析的参数个数
 */
int sscanf_ns(const char *buf, const char *fmt, ...);

/**
 *    从格式化输出中解析获取参数
 *    对于%c和%s必须附加参数指定缓冲区长度，长度不足时截断。%s会加入'/0'，而%c不会加入
 *    例如： sscanf_n("%d, %s", &i, buf, 5);
 *    @buf:     输入字符串
 *    @fmt:     格式字符
 *    @...:     结果参数
 *    @return   成功解析的参数个数
 */
int sscanf_n(const char *buf, const char *fmt, ...);

/**
 *    从格式化文件中解析获取参数，不可用于stdin
 *    对于%c和%s必须附加参数指定缓冲区长度，长度不足时截断。%s会加入'/0'，而%c不会加入
 *    例如： fscanf_n(fp, "%d, %s", &i, buf, 5);
 *    @fmt:     格式字符
 *    @...:     结果参数
 *    @return   成功解析的参数个数
 */
int fscanf_n(FILE *stream, const char *fmt, ...);

/**
 * memcpy 的安全版本
 * 标准库的memcpy不能处理dest和src有重叠的内存区域
 * 这个版本可以安全处理任何情况
 *
 *  @dest:    目的地址
 *  @src:     源地址
 *  @return   返回dest地址
 */
void *memcpy_s(void *dest, const void *src, size_t n);

/**
 * strdup 的内存监控版本
 * strdup_s 将使用wns_malloc进行分配，用wns_free释放分配的内存
 *
 */
char *strdup_s(const char *s);

/**
 * strndup 的内存监控版本
 * strndup_s 将使用wns_malloc进行分配，用wns_free释放分配的内存
 *
 */
char *strndup_s(const char *s, size_t n);


/** \} */
#endif /* __LIBWNS_STRUTILS_H__ */
