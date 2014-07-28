#ifndef __LIBWNS_COMM_H__
#define __LIBWNS_COMM_H__

/**
 * Check if a branch is likely to be taken.
 *
 * This gcc builtin allows the developer to indicate if a branch is
 * likely to be taken. Example:
 *
 *   if (likely(x > 1))
 *      do_stuff();
 *
 */
#ifndef likely
# define likely(x)  __builtin_expect(!!(x), 1)
#endif

/**
 * Check if a branch is unlikely to be taken.
 *
 * This gcc builtin allows the developer to indicate if a branch is
 * unlikely to be taken. Example:
 *
 *   if (unlikely(x < 1))
 *      do_stuff();
 *
 */
#ifndef unlikely
# define unlikely(x)    __builtin_expect(!!(x), 0)
#endif

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
/**
 * 通过成员，找结构体指针
 * container_of - cast a member of a structure out to the containing structure
 *
 * @ptr:    the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({            \
		const typeof(((type *)0)->member)*__mptr = (ptr);    \
		(type *)((char *)__mptr - offsetof(type, member)); })
#endif

#define wns_nop() do{}while(0)

#if 0
#define wns_pause() do{}while(0)
#else
#define wns_pause() wns_yield()
#endif

/* 线程放弃cpu，调度 */
#include <sched.h>
#define wns_yield() sched_yield()

/* true if x is a power of 2 */
#define POWEROF2(x) ((((x)-1) & (x)) == 0)

#define MAX_SIGNED_32INT (0x7fffffff)
#define MAX_UNSIGNED_32INT (0xffffffff)

#define WNS_MIN(a,b) (((a)<(b))?(a):(b))
#define WNS_MAX(a,b) (((a)>(b))?(a):(b))

#define WNS_SWAP(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)


#define PACK_4 __attribute((aligned (4)))
#define PACK_1 __attribute__ ((packed))

#endif /* __LIBWNS_COMM_H__ */
