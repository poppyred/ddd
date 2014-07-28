/**
 * \file backtrace.h
 * 堆栈回朔
 * \author hyb
 * \defgroup base 基础函数库
 * \{
 */
#ifndef __LIBWNS_BACKTRACE_H__
#define __LIBWNS_BACKTRACE_H__

/**
 * 将当前调用堆栈信息写入文件，调试接口
 * \param fd 被写打开的文件描述符
 * \par 示例
 * \code
	backtrace_dump(2);		//将调用堆栈写至标准错误
 * \endcode
 */
void h_backtrace_dump(int fd);

/** \} */
#endif /* __LIBWNS_BACKTRACE_H__ */
