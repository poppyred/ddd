#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>

#include "libbase_strutils.h"
#include "libbase_memory.h"
#include "libbase_type.h"

#define MAX_MATCH_SIZE  (1024)

int str_find(const char *str, unsigned int slen,
        const char *match, unsigned int mlen,
        unsigned int start)
{
    int next[MAX_MATCH_SIZE] = {0};
    int m = 0, i = 0;
    int pos = 2;    /* the current positon we are computing in next */
    int cnd = 0;    /* the index in match of next */

    if (!str || !match || mlen >= MAX_MATCH_SIZE ||
        start >= slen || mlen == 0)
    {
        return -1;
    }

    next[0] = -1;
    next[1] = 0;

    while(pos < mlen) {
        if (match[pos - 1] == match[cnd]) { /* substring continues */
            cnd++;
            next[pos] = cnd;
            pos++;
        } else if(cnd > 0) {                /* call back */
            cnd = next[cnd];
        } else {                            /* run out of candidates */
            next[pos++] = 0;
        }
    }

    str += start;
    slen -= start;

    if (slen < mlen)
        return -1;

    while (m + i < slen) {
        if (match[i] == str[m + i]) {
            if (i == mlen - 1)
                return m + start;
            i++;
            continue;
        }
        m = m + i - next[i];
        if (next[i] > -1)
            i = next[i];
        else
            i = 0;
    }
    return -1;
}

const char *str_nstr(const char *str, unsigned int len,
        const char *match, unsigned int mlen)
{
    int n = str_find(str, len, match, mlen, 0);

    if (n < 0)
        return NULL;

    return str + n;
}

const char *str_nchr(const char *str, unsigned int len, int c)
{
    while (len) {
        if (*str == c)
            return str;

        str++;
        len--;
    }

    return NULL;
}

#ifndef   SCHAR_MIN
#  define SCHAR_MIN     (-128)
#endif
#ifndef   SCHAR_MAX
#  define SCHAR_MAX     127
#endif
#ifndef   UCHAR_MAX
#  define UCHAR_MAX     255
#endif
#ifndef   SHRT_MIN
#  define SHRT_MIN      (-32768)
#endif
#ifndef   SHRT_MAX
#  define SHRT_MAX      32767
#endif
#ifndef   USHRT_MAX
#  define USHRT_MAX     65535
#endif



#ifdef _MSC_VER

// 禁止所有POSIX兼容警告
//#pragma warning(disable: 4996)
// 条件表达式为常量
//#pragma warning(disable: 4127)

//低于VS2008的版本特殊处理，如VC6.0
#  if _MSC_VER < 1500
#  define vsnprintf _vsnprintf
#  endif

typedef __int64          INT64;
typedef unsigned __int64 UINT64;

#else

typedef int64_t  INT64;
typedef uint64_t UINT64;
#endif

#if (_XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L)
#    ifndef ULLONG_MAX
#      define ULLONG_MAX     0xffffffffffffffffui64
#    endif
#    ifndef LLONG_MAX
#      define LLONG_MAX      9223372036854775807i64
#    endif
#else
#    ifndef ULLONG_MAX
#      define ULLONG_MAX     (~(uint64_t)0)
#    endif
#    ifndef LLONG_MAX
#      define LLONG_MAX      ((~(uint64_t)0) >> 1)
#    endif
#endif

/**
 *    字符串连接。时间复杂度O(m + n), m和n分别为目的串和源串长度
 *    strcat_n保证缓冲区内字符串以0结尾，除非size == 0
 *    @dst:     目的字符串地址
 *    @size:    目的字符串缓冲区总长度
 *    @src:     源字符串地址
 *    @return    < 0 时表示缓冲区长度不够用，已作截断处理
 */
int strcat_n(char *dst, size_t size, const char *src)
{
    size_t dst_len;
    size_t src_len;
    size_t rest;

    STRSAFE_ASSERT(dst && src);
    STRSAFE_ASSERT((dst < src && (dst + size) < src)
        || (dst > src && (src + size) < dst));

    dst_len = strlen(dst);
    if(!size || dst_len >= size - 1){
        return -1;
    }

    //由于不同的C库strncat处理空间不足时有不同的行为，这里算出长度后memcpy
    src_len = strlen(src);
    rest = size - dst_len;

    if(rest > src_len){
        memcpy_s(dst + dst_len, src, src_len);
        dst[dst_len + src_len] = '\0';
        return 0;
    }else{
        memcpy_s(dst + dst_len, src, rest - 1);
        dst[size - 1] = '\0';
        return -1;
    }

   //return strcpy_n(dst + dst_len, size - dst_len, src);
}

/**
 *    字符串连接，防截断版本。时间复杂度O(m + 2n), m和n分别为目的串和源串长度
 *    strcat_ns保证缓冲区内字符串以0结尾，除非size == 0
 *    @dst:     目的字符串地址
 *    @size:    目的字符串缓冲区总长度
 *    @src:     源字符串地址
 *    @count:   最多连接count个字符到目的串，count为SANGFOR_TRUNC这个特殊值表示缓冲区长度不足时可以截断
 *    @return    < 0 时表示缓冲区长度不够用，count为SANGFOR_TRUNC时截断，否则目的串不做任何修改
 */
int strcat_ns(char *dst, size_t size, const char *src, size_t count)
{
    size_t dst_len;
    size_t src_len;
    size_t rest;
    size_t copy_len;

    STRSAFE_ASSERT(dst && src);
    STRSAFE_ASSERT((dst < src && (dst + size) < src)
        || (dst > src && (src + size) < dst));

    dst_len = strlen(dst);
    if(!size || dst_len >= size - 1){
        return -1;
    }

    src_len = strlen(src);
    rest = size - dst_len;
    copy_len = count <= src_len ? count : src_len;

    if(count != SANGFOR_TRUNC && copy_len + 1 > rest){
        return -1;
    }

    if(rest > copy_len){
        memcpy_s(dst + dst_len, src, copy_len);
        dst[dst_len + copy_len] = '\0';
        return 0;
    }else{
        memcpy_s(dst + dst_len, src, rest - 1);
        dst[size - 1] = '\0';
        return -1;
    }
}


/**
 *    字符串拷贝
 *    strcpy_n保证缓冲区内字符串以0结尾，除非 size == 0
 *    @dst:     目的字符串地址
 *    @size:    目的字符串缓冲区总长度
 *    @src:     源字符串地址
 *    @return    < 0 时表示缓冲区长度不够用，已作截断处理
 */
int strcpy_n(char *dst, size_t size, const char *src)
{
    size_t copy_len;

    STRSAFE_ASSERT(dst && src);
    STRSAFE_ASSERT((dst < src && (dst + size) < src)
        || (dst > src && (src + size) < dst));

    if(!size){
        return -1;
    }

/*    从测试结果看，strlen + memcpy性能更好，特别是32bit系统
    dst[size - 1] = '\0';
    strncpy(dst, src, size);

    //无论是glibc还是VC，当src长度大于dst_len时不会在末尾加'\0'
    if(dst[size - 1] != '\0'){
        dst[size - 1] = '\0';
        return -1;
    }
 */
    copy_len = strlen(src);

    if(copy_len < size){
        memcpy_s(dst, src, copy_len);
        dst[copy_len] = '\0';
        return 0;
    }else{
        memcpy_s(dst, src, size - 1);
        dst[size - 1] = '\0';
        return -1;
    }

}

/**
 *    字符串拷贝，防截断版本
 *    strcpy_ns保证缓冲区内字符串以0结尾，除非 size == 0
 *    @dst:     目的字符串地址
 *    @size:    目的字符串缓冲区总长度
 *    @src:     源字符串地址
 *    @count:   最多拷贝count个字符到目的串，count为SANGFOR_TRUNC这个特殊值表示缓冲区长度不足时可以截断
 *    @return    < 0 时表示缓冲区长度不够用，count为SANGFOR_TRUNC时截断，否则目的缓冲区不做任何修改
 */
int strcpy_ns(char *dst, size_t size, const char *src, size_t count)
{
    size_t src_len;
    size_t copy_len;

    STRSAFE_ASSERT(dst && src);
    STRSAFE_ASSERT((dst < src && (dst + size) < src)
        || (dst > src && (src + size) < dst));

    if(!size){
        return -1;
    }

    src_len = strlen(src);
    copy_len = count <= src_len ? count : src_len;

    if(count != SANGFOR_TRUNC && copy_len + 1 > size){
        return -1;
    }

    if(copy_len < size){
        memcpy_s(dst, src, copy_len);
        dst[copy_len] = '\0';
        return 0;
    }else{
        memcpy_s(dst, src, size - 1);
        dst[size - 1] = '\0';
        return -1;
    }
}


/**
 *    格式化输出到字符串缓冲区
 *    vsprintf_n保证缓冲区内字符串以0结尾，除非 size == 0
 *    @buf:     输出缓冲区地址
 *    @size:    输出缓冲区总长度
 *    @fmt:     输出格式控制字符串
 *    @ap:      可变参数列表
 *    @return    < 0 时表示缓冲区长度不够用，已截断处理；否则返回实际输出的字符个数
 */
int vsprintf_n(char* buf, size_t size, const char* fmt, va_list ap)
{
    int ret = vsnprintf(buf, size, fmt, ap);
    /*
    * requried_len < size 时，glibc和VC均返回requried_len
    * requried_len = size 时，glibc和VC均返回requried_len，
    *        但glibc会截取最后一个字符并补null，而VC不会截断也不补null
    * requried_len > bufSize时，glibc依然返回requried_len，截断补null; VC返回负值，不补null
    */
    if (ret < 0 || ret >= (int)size){
        if (size == 0){
            return -1;
        }
        buf[size - 1] = '\0';
        return ret > 0 ? -ret : ret;
    }
    return ret;
}

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
int vsprintf_ns(char* buf, size_t size, size_t count,
        const char* fmt, va_list ap)
{
    size_t use_len;
    int ret;

    if(count == SANGFOR_TRUNC){
        return vsprintf_n(buf, size, fmt, ap);
    }
    use_len = count <= size ? count : size;
    ret = vsnprintf(buf, use_len, fmt, ap);
    /*
    * requried_len < use_len 时，glibc和VC均返回requried_len
    * requried_len = use_len 时，glibc和VC均返回requried_len，
    *        但glibc会截取最后一个字符并补null，而VC不会截断也不补null
    * requried_len > bufSize时，glibc依然返回requried_len，截断补null; VC返回负值，不补null
    */
    if (ret < 0 || ret >= (int)use_len){
        if (size == 0){
            return -1;
        }
        buf[0] = '\0';
        return -1;
    }
    return ret;
}

/**
 *    格式化输出到字符串缓冲区
 *    sprintf_n保证缓冲区内字符串以0结尾，除非 size == 0
 *    @buf:     输出缓冲区地址
 *    @size:    输出缓冲区总长度
 *    @fmt:     输出格式控制字符串
 *    @...:     可变参数
 *    @return    < 0 时表示缓冲区长度不够用，已截断处理；否则返回实际输出的字符个数
 */
int sprintf_n(char* buf, size_t size, const char* fmt, ...)
{
    va_list ap;
    int ret;

    STRSAFE_ASSERT(buf);
    STRSAFE_ASSERT(fmt);

    va_start(ap, fmt);
    ret = vsprintf_n(buf, size, fmt, ap);
    va_end(ap);
    return ret;
}

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
int sprintf_ns(char* buf, size_t size, size_t count, const char* fmt, ...)
{
    va_list ap;
    int ret;

    STRSAFE_ASSERT(buf);
    STRSAFE_ASSERT(fmt);

    va_start(ap, fmt);
    ret = vsprintf_ns(buf, size, count, fmt, ap);
    va_end(ap);
    return ret;
}



//////////////////////////////////////////////////////////
//   以下l_前缀的函数仅供内部使用
//////////////////////////////////////////////////////////
static const char *l_skip_spaces(const char *str)
{
    while (isspace(*str))
        ++str;
    return str;
}

static int l_skip_atoi(const char **s)
{
    int i = 0;

    while (isdigit(**s))
        i = i*10 + *((*s)++) - '0';

    return i;
}

static inline char l_tolower(const char c)
{
    return (char)(c | 0x20);
}

static unsigned int l_simple_guess_base(const char *cp)
{
    if (cp[0] == '0') {
        if (l_tolower(cp[1]) == 'x' && isxdigit(cp[2]))
            return 16;
        else
            return 8;
    } else {
        return 10;
    }
}


static UINT64 l_simple_strtoull(const char *cp, const char **endp,
        unsigned int base, int *poverflow)
{
    UINT64 result = 0;
    UINT64 V;
    UINT64 R;

#ifndef __KERNEL__
    V = ULLONG_MAX / base;
    R = ULLONG_MAX % base;
#else
    V = ULLONG_MAX;
    R = do_div(V, base);
#endif

    if (!base)
        base = l_simple_guess_base(cp);

    if (base == 16 && cp[0] == '0' && l_tolower(cp[1]) == 'x')
        cp += 2;

    while (isxdigit(*cp)) {
        unsigned int value;

        value = isdigit(*cp) ? *cp - '0' : l_tolower(*cp) - 'a' + 10;
        if (value >= base)
            break;
        if(result > V || (result == V && value > R)){
            *poverflow = 1;
            return 0;
        }
        result = result * base + value;
        cp++;
    }
    if (endp)
        *endp = cp;

    return result;
}


static unsigned long l_simple_strtoul(const char *cp, const char **endp,
        unsigned int base, int *poverflow)
{
    UINT64 ret = l_simple_strtoull(cp, endp, base, poverflow);
    if(*poverflow || !endp){
        return 0;
    }

    // deleted by adam - 2012-10-26 ----------------------------------------
//    if(ret > ULONG_MAX){
//        *poverflow = 1;
//        *endp = cp;
//        return 0;
//    }
    // end of deleted by adam  ---------------------------------------------

    return (unsigned long)ret;
}


static long l_simple_strtol(const char *cp, const char **endp,
        unsigned int base, int *poverflow)
{
    unsigned long ret;
    int sign = 1;

    if (*cp == '-'){
        cp++;
        sign = -1;
    }

    ret = l_simple_strtoul(cp, endp, base, poverflow);

    if(*poverflow){
        return 0;
    }

    if(ret > LONG_MAX){
        *poverflow = 1;
        *endp = cp;
        return 0;
    }

    return ((long)ret) * sign;
}

static INT64 l_simple_strtoll(const char *cp, const char **endp,
        unsigned int base, int *poverflow)
{
    UINT64 ret;
    int sign = 1;

    if (*cp == '-'){
        cp++;
        sign = -1;
    }

    ret = l_simple_strtoull(cp, endp, base, poverflow);

    if(*poverflow || !endp){
        return 0;
    }

    if(ret > LLONG_MAX){
        *poverflow = 1;
        *endp = cp;
        return 0;
    }

    return ((INT64)ret) * sign;
}


static int l_vsscanf_ns(size_t trunc, const char *buf, const char **endp,
        const char *fmt, va_list args)
{
    const char *str = buf;
    const char *next;
    char digit;
    char is_sign;
    char qualifier;
    char base;
    int field_width;
    int num = 0;

    while (*fmt && *str) {
        /* skip any white space in format */
        /* white space in format matchs any amount of
         * white space, including none, in the input.
         */
        if (isspace(*fmt)) {
            fmt = l_skip_spaces(++fmt);
            str = l_skip_spaces(str);
        }

        /* anything that is not a conversion must match exactly */
        if (*fmt != '%' && *fmt) {
            if (*fmt++ != *str++)
                break;
            continue;
        }

        if (!*fmt)
            break;
        ++fmt;

        /* skip this conversion.
         * advance both strings to next white space
         */
        if (*fmt == '*') {
            while (!isspace(*fmt) && *fmt != '%' && *fmt)
                fmt++;
            while (!isspace(*str) && *str)
                str++;
            continue;
        }

        /* get field width */
        field_width = -1;
        if (isdigit(*fmt))
            field_width = l_skip_atoi(&fmt);

        /* get conversion qualifier */
        qualifier = -1;
        if (*fmt == 'h' || l_tolower(*fmt) == 'l' ||
            l_tolower(*fmt) == 'z') {
            qualifier = *fmt++;
            if (unlikely(qualifier == *fmt)) {
                if (qualifier == 'h') {
                    qualifier = 'H';
                    fmt++;
                } else if (qualifier == 'l') {
                    qualifier = 'L';
                    fmt++;
                }
            }
        }

        if (!*fmt || !*str)
            break;

        base = 10;
        is_sign = 0;

        switch (*fmt++) {
        case 'c':
            {
                char *s = (char *)va_arg(args, char*);
                size_t size = (size_t)va_arg(args, size_t);
                size_t copy_limit = size;
                if (field_width != -1 && (size_t)field_width < size)
                    copy_limit = field_width;
                if(!trunc){
                    size_t i;
                    for(i = 0; i < copy_limit && str[i]; i++) {
                        // do nothing
                    }
                    if(str[i])
                        goto RETURN;
                }
                do {
                    *s++ = *str++;
                } while (--copy_limit > 0 && *str);

                num++;
            }
            continue;
        case 's':
            {
                char *s = (char *)va_arg(args, char *);
                size_t size = (size_t)va_arg(args, size_t);
                size_t copy_limit = size - 1;
                if (field_width != -1 && (size_t)field_width < size)
                    copy_limit = field_width;
                if(!trunc){
                    size_t i;
                    for(i = 0; i < copy_limit && str[i]; i++) {
                        // do nothing
                    }
                    if(str[i])
                        goto RETURN;
                }
                /* first, skip leading white space in buffer */
                str = l_skip_spaces(str);

                /* now copy until next white space */
                while (*str && !isspace(*str) && copy_limit--)
                    *s++ = *str++;
                *s = '\0';
                num++;
            }
            continue;
        case 'n':
            /* return number of characters read so far */
            {
                int *i = (int *)va_arg(args, int*);
                *i = str - buf;
            }
            continue;
        case 'o':
            base = 8;
            break;
        case 'x':
        case 'X':
            base = 16;
            break;
        case 'i':
            base = 0;
        case 'd':
            is_sign = 1;
        case 'u':
            break;
        case '%':
            /* looking for '%' in str */
            if (*str++ != '%')
                goto RETURN;
            continue;
        default:
            /* invalid format; stop here */
            goto RETURN;
        }

        /* have some sort of integer conversion.
         * first, skip white space in buffer.
         */
        str = l_skip_spaces(str);

        digit = *str;
        if (is_sign && digit == '-')
            digit = *(str + 1);

        if (!digit
            || (base == 16 && !isxdigit(digit))
            || (base == 10 && !isdigit(digit))
            || (base == 8 && (!isdigit(digit) || digit > '7'))
            || (base == 0 && !isdigit(digit)))
            break;

        switch (qualifier) {
        case 'H':    /* that's 'hh' in format */
            if (is_sign) {
                signed char *s = (signed char *)va_arg(args, signed char *);
                int overflow = 0;
                long result = l_simple_strtol(str, &next, base, &overflow);
                if(overflow || result < SCHAR_MIN ||result > SCHAR_MAX){
                    goto RETURN;
                }
                *s = (signed char)result;
            } else {
                unsigned char *s = (unsigned char *)va_arg(args,
                        unsigned char *);
                int overflow = 0;
                unsigned long result = l_simple_strtoul(str, &next,
                        base, &overflow);
                if(overflow || result > UCHAR_MAX){
                    goto RETURN;
                }
                *s = (unsigned char)result;
            }
            break;

        case 'h':
            if (is_sign) {
                short *s = (short *)va_arg(args, short *);
                int overflow = 0;
                long result = l_simple_strtol(str, &next, base, &overflow);
                if(overflow || result < SHRT_MIN ||result > SHRT_MAX){
                    goto RETURN;
                }
                *s = (short)result;
            } else {
                unsigned short *s = (unsigned short *)va_arg(args,
                        unsigned short *);
                int overflow = 0;
                unsigned long result = l_simple_strtoul(str, &next,
                        base, &overflow);
                if(overflow || result > USHRT_MAX){
                    goto RETURN;
                }
                *s = (unsigned short)result;
            }
            break;
        case 'l':
            if (is_sign) {
                long *l = (long *)va_arg(args, long *);
                int overflow = 0;
                long result = l_simple_strtol(str, &next, base, &overflow);
                if(overflow){
                    goto RETURN;
                }
                *l = result;
            } else {
                unsigned long *l = (unsigned long *)va_arg(args,
                        unsigned long *);
                int overflow = 0;
                unsigned long result = l_simple_strtoul(str, &next,
                        base, &overflow);
                if(overflow){
                    goto RETURN;
                }
                *l = result;
            }
            break;
        case 'L':
            if (is_sign) {
                INT64 *l = (INT64 *)va_arg(args, INT64 *);
                int overflow = 0;
                INT64 result = l_simple_strtoll(str, &next, base, &overflow);
                if(overflow){
                    goto RETURN;
                }
                *l = result;
            } else {
                UINT64 *l = (UINT64 *)va_arg(args, UINT64 *);
                int overflow = 0;
                UINT64 result = l_simple_strtoull(str, &next, base, &overflow);
                if(overflow){
                    goto RETURN;
                }
                *l = result;
            }
            break;
        case 'Z':
        case 'z':            //此处假定size_t为unsigned long类型
            {
                size_t *s = (size_t *)va_arg(args, size_t *);
                int overflow = 0;
                unsigned long result = l_simple_strtoul(str, &next,
                        base, &overflow);
                if(overflow){
                    goto RETURN;
                }
                *s = (size_t)result;
            }
            break;
        default:
            if (is_sign) {
                int *i = (int *)va_arg(args, int *);
                int overflow = 0;
                long result = l_simple_strtol(str, &next, base, &overflow);
                if(overflow || result < INT_MIN || result > INT_MAX){
                    goto RETURN;
                }
                *i = (int)result;
            } else {
                unsigned int *i = (unsigned int *)va_arg(args, unsigned int*);
                int overflow = 0;
                unsigned long result = l_simple_strtoul(str, &next,
                        base, &overflow);
                if(overflow || result > UINT_MAX){
                    goto RETURN;
                }
                *i = (unsigned int)result;
            }
            break;
        }
        num++;

        if (!next)
            break;
        str = next;
    }

    /*
     * Now we've come all the way through so either the input string or the
     * format ended. In the former case, there can be a %n at the current
     * position in the format that needs to be filled.
     */
    if (*fmt == '%' && *(fmt + 1) == 'n') {
        int *p = (int *)va_arg(args, int *);
        *p = str - buf;
    }
RETURN:
    *endp = str;
    return num;
}


/**
 *    从格式化输出中解析获取参数，防截断版本
 *    对于%c和%s必须附加参数指定缓冲区长度，长度不足时中止解析。%s会加入'/0'，而%c不会加入
 *    例如： sscanf_ns("%d, %s", &i, buf, 5);
 *    @buf:      输入字符串
 *    @fmt:      格式字符
 *    @...:      结果参数
 *    @return  成功解析的参数个数
 */
int sscanf_ns(const char *buf, const char *fmt, ...)
{
    va_list args;
    int i;
    const char *end;

    va_start(args, fmt);
    i = l_vsscanf_ns(0, buf, &end, fmt, args);
    va_end(args);

    return i;
}

/**
 *    从格式化输出中解析获取参数
 *    对于%c和%s必须附加参数指定缓冲区长度，长度不足时截断。%s会加入'/0'，而%c不会加入
 *    例如： sscanf_n("%d, %s", &i, buf, 5);
 *    @buf:     输入字符串
 *    @fmt:     格式字符
 *    @...:     结果参数
 *    @return   成功解析的参数个数
 */
int sscanf_n(const char *buf, const char *fmt, ...)
{
    va_list args;
    int i;
    const char *end;

    va_start(args, fmt);
    i = l_vsscanf_ns(SANGFOR_TRUNC, buf, &end, fmt, args);
    va_end(args);

    return i;
}
void *memcpy_s(void *dest, const void *src, size_t n)
{
    return memmove(dest, src, n);
}

char *strdup_s(const char *s)
{
    size_t len;
    char *buf;

    if (!s)
        return NULL;

    len = strlen(s) + 1;
    buf = h_malloc(len);
    if (buf)
        memcpy(buf, s, len);//modi by lwj:不可能有重叠
    return buf;
}

#if (_XOPEN_SOURCE < 700 && _POSIX_C_SOURCE < 200809L)
static size_t strnlen(const char *s, size_t count)
{
    const char *sc;

    for (sc = s; count-- && *sc != '\0'; ++sc) {
        // do nothing
    }
    return sc - s;
}
#endif

char *strndup_s(const char *s, size_t max)
{
    size_t len;
    char *buf;

    if (!s)
        return NULL;

    len = strnlen(s, max);
    buf = h_malloc(len + 1);
    if (buf) {
        memcpy(buf, s, len);
        buf[len] = '\0';
    }
    return buf;
}

#ifndef __KERNEL__

#define FSCAN_BUF_UNIT    200

/**
 *    从格式化文件中解析获取参数，不可用于stdin
 *    对于%c和%s必须附加参数指定缓冲区长度，长度不足时截断。%s会加入'/0'，而%c不会加入
 *    例如： fscanf_n(fp, "%d, %s", &i, buf, 5);
 *    @fmt:     格式字符
 *    @...:     结果参数
 *    @return   成功解析的参数个数
 */
int fscanf_n(FILE *stream, const char *fmt, ...)
{
    va_list args;
    int num;
    int k = 0;
    char *buf;
    const char *end;
    int rest_data;
    int buf_len;
    int data_len;

TRY_AGAIN:
    k++;
    num = 0;
    buf_len = k * FSCAN_BUF_UNIT;
    buf = (char *)h_malloc(buf_len);
    if(!buf)
        goto RETURN;
    data_len = fread(buf, 1, buf_len - 1, stream);
    if(!data_len)
        goto FREE;
    buf[data_len] = '\0';

    va_start(args, fmt);
    num = l_vsscanf_ns(SANGFOR_TRUNC, buf, &end, fmt, args);
    va_end(args);

    rest_data = data_len - (end - buf);
    if(rest_data){
        fseek(stream, -rest_data, SEEK_CUR);
        printf("log %d: rest_data = %d k = %d \n", __LINE__, rest_data, k);
    }else if(k < 6){    //有可能是buf中缓存的字符串长度不足
        if(feof(stream)){
            printf("log %d: rest_data = %d k = %d Get EOF. \n", __LINE__, rest_data, k);
            goto FREE;
        }
        fseek(stream, -data_len, SEEK_CUR);
        h_free(buf);
        printf("log %d: rest_data = %d k = %d \n", __LINE__, rest_data, k);
        goto TRY_AGAIN;
    }
FREE:
    h_free(buf);
RETURN:
    return num;
}


#endif


#ifdef  STRSAFE_TEST

#ifdef _MSC_VER
// 禁止所有POSIX兼容警告
//#pragma warning(disable: 4996)

// 条件表达式为常量
//#pragma warning(disable: 4127)

#endif

#include <stdio.h>
#include "strsafe.h"

#define BUF_LEN  64
#define TEST_FILE    "test.txt"

char buf[BUF_LEN];
const char *src = "1234567890";

int main()
{
    int ret;

    printf("limit: \n LLONG_MAX: %lld \n ULLONG_MAX: %llu \n",
            LLONG_MAX, ULLONG_MAX);

    //从字符串中解析获取参数
    INT64 i64 = 0;
    UINT64 u64 = 0;
    strcpy_n(buf, BUF_LEN, "1111111111111111111111111111111111");
    ret = sscanf_n(buf, "%lld", &i64, sizeof(i64));
    printf("Line %d: i64 = %lld, ret = %d \n", __LINE__, i64, ret);

    ret = sscanf_n(buf, "%lld", &u64, sizeof(u64));
    printf("Line %d: u64 = %llu, ret = %d \n", __LINE__, u64, ret);

    sprintf_n(buf, BUF_LEN, "%lld", LLONG_MAX);
    ret = sscanf_n(buf, "%lld", &i64, sizeof(i64));
    printf("Line %d: i64 = %lld, ret = %d \n", __LINE__, i64, ret);

    sprintf_n(buf, BUF_LEN, "%llu", ULLONG_MAX);
    ret = sscanf_n(buf, "%Lu", &u64, sizeof(u64));
    printf("Line %d: i64 = %llu, ret = %d \n", __LINE__, u64, ret);


    //字符串拷贝
    ret = strcpy_n(buf, 5, src);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);

    ret = strcpy_ns(buf, 5, src, 3);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);

    ret = strcpy_ns(buf, 5, src, 30);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);

    ret = strcpy_ns(buf, 5, src, SANGFOR_TRUNC);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);

    //字符串连接
    ret = strcat_n(buf, BUF_LEN, src);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);

    buf[BUF_LEN - 4] = '\0';
    ret = strcat_ns(buf, BUF_LEN, src, 3);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);
    buf[BUF_LEN - 4] = '\0';
    ret = strcat_ns(buf, BUF_LEN, src, 4);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);
    buf[BUF_LEN - 4] = '\0';
    ret = strcat_ns(buf, BUF_LEN, src, SANGFOR_TRUNC);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);

    //字符串格式化输出
    ret = sprintf_n(buf, BUF_LEN, "The formatted string is :%s \n", src);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);
    ret = sprintf_ns(buf, BUF_LEN, 5, "The formatted string is :%s \n", src);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);
    ret = sprintf_ns(buf, BUF_LEN, SANGFOR_TRUNC,
            "The formatted string is :%s \n", src);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);

    //从字符串中解析获取参数
    strcpy_n(buf, 5, "xxxxxxxxxx");
    ret = sscanf_n(src, "%5c", buf, 3);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);
    strcpy_n(buf, 5, "xxxxxxxxxx");
    ret = sscanf_n(src, "%5s", buf, 3);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);

    strcpy_n(buf, 5, "xxxxxxxxxx");
    ret = sscanf_ns(src, "%5c", buf, 3);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);
    strcpy_n(buf, 5, "xxxxxxxxxx");
    ret = sscanf_ns(src, "%5s", buf, 3);
    printf("Line %d: buf = %s, ret = %d \n", __LINE__, buf, ret);


    //从文件解析字符参数
    FILE *fp = fopen(TEST_FILE, "w");
    if(!fp){
        printf("Line %d: open file %s failed! \n", __LINE__, TEST_FILE);
        return -1;
    }
    fprintf(fp, "123456789011121314");
    fclose(fp);

    char str[3] = {0};
    fp = fopen(TEST_FILE, "r");
    if(!fp){
        printf("Line %d: open file %s failed! \n", __LINE__, TEST_FILE);
        return -1;
    }
    ret = fscanf_n(fp, "%s", str, 3);
    printf("Line %d: str = %s, ret = %d \n", __LINE__, str, ret);
    ret = fscanf_n(fp, "%s", str, 3);
    printf("Line %d: str = %s, ret = %d \n", __LINE__, str, ret);
    fclose(fp);

    return 0;
}



#endif












