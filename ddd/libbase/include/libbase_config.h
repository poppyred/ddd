#ifndef __LIBWNS_CONFIG_H__
#define __LIBWNS_CONFIG_H__

//#ifndef SF_ARCH_X86_64
//#define SF_ARCH_X86_64
//#endif

//#ifndef _SF_COMDEF_H /* 兼容以前的库，如果没有引用以前的库定义的类型，则在这里定义 */
//#define _SF_COMDEF_H
#define DEFINE_SF_TYPE
//#endif

#define SF_USE_PORTABLE_ATOMIC

// 不同的cpu架构，必须使用如下指定的宏定义 --应该通过编译时由-D指定
#if defined(SF_ARCH_X86_64)
    #define SMP_CACHE_BYTE	64
#elif defined(SF_ARCH_MIPS_32)
    #define SMP_CACHE_BYTE	32
#else
//#define SF_ARCH_I686
    #define SMP_CACHE_BYTE	32
#endif

#define __wns_arch_cache_aligned __attribute__((__aligned__(SMP_CACHE_BYTE)))

#if defined(SF_ARCH_X86_64) && defined(SF_ARCH_I686)
#define BACKTRACE_DUMP
#endif

#if defined(SF_ARCH_MIPS_32)
#define WORDS_BIGENDIAN
#endif

#endif /* __LIBWNS_CONFIG_H__ */

