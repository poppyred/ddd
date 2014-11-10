//	$Id: log_util.h 2013-05-23 likunxiang$
//

#ifndef _LOG_UTIL_H
#define _LOG_UTIL_H
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>	/* PRI* macros */
#include <unistd.h>	/* close */
#include <sys/socket.h>	/* sockaddr.. */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>	/* timersub */

//#include <sys/ioctl.h>	/* ioctl */
//#include <errno.h>
//#include <string.h>	/* strcmp */
//#include <fcntl.h>	/* open */
//#include <ifaddrs.h>	/* getifaddrs */
//#include <sys/mman.h>	/* PROT_* */
//#include <sys/poll.h>
//#include <sys/param.h>
//#include <sys/sysctl.h>	/* sysctl */
//
//#include <net/ethernet.h>
//#include <net/if.h>	/* ifreq */

#define log_likely(x)       __builtin_expect(!!(x), 1)
#define log_unlikely(x)       __builtin_expect(!!(x), 0)

//#if defined(__x86_64__) || defined(_M_X64)
#if (__SIZEOF_POINTER__ == 4)
#pragma message("32 macro activated!")
#define LU "llu"
#else
#pragma message("64 macro activated!")
#define LU "lu"
#endif

#define SAFE_DELETE(p)       { if(p) { free(p);     (p)=NULL; } }
#define MAX_PATH            256
#define BUFSIZE             4096
#define MAX_BUFF_LEN        8192
#define MAX_FILENAME        256
#define MAX_NAME_LEN        50
#define INVALID_SOCKET (-1)

// XXX does it work on 32-bit machines ?
inline long log_u_engine_sleep(long time);
inline char* log_u_strlcpy( char* dest, const char* src, size_t n );
int log_u_get_exec_path( char* path );
int log_u_create_dir(const char *sPathName);
int log_u_my_atoi(const char* str, int radix);
int log_u_splite_str(char *str, int str_size, char *buf, int buf_size, char **pp, int pp_size, char delimit);

#endif /* _NM_UTIL_H */
