/*!
 * \file libbase_appname.h
 * \brief 通过/proc/%d/status获取应用程序执行时的名称
 * mips架构下没有定义__progname这个变量, 所以通过这个方式获取应用程序名
 *
 * \author Adam Xiao (iefcu), iefcuxy@gmail.com
 * \date 2012-11-07 08:53:30
 */

#ifndef _LIBWNS_APPNAME_H_
#define _LIBWNS_APPNAME_H_

#include <sys/types.h>

__BEGIN_DECLS

/**
 * 获取当前程序的appname
 */
const char* wns_current_appname(void);

__END_DECLS

#endif /* end of include guard: _LIBWNS_APPNAME_ */
