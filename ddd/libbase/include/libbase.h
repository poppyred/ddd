#ifndef __LIBWNS_H__
#define __LIBWNS_H__

#include <sys/cdefs.h>
__BEGIN_DECLS

#include "libbase_config.h"
#include "libbase_atomic.h"
#include "libbase_backtrace.h"
#include "libbase_binsearch.h"
#include "libbase_chunk_ring.h"
#include "libbase_tcp_ring.h"
#include "libbase_comm.h"
#include "libbase_crc.h"
#include "libbase_dirutils.h"
#include "libbase_error.h"
#include "libbase_hash.h"
#include "libbase_hashlist.h"
#include "libbase_heap.h"
#include "libbase_i18n.h"
//#include "libbase_xml_i18n.h"
#include "libbase_iniparser.h"
#include "libbase_ip_compatible.h"
#include "libbase_list.h"
//#include "libbase_log.h"
#include "libbase_memory.h"
#include "libbase_mutex.h"
#include "libbase_rbtree.h"
#include "libbase_ring.h"
#include "libbase_stack.h"
#include "libbase_strmstr.h"
#include "libbase_strutils.h"
#include "libbase_type.h"
#include "libbase_daemon.h"
#include "libbase_bmh.h"
#include "libbase_mwm.h"
#include "libbase_str_trim.h"
//#include "libbase_module.h"
#include "libbase_byteorder.h"
#include "libbase_jhash.h"
#include "libbase_compiler.h"
#include "libbase_appname.h"
#include "libbase_list_sort.h"
#include "libbase_coder.h"
#include "libbase_log2.h"
#include "libbase_data_table.h"
#include "libbase_netutil.h"
#include "libbase_utils.h"
//#include "libbase_sigsegv.h"
#include "libbase_daemon.h"

__END_DECLS

/*
 * 禁用的函数 在编译gtest 或  php扩展时需要关闭下面的功能
 * stdio.h 和string.h等标准库必须定义在这个前面
 */

#if !defined(XTEST) && !defined(WNS_IGNORE_FORBIDDEN)

#if !defined(_MSC_VER) && !defined(WNS_USE_SYSTEM)
//#define system system_is_forbidden
#define popen popen_is_forbidden@
#endif

//#define strcpy strcpy_is_forbidden_use_memcpy_instead@
#define sprintf sprintf_is_forbidden_use_snprintf_instead@
#define strcat strcat_is_forbidden@
#define snprintf snprintf_is_forbidden_use_str_snprintf_instead@

#ifdef strncat
#undef strncat
#endif
#define strncat strncat_is_forbidden@

#define fscanf fscanf_is_forbidden_parse_yourself@
#define scanf scanf_is_forbidden_parse_yourself@

#ifdef alloca
#undef alloca
#endif
#define alloca alloca_is_forbidden@

#ifdef strdup
#undef strdup
#endif
#define strdup strdup_is_forbidden_use_strdup_s_instead@

#ifdef strndup
#undef strndup
#endif
#define strndup strndup_is_forbidden_use_strndup_s_instead@

#endif

#if defined(_MSC_VER)
#define inline __inline
#define __attribute__(x)
#endif

#endif /* __LIBWNS_H__ */
