#ifndef __LIBWNS_COMPILER__H__
#define __LIBWNS_COMPILER__H__

#include "libbase_config.h"
/**
 * 不导出符号。 如：动态库由多个.c文件组成，各.c提供内部接口，但如果用static由会导致内部找不到符号
 * 如果不用static由会把该函数符号导出，可能导致使用该动态库的程序符号冲突
 * 使用方法：在.h声明函数前加上，即：
 * LOCAL_API void xxx_func();
 */
#if __GNUC__ >= 4

#define LOCAL_API  __attribute__ ((visibility("hidden")))

#else

#define LOCAL_API

#endif

#ifdef __cplusplus

#define EXPORT_API extern "C" 

#else

#define EXPORT_API extern 

#endif



#endif /* __LIBWNS__COMPILER__H__ */

