/******************************************************************************
 * @author:     hyb
 * @date:       2011-03-05
 * @brief:      IPv4/IPv6 compatible interfaces.
 ******************************************************************************/

#ifndef __LIBWNS_IP_COMPATIBLE_H__
#define __LIBWNS_IP_COMPATIBLE_H__

#ifdef __KERNEL__ /* KERNEL COMPILE */

#include <linux/in.h>
#include <linux/in6.h>
#include <linux/types.h>
#include <linux/module.h>
#include <net/ipv6.h>

#else  /* APPLICATION COMPILE */

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "libbase_config.h"

typedef uint16_t __be16;
typedef uint32_t __be32;

#define __force

/**
 * ffz - find first zero bit in word
 * @word: The word to search
 *
 * Undefined if no zero exists, so code should check against ~0UL first.
 */
#if defined(SF_ARCH_I686) || defined(SF_ARCH_X86_64)
#ifdef SF_ARCH_I686
static inline unsigned long ffz(unsigned long word)
{
    asm("bsf %1,%0"
        : "=r" (word)
        : "r" (~word));
    return word;
}
#endif
#ifdef SF_ARCH_X86_64
# define ia64_popcnt(x)     __builtin_popcountl(x)
static inline unsigned long
ffz (unsigned long x)
{
    unsigned long result;

    result = ia64_popcnt(x & (~x - 1));
    return result;
}
#endif
#else
static inline unsigned long __ffs(unsigned long word)
{
    int num = 0;

    if (sizeof(word) == 8) {
        if ((word & 0xffffffff) == 0) {
            num += 32;
            word >>= num; /* 32, for no warm on 32bit word */
        }
    }
    if ((word & 0xffff) == 0) {
        num += 16;
        word >>= 16;
    }
    if ((word & 0xff) == 0) {
        num += 8;
        word >>= 8;
    }
    if ((word & 0xf) == 0) {
        num += 4;
        word >>= 4;
    }
    if ((word & 0x3) == 0) {
        num += 2;
        word >>= 2;
    }
    if ((word & 0x1) == 0)
        num += 1;
    return num;
}
#define ffz(x)  __ffs(~(x))
#endif


#endif /* END COMPILE COMPATIBLE */

#define HW_MACADDR_LEN          6

#define __ARRAY_SIZE(array) (sizeof((array))/sizeof((array)[0]))

#define MAX_IPV4_TEXTBUF_LEN    sizeof("255.255.255.255")

#define MAX_IPV6_TEXTBUF_LEN \
    sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")

#define MAX_RANGE_STRING_BUFF   128

#define MAX_IP_AND_MASK_LEN \
    (sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255/ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"))


#ifdef __cplusplus
extern "C"
{
#endif

#define __JHASH_GOLDEN_RATIO    0x9e3779b9

#define l3_jhash_mix(a, b, c) \
    { \
        a -= b; a -= c; a ^= (c>>13); \
        b -= c; b -= a; b ^= (a<<8); \
        c -= a; c -= b; c ^= (b>>13); \
        a -= b; a -= c; a ^= (c>>12);  \
        b -= c; b -= a; b ^= (a<<16); \
        c -= a; c -= b; c ^= (b>>5); \
        a -= b; a -= c; a ^= (c>>3);  \
        b -= c; b -= a; b ^= (a<<10); \
        c -= a; c -= b; c ^= (b>>15); \
    }

enum
{
    L3_IPV4_TYPE = AF_INET,
    L3_IPV6_TYPE = AF_INET6,
    L3_UNKNOWN_TYPE
};

/* Layer 3 地址格式定义
 */
union __l3_addr
{
    uint32_t        all[4];
    __be32          ip6[4];

    struct in6_addr in6;
};

struct l3_addr_st
{
    union __l3_addr addr;
};


#define addr_all addr.all
#define addr_ip  addr.ip6[3]
#define addr_ip6 addr.ip6

#define addr_in  addr.in6.s6_addr32[3]
#define addr_in6 addr.in6


//将ipv6地址格式化输出到MYSQL。 wzg
#ifndef NIPV6TOMYSQL
#define NIPV6TOMYSQL(ip)                \
    ntohl(ip.addr_all[0]),              \
    ntohl(ip.addr_all[1]),              \
    ntohl(ip.addr_all[2]),              \
    ntohl(ip.addr_all[3])
#endif


/* Layer 3地址范围结构定义
 *      本结构描述一个IP范围闭区间 "[,]"
*/
struct l3_range_st
{
    struct l3_addr_st start;
    struct l3_addr_st end;
};

/* MAC地址格式定义
*/
struct mac_addr_st
{
    uint8_t         ha[HW_MACADDR_LEN];
};


/* Bob Jenkins's哈希算法,v2.
*/
static __inline uint32_t
l3_jhash2(const uint32_t *k, uint32_t length, uint32_t initval)
{
    uint32_t a, b, c, len;

    a = b = __JHASH_GOLDEN_RATIO;
    c = initval;
    len = length;

    while (len >= 3)
    {
            a += k[0];
            b += k[1];
            c += k[2];
            l3_jhash_mix(a, b, c);
            k += 3;
            len -= 3;
    }

    c += length * 4;

    switch (len)
    {
    case 2 : b += k[1];
            /* fall through */
    case 1 : a += k[0];
            /* fall through */
    };

    l3_jhash_mix(a,b,c);

    return c;
}


/*
* A special ultra-optimized versions that knows they are hashing exactly
* 3, 2 or 1 word(s).
*/
static __inline uint32_t
l3_jhash_3words(uint32_t a, uint32_t b, uint32_t c, uint32_t initval)
{
    a += __JHASH_GOLDEN_RATIO;
    b += __JHASH_GOLDEN_RATIO;
    c += initval;

    l3_jhash_mix(a, b, c);

    return c;
}


static __inline uint32_t
l3_jhash_2words(uint32_t a, uint32_t b, uint32_t initval)
{
    return l3_jhash_3words(a, b, 0, initval);
}


static __inline uint32_t
l3_jhash_1word(uint32_t a, uint32_t initval)
{
    return l3_jhash_3words(a, 0, 0, initval);
}


#define L3_IPV4_MASK        (0xFFFF0000)
#define L3_IPV4_MARK        (0xFFBF0000)

/* 功能：获得一个Layer 3地址对象的类型
 * 参数：
 *      @l3_addr: Layer 3地址对象的地址
 * 返回值：
 *      L3_IPV4_TYPE - IPv4
 *      L3_IPV6_TYPE - IPv6
 * 注意: 由于rfc3513规定，高16位为0xFFBF0000的地址为非法地址，
 *       现采用这种方法来标记ipv4地址和掩码


    An IPv6 multicast address is an identifier for a group of interfaces
    (typically on different nodes).  An interface may belong to any
    number of multicast groups.  Multicast addresses have the following
    format:

    |   8    |  4 |  4 |                  112 bits                   |
    +------ -+----+----+---------------------------------------------+
    |11111111|flgs|scop|                  group ID                   |
    +--------+----+----+---------------------------------------------+

    binary 11111111 at the start of the address identifies the
    address as being a multicast address.

                                    +-+-+-+-+
    flgs is a set of 4 flags:       |0|0|0|T|
                                    +-+-+-+-+

    The high-order 3 flags are reserved, and must be initialized
    to 0.

    T = 0 indicates a permanently-assigned ("well-known")
    multicast address, assigned by the Internet Assigned Number
    Authority (IANA).

    T = 1 indicates a non-permanently-assigned ("transient")
    multicast address.

    scop is a 4-bit multicast scope value used to limit the scope
    of the multicast group.  The values are:

    0  reserved
    1  interface-local scope
    2  link-local scope
    3  reserved
    4  admin-local scope
    5  site-local scope
    6  (unassigned)
    7  (unassigned)
    8  organization-local scope
    9  (unassigned)
    A  (unassigned)
    B  (unassigned)
    C  (unassigned)
    D  (unassigned)
    E  global scope
    F  reserved
*/

static __inline uint8_t
l3_addr_type(const struct l3_addr_st *l3_addr)
{
    __be32 st = l3_addr->addr_ip6[0];

    if ((st & htonl(L3_IPV4_MASK)) == htonl(L3_IPV4_MARK))
        return L3_IPV4_TYPE;
    else
        return L3_IPV6_TYPE;
}



/* 功能：获得一个Layer 3掩码地址对象的类型
 * 参数：
 *      @l3_mask: Layer 3掩码地址对象的地址
 * 返回值：
 *      L3_IPV4_TYPE - IPv4
 *      L3_IPV6_TYPE - IPv6
*/
static __inline uint8_t
l3_mask_type(const struct l3_addr_st *l3_mask)
{
    return l3_addr_type(l3_mask);
}



/* 功能：判断一个Lay 3地址对象是否是IPv4类型
 * 参数：
 *      @l3_addr: Lay 3地址对象的地址
 * 返回值：非0 - 是IPv4类型; 0 - 不是IPv4类型
*/
static __inline int
is_ipv4_addr(const struct l3_addr_st *l3_addr)
{
    return l3_addr_type(l3_addr) == L3_IPV4_TYPE;
}



/* 功能：判断一个Layer 3地址对象是否是IPv6类型
 * 参数：
 *      @l3_addr: Layer 3地址对象的地址
 * 返回值：非0 - 是IPv6类型; 0 - 不是IPv6类型
*/
static __inline int
is_ipv6_addr(const struct l3_addr_st *l3_addr)
{
    return l3_addr_type(l3_addr) == L3_IPV6_TYPE;
}



/* 功能：给一个Layer 3地址对象设置IPv4地址值
 * 参数：
 *      @l3_addr: Layer 3地址对象的地址
 *      @addr: IPv4地址值
 * 返回值：0 - 成功; 非0 - 失败
*/
static __inline int
set_ipv4_addr(struct l3_addr_st *l3_addr, uint32_t addr)
{
    l3_addr->addr_in = addr;
    l3_addr->addr_ip6[0] = htonl(L3_IPV4_MARK);
    return 0;
}



/* 功能：给一个Layer 3地址对象设置IPv4地址值
 * 参数：
 *      @l3_addr: Layer 3地址对象的地址
 *      @in: IPv4地址对象的地址
 * 返回值：0 - 成功; 非0 - 失败
 * 注意: 这里将IPv4地址对象强制转换成__be32，如果struct in_addr定义改变
 *       必须修改这里的代码
*/
static __inline int
set_ipv4_addr2(struct l3_addr_st *l3_addr, const struct in_addr *in)
{
    l3_addr->addr_in = *(__be32 *)in;
    l3_addr->addr_ip6[0] = htonl(L3_IPV4_MARK);
    return 0;
}



/* 功能：判断一个in6_addr是否合法，即能否转为l3_addr_st
 * 参数：
 *      @in6: IPv6地址对象的地址
 * 返回值：0 - 非法；1 - 合法
*/
static __inline int
is_in6_addr_valid(const struct in6_addr *in6)
{
    if ((in6->s6_addr32[0] & htonl(L3_IPV4_MASK)) == htonl(L3_IPV4_MARK))
        return 0;

    return 1;
}



/* 功能：给一个Layer 3地址对象设置IPv6地址值
 * 参数：
 *      @l3_addr: Layer 3地址对象的地址
 *      @in6: IPv6地址对象的地址
 * 返回值：0 - 成功；非0 - 失败
*/
static __inline int
set_ipv6_addr(struct l3_addr_st *l3_addr, const struct in6_addr *in6)
{
    if (!is_in6_addr_valid(in6))
        return -1;

    l3_addr->addr_in6 = *in6;
    return 0;
}


/* 功能: 将in6_addr的指针转换为l3_addr指针
 * 参数:
 *      @in6: IPv6地址对象的地址
 * 返回值: l3_addr指针
 */
static __inline struct l3_addr_st*
in6_addr_to_l3_addr(struct in6_addr *in6)
{
    if (!is_in6_addr_valid(in6))
        return NULL;

    return (struct l3_addr_st*)in6;
}



/* 功能：判断两个IPv4地址对象是否相等
 * 参数：
 *      @a1: 指向IPv4地址对象之一
 *      @a2: 指向IPv4地址对象之二
 * 返回值：非0 - 相等; 0 - 不相等
*/
static __inline int
l3_ipv4_addr_equal(const __be32 *a1, const __be32 *a2)
{
    return *a1 == *a2;
}



/* 功能：判断两个IPv6地址对象是否相等
 * 参数：
 *      @a1: 指向IPv6地址对象之一
 *      @a2: 指向IPv6地址对象之二
 * 返回值：非0 - 相等; 0 - 不相等
*/
static __inline int
l3_ipv6_addr_equal(const struct in6_addr *a1, const struct in6_addr *a2)
{
    return (((a1->s6_addr32[0] ^ a2->s6_addr32[0]) |
                (a1->s6_addr32[1] ^ a2->s6_addr32[1]) |
                (a1->s6_addr32[2] ^ a2->s6_addr32[2]) |
                (a1->s6_addr32[3] ^ a2->s6_addr32[3])) == 0);
}



/* 功能：判断两个Layer 3地址对象是否相等
 * 参数：
 *      @l3_addr1: 指向Layer 3地址对象之一
 *      @l3_addr2: 指向Layer 3地址对象之二
 * 返回值：非0 - 相等; 0 - 不相等
*/
static __inline int
l3_addr_equal(const struct l3_addr_st *l3_addr1,
    const struct l3_addr_st *l3_addr2)
{
    if (l3_addr_type(l3_addr1) != l3_addr_type(l3_addr2))
            return 0;

    switch(l3_addr_type(l3_addr1))
    {
    case L3_IPV4_TYPE:
            return l3_ipv4_addr_equal(&l3_addr1->addr_in,
                    &l3_addr2->addr_in);

    case L3_IPV6_TYPE:
            return l3_ipv6_addr_equal(&l3_addr1->addr_in6,
                    &l3_addr2->addr_in6);

    default:
            return 0;
    }
}



/* 功能：跟据Layer 3地址对象产生一个哈希键值
 * 参数：
 *      @l3_addr: 指向Layer 3地址对象
 *      @rnd：随机数
 * 返回值：32位的值
*/
static __inline uint32_t
l3_addr_hash(const struct l3_addr_st *l3_addr, uint32_t rnd)
{
    uint32_t n;

    if (is_ipv6_addr(l3_addr))
    {
        n = sizeof(struct in6_addr);
        return l3_jhash2((uint32_t*)l3_addr->addr_ip6, n/sizeof(uint32_t), rnd);
    }

    n = sizeof(struct in_addr);
    return l3_jhash2((uint32_t*)&l3_addr->addr_ip, n/sizeof(uint32_t), rnd);
}



/* 功能：跟据Layer 3地址对象和端口产生一个哈希键值
 * 参数：
 *      @l3_addr: 指向Layer 3地址对象
 *      @port: 端口号
 *      @rnd：随机数
 * 返回值：32位的值
*/
static __inline uint32_t
l3_hash_2tuple(const struct l3_addr_st *l3_addr,
    uint16_t port, uint32_t rnd)
{
    uint32_t n;

    if (is_ipv6_addr(l3_addr))
    {
        n = sizeof(struct in6_addr);
        return l3_jhash_2words(
            l3_jhash2((uint32_t*)l3_addr->addr_ip6, n/sizeof(uint32_t), rnd),
            (__force uint32_t)port, rnd);
    }

    return l3_jhash_2words((__force uint32_t)l3_addr->addr_ip,
        (__force uint32_t)port, rnd);
}



/* 功能：跟据Layer 3地址对象和端口和4层协议号产生一个哈希键值
 * 参数：
 *      @l3_addr: 指向Layer 3地址对象
 *      @port: 端口号
 *      @proto: 4层协议号
 *      @rnd：随机数
 * 返回值：32位的值
*/
static __inline uint32_t
l3_hash_3tuple(const struct l3_addr_st *l3_addr,
    uint16_t port, uint8_t proto, uint32_t rnd)
{
    uint32_t n;

    if (is_ipv6_addr(l3_addr))
    {
        n = sizeof(struct in6_addr);
        return l3_jhash_3words(
            l3_jhash2((uint32_t*)l3_addr->addr_ip6, n/sizeof(uint32_t), rnd),
            (__force uint32_t)port, (__force uint32_t)proto, rnd);
    }

    return l3_jhash_3words((__force uint32_t)l3_addr->addr_ip,
        (__force uint32_t)port, (__force uint32_t)proto, rnd);
}



/* 功能：跟据五元组产生一个哈希键值
 * 参数：
 *      @l3_saddr: 指向Layer 3源地址对象
 *      @l3_daddr: 指向Layer 3目标地址对象
 *      @sport: 源端口号
 *      @dport: 目的端口号
 *      @rnd：随机数
 * 返回值：32位的值
*/
static __inline uint32_t
l3_hash_4tuple(const struct l3_addr_st *l3_saddr,
    const struct l3_addr_st *l3_daddr, uint16_t sport,
    uint16_t dport, uint32_t rnd)
{
    uint8_t t1, t2;
    uint32_t n;

    t1 = l3_addr_type(l3_saddr);
    t2 = l3_addr_type(l3_daddr);

    if (t1 != t2)
        return 0;

    if (L3_IPV6_TYPE == t1)
    {
        n = sizeof(struct in6_addr);
        return l3_jhash_3words(
            l3_jhash2((uint32_t*)l3_saddr->addr_ip6, n/sizeof(uint32_t), rnd),
            l3_jhash2((uint32_t*)l3_daddr->addr_ip6, n/sizeof(uint32_t), rnd),
            ((sport) << 16) | dport, rnd);
    }

    return l3_jhash_3words((__force uint32_t)l3_saddr->addr_ip,
        (__force uint32_t)l3_daddr->addr_ip,
        ((sport) << 16) | dport, rnd);
}



/* 功能：跟据五元组产生一个哈希键值
 * 参数：
 *      @l3_saddr: 指向Layer 3源地址对象
 *      @l3_daddr: 指向Layer 3目标地址对象
 *      @sport: 源端口号
 *      @dport: 目的端口号
 *      @proto: 4层协议号
 *      @rnd：随机数
 * 返回值：32位的值
*/
static __inline uint32_t
l3_hash_5tuple(const struct l3_addr_st *l3_saddr,
    const struct l3_addr_st *l3_daddr, uint16_t sport,
    uint16_t dport, uint8_t proto, uint32_t rnd)
{
    uint8_t t1, t2;
    uint32_t n;

    t1 = l3_addr_type(l3_saddr);
    t2 = l3_addr_type(l3_daddr);

    if (t1 != t2)
        return 0;

    if (L3_IPV6_TYPE == t1)
    {
        n = sizeof(struct in6_addr);
        return l3_jhash_3words(
            l3_jhash2((uint32_t*)l3_saddr->addr_ip6, n/sizeof(uint32_t), rnd),
            l3_jhash2((uint32_t*)l3_daddr->addr_ip6, n/sizeof(uint32_t), rnd),
            ((sport) << 16) | dport, rnd ^ proto);
    }

    return l3_jhash_3words((__force uint32_t)l3_saddr->addr_ip,
        (__force uint32_t)l3_daddr->addr_ip,
        ((sport) << 16) | dport, rnd ^ proto);
}



/* 功能：跟据IPv4地址对象产生一个in_addr
 * 参数：
 *      @l3_addr: 指向Layer 3地址对象
 * 返回值：32位的值
*/
static __inline struct in_addr*
ipv4_to_in_addr(const struct l3_addr_st *l3_addr)
{
    if (!is_ipv4_addr(l3_addr))
        return NULL;

    return (struct in_addr*)(&l3_addr->addr_ip);
}



/* 功能：跟据IPv6地址对象产生一个in6_addr
 * 参数：
 *      @l3_addr: 指向Layer 3地址对象
 * 返回值：32位的值
*/
static __inline struct in6_addr*
ipv6_to_in6_addr(const struct l3_addr_st *l3_addr)
{
    if (!is_ipv6_addr(l3_addr))
        return NULL;

    return (struct in6_addr*)(l3_addr->addr_ip6);
}



/* 功能：比较两个Layer 3地址的前缀部分是否相等
 * 参数：
 *      @l3_addr1: 指向Layer 3地址对象
 *      @l3_addr2: 指向Layer 3地址对象
 *      @prefix_len：前缘位数
 * 返回值：非0 - 相等，0 - 不相等
*/
extern int
l3_addr_prefix_equal(const struct l3_addr_st *l3_addr1,
    const struct l3_addr_st *l3_addr2, uint32_t prefix_len);



/* 功能：比较两个IPv4类型Layer 3地址的网络号是否相等
 * 参数：
 *      @ipv4_addr1: 指向IPv4类型Layer 3地址对象
 *      @ipv4_addr2: 指向IPv4类型Layer 3地址对象
 *      @netmask：代表网络掩码的Layer 3地址对象
 * 返回值：非0 - 相等; 0 - 不相等或有错误
*/
extern int
ipv4_addr_mask_equal(const struct l3_addr_st *ipv4_addr1,
    const struct l3_addr_st *ipv4_addr2, const struct l3_addr_st *netmask);



/* 功能：比较两个Layer 3地址的大小
 * 参数：
 *      @l3_addr1: 指向Layer 3地址对象
 *      @l3_addr2: 指向Layer 3地址对象
 * 返回值：0 - 相等; -1 - 小于; 1 - 大于; -2 - 地址类型错误;
 *          3 - l3_addr1是ipv6，l3_addr2是ipv4类型
 *          -3 - l3_addr1是ipv4，l3_addr2是ipv6类型
*/
extern int
l3_addr_cmp(const struct l3_addr_st *l3_addr1, const struct l3_addr_st *l3_addr2);



/* 将Layer 3地址从主机字节序转到网络字节序
 * 参数:
 *      @l3_net_addr: 网络字节序的Layer 3地址
 *      @l3_host_addr: 返回参数，主机字节序的Layer 3地址
 * 返回值: 0 - 转换成功
 *          -1 - 没有空间存储返回参数
 *          -2 - 非法的地址类型
 *          -3 - 不能转换成合法的IPv6地址
 */
extern int
l3_addr_ntoh(const struct l3_addr_st *l3_net_addr, struct l3_addr_st *l3_host_addr);



/* 将Layer 3地址从主机字节序转到网络字节序
 * 参数:
 *      @l3_addr: 需要进行转换的Layer 3地址,保存转换的结果
 * 返回值: 0 - 转换成功
 *          -1 - 没有空间存储返回参数
 *          -2 - 非法的地址类型
 *          -3 - 不能转换成合法的IPv6地址
 */
extern int
l3_addr_ntoh2(struct l3_addr_st *l3_addr);


#if 0
/* 功能：比较两个主机序的Layer 3地址的大小
 * 参数：
 *      @l3_addr1: 指向Layer 3主机序地址对象
 *      @l3_addr2: 指向Layer 3主机序地址对象
 * 返回值：0 - 相等; -1 - 小于; 1 - 大于; -2 - 地址类型错误;
 *          3 - l3_addr1是ipv6，l3_addr2是ipv4类型
 *          -3 - l3_addr1是ipv4，l3_addr2是ipv6类型
*/
extern int
l3_host_addr_cmp(const struct l3_addr_st *l3_host_addr1,
    const struct l3_addr_st *l3_host_addr2);



typedef int (*addr_cmp_fn)(const struct l3_addr_st *,
    const struct l3_addr_st *);
#endif


/* 功能：在一个有序的地址范围数组中进行二分法查找
 *      判断给定的地址是否在地址范围内
 *      按照主机序比较进行查找
 * 参数：
 *      @l3_addr: 指向Layer 3主机序地址对象
 *      @l3_range_ary: 地址范围的数组
 * 返回值：0 不在范围内，1 在范围内
*/
extern int
l3_addr_binary_search(const struct l3_addr_st *l3_addr,
    const struct l3_range_st l3_range_ary[],
    unsigned int size);



/* 功能：在一个有序的地址范围数组中进行二分法查找
 *      判断给定的地址是否在地址范围内
 *      按照网络序比较进行查找
 * 参数：
 *      @l3_addr: 指向Layer 3主机序地址对象
 *      @l3_range_ary: 地址范围的数组
 * 返回值：0 不在范围内，1 在范围内
*/
extern int
l3_net_addr_binary_search(const struct l3_addr_st *l3_addr,
    const struct l3_range_st l3_range_ary[],
    unsigned int size);



/* 功能：将一个IPv4类型地址转化为IPv6之IPv4映射地址
 * 参数：
 *      @l3_addr: 指向要转化的Layer 3地址对象
 * 返回值：0 - 成功; <0 - 失败，原地址对象不是IPv4类型
*/
extern int
ipv4_to_ipv6_mapping(struct l3_addr_st *l3_addr);



/* 功能：判断一个Layer 3地址是否是IPv6之IPv4映射地址
 * 参数：
 *      @l3_addr: 指向要判断的Layer 3地址对象
 * 返回值：非0 - 是映射地址; 0 - 不是映射地址
*/
extern int
is_ipv6_addr_mapping(struct l3_addr_st *l3_addr);



/* 功能：将一个IPv4类型地址转化为IPv6之IPv4兼容地址
 * 参数：
 *      @l3_addr: 指向要转化的Layer 3地址对象
 * 返回值：0 - 成功; <0 - 失败，原地址对象不是IPv4类型
*/
extern int
ipv4_to_ipv6_compatible(struct l3_addr_st *l3_addr);



/* 功能：判断一个Layer 3地址是否是IPv6之IPv4兼容地址
 * 参数：
 *      @l3_addr: 指向要判断的Layer 3地址对象
 * 返回值：非0 - 是兼容地址; 0 - 不是兼容地址
*/
extern int
is_ipv6_addr_compatible(struct l3_addr_st *l3_addr);



/* 功能: 判断一个Layer 3地址是否是多播地址
 * 参数：
 *      @l3_addr: 指向要判断的Layer 3地址对象
 * 返回值：非0 - 是多播地址; 0 - 不是多播地址
*/
static __inline int
is_l3_addr_multicast(struct l3_addr_st *l3_addr)
{
    if (is_ipv6_addr(l3_addr))
        return (l3_addr->addr_ip6[0] & htonl(0xFF000000)) == htonl(0xFF000000);
    else if (is_ipv4_addr(l3_addr))
        return (l3_addr->addr_ip & htonl(0xF0000000)) == htonl(0xE0000000);

    return 0;
}



/* 功能: 判断一个Layer 3地址是否是环回地址
 * 参数：
 *      @l3_addr: 指向要判断的Layer 3地址对象
 * 返回值：非0 - 是环回地址; 0 - 不是环回地址
*/
static __inline int
is_l3_addr_loopback(struct l3_addr_st *l3_addr)
{
    if (is_ipv6_addr(l3_addr))
        return ((l3_addr->addr_ip6[0] | l3_addr->addr_ip6[1] |
            l3_addr->addr_ip6[2] | (l3_addr->addr_ip6[3] ^ htonl(1))) == 0);
    else if (is_ipv4_addr(l3_addr))
        return (l3_addr->addr_ip & htonl(0xFF000000)) == htonl(0x7F000000);

    return 0;
}


/* 功能: 判断一个IPv6地址是否是Link Local地址
 * 参数：
 *      @l3_addr: 指向要判断的Layer 3地址对象
 * 返回值：非0 - 是Link Local地址; 0 - 不是Link Local地址
*/
static __inline int
is_link_local_ipv6_addr(struct l3_addr_st *l3_addr)
{
    if (is_ipv6_addr(l3_addr) &&
        (l3_addr->addr_ip6[0] == htonl(0xFE800000)) &&
        (l3_addr->addr_ip6[1] == htonl(0x0)))
        return 1;
    return 0;
}



/* 功能：将一个IPv6类型的IPv4 兼容/映射地址转化为对应的IPv4地址
 * 参数：
 *      @l3_addr: 指向要转化的Layer 3地址对象
 * 返回值：0 - 转换成功; <0 - 失败，原地址不是兼容/映射地址
*/
extern int
ipv6_to_ipv4_special(struct l3_addr_st *l3_addr);



/* 功能：判断一个l3地址是否全0
 * 参数：
 *      @l3_addr: 指向要判断的Layer 3地址对象
 * 返回值：0 不是全0地址; 非0 是全0地址;
*/
extern int
is_l3_addr_zero(const struct l3_addr_st *l3_addr);



/* 功能：判断一个l3地址是否全1
 * 参数：
 *      @l3_addr: 指向要判断的Layer 3地址对象
 * 返回值：0 不是全1地址; 非0 是全1地址;
*/
extern int
is_l3_addr_all_set(const struct l3_addr_st *l3_addr);



/* 功能：判断一个l3地址是否全0
 * 参数：
 *      @l3_addr: 指向要判断的Layer 3地址对象
 * 返回值：0 不是全0地址; 非0 是全0地址;
*/
static __inline const char*
l3_addr_to_binary(const struct l3_addr_st *l3_addr)
{
    if (!l3_addr)
        return NULL;

    if (is_ipv4_addr(l3_addr))
        return (const char*)(&l3_addr->addr_ip);

    if (is_ipv6_addr(l3_addr))
        return (const char*)(l3_addr->addr_ip6);

    return NULL;
}



/* 功能：判断一个Layer 3地址是否在某个范围之内
 * 参数：
 *      @l3_addr: 指向要判断的Layer 3地址对象
 *      @l3_range: 指向代表范围的对象
 * 返回值：0 - 在这个范围内
 *　　　　 1 - 没在范围之内，大于高边界
 *　　　  -1 - 没在范围之内，小于低边界
 *　　　　-2 - 地址类型有问题
*/
extern int
l3_addr_in_range(const struct l3_addr_st *l3_addr,
    const struct l3_range_st *l3_range);



/* 功能：判断一个Layer 3地址是否在某个范围之内
 * 参数：
 *      @l3_addr: 指向要判断的Layer 3地址对象
 *      @p_low: 范围的低边界
 *      @p_hi: 范围的高边界
 * 返回值：0 - 在这个范围内
 *　　　　 1 - 没在范围之内，大于upper边界
 *　　　  -1 - 没在范围之内，小于lower边界
 *　　　　-2 - 地址类型有问题
*/
extern int
l3_addr_in_range2(const struct l3_addr_st *l3_addr,
    const struct l3_addr_st *p_low, const struct l3_addr_st *p_hi);



/* 功能：将一个Layer 3地址格式化成一个可读的字符串
 * 参数：
 *      @l3_addr: 指向要格式化的Layer 3地址对象
 *      @buf: 存放字符串的缓冲区首地址
 *      @size: 缓冲区的长度
 * 返回值：>0 - 字符串的长度
 *　　　　 -1 - 失败，缓冲区长度太小
 *　　　　 -2 - 失败，未知的地址类型
*/
extern int
l3_addr_to_string(const struct l3_addr_st *l3_addr, char buf[], uint32_t size);



/* 功能：在终端上输出Layer 3地址的可读形式
 * 参数：
 *      @l3_addr: 指向要输出的Layer 3地址对象
 * 返回值：无
*/
extern void
print_l3_addr(const struct l3_addr_st *l3_addr);



/* 功能：将一个Layer 3掩码地址格式化成一个可读的字符串
 * 参数：
 *      @l3_addr: 指向要格式化的Layer 3掩码地址对象
 *      @buf: 存放字符串的缓冲区首地址
 *      @size: 缓冲区的长度
 * 返回值：>0 - 字符串的长度
 *　　　　 -1 - 失败，缓冲区长度太小
 *　　　　 -2 - 失败，未知的地址类型
*/
extern int
l3_mask_to_string(const struct l3_addr_st *l3_mask, char buf[], uint32_t size);


/* 功能：在终端上输出Layer 3掩码地址的可读形式
 * 参数：
 *      @l3_mask: 指向要输出的Layer 3掩码地址对象
 * 返回值：无
*/
extern void
print_l3_mask(const struct l3_addr_st *l3_mask);



/* 功能：将掩码从前缀形式转化成ipv4地址形式
 * 参数：
 *      @prefix_len: 前缀形式的掩码
 *      @mask: 指向要输出的Layer 3掩码地址对象
 * 返回值：0表示转化成功，非0表示转化失败
*/
extern int
get_ipv4_mask(long prefix_len, struct l3_addr_st *mask);



/* 功能：将掩码从前缀形式转化成ipv4地址形式
 * 参数：
 *      @prefix_len: 前缀形式的掩码
 *      @mask: 指向要输出的Layer 3掩码地址对象
 * 返回值：0表示转化成功，非0表示转化失败
*/
extern int
get_ipv6_mask(long prefix_len, struct l3_addr_st *mask);



/* 功能：将地址形式的掩码转化成前缀形式
 * 参数：
 *      @mask: Layer 3掩码地址对象
 *      @prefix_len: 输出参数前缀形式的掩码
 * 返回值：0表示转化成功，非0表示转化失败
*/
extern int
get_l3_mask_prefix(struct l3_addr_st *mask, unsigned long *prefix_len);



/* 功能：将一个可读的文本字符串地址转化成一个Layer 3地址对象
 * 参数：
 *      @src: 源字符串，类似于:"192.168.1.1", "::" ...
 *      @dst: 目的Layer 3地址对象的地址
 * 返回值：0 - 成功; <0 - 失败，字符串格式错误
*/
extern int
string_to_l3_addr(const char *src, struct l3_addr_st *dst);



/* 功能：获得一个可读的文本字符串地址类型
 * 参数：
 *      @str: 字符串，类似于:"192.168.1.1", "::" ...
 * 返回值：L3_IPV4_TYPE - IPv4地址
 *      　 L3_IPV6_TYPE - IPv6地址
 *         L3_UNKNOWN_TYPE - 未知的字符串类型
*/
extern uint8_t
l3_string_type(const char *str);



/* 功能：Layer 3地址的可读值加上1
 * 参数：
 *      @l3_addr: 指向要递增的Layer 3地址对象
 * 返回值：0 - 成功; -1 - 出错，已经是最大值了; -2 - 未知地址类型
*/
extern int
l3_addr_inc(struct l3_addr_st *l3_addr);



/* 功能：Layer 3地址的可读值减去1
 * 参数：
 *      @l3_addr: 指向要递减的Layer 3地址对象
 * 返回值：0 - 成功; -1 - 出错，已经是最小值了; -2 - 未知地址类型
*/
extern int
l3_addr_dec(struct l3_addr_st *l3_addr);



/* 功能：判断Layer 3地址1是否比Layer 3地址1小1
 * 参数：
 *      @l3_addr1: 用做判断的地址1
 *      @l3_addr2: 用做判断的地址2
 * 返回值： 1 - Layer 3地址1比Layer 3地址1小1
 *          0 - Layer 3地址1没有比Layer 3地址1小1
 *          -1 - 参数非法
 *          -2 - Layer 3地址类型不同
*/
extern int
is_l3_addr_close_to(const struct l3_addr_st *l3_addr1,
    const struct l3_addr_st *l3_addr2);



/* 功能：根据两个Layer 3地址对象设置一个地址范围结构体
 * 参数：
 *      @l3_range：指向要设置的地址范围结构体
 *      @l3_addr_low：地址范围的低边界
 *      @l3_addr_hi：地址范围的高边界
 * 返回值：0 - 成功; -1 - 地址类型不匹配; -2 - 范围描述不正确
*/
extern int
set_l3_range(struct l3_range_st *l3_range, const struct l3_addr_st *l3_addr_low,
    const struct l3_addr_st *l3_addr_hi);



/* 功能：将一个地址范围结构体转化为可读的文本形式
 * 参数：
 *      @l3_range：指向要转化的地址范围结构体
 *      @buf：存放文本串的缓冲区
 *      @size：缓冲区的长度
 * 返回值：>0 - 成功，字符串的长度; -1 - 失败,缓冲区长度太小; -2 - 未知的地址类型
*/
extern int
l3_range_to_string(const struct l3_range_st *l3_range, char buf[],
    uint32_t size);



/* 功能：检查一个字符串是否是一个非法的IPv4/IPv6地址串
 * 参数：
 *      @szip：指向要检查的字符串
 * 返回值：非0 - 非法的字符串; 0 - 合法的字符串
*/
static __inline int
is_invalid_ip(const char *szip)
{
    struct l3_addr_st l3_addr;

    if (string_to_l3_addr(szip, &l3_addr) != 0)
            return 1;

    return 0;
}



/* 功能：检查一个字符串是否是一个非法的IPv4掩码
 * 参数：
 *      @szmask：指向要检查的字符串
 * 返回值：非0 - 非法的掩码字符串; 0 - 合法的掩码字符串
*/
static __inline int
is_invalid_v4_mask(const char *szmask)
{
    uint32_t u_mask_not;
    struct l3_addr_st netmask;

    if (string_to_l3_addr(szmask, &netmask) != 0 ||
            !is_ipv4_addr(&netmask))
    {
            return 1;
    }

    u_mask_not = ~ntohl(netmask.addr_ip);
    if (u_mask_not & (u_mask_not + 1))
            return 1;

    return 0;
}



/* 功能：检查一个l3地址是否是一个非法的掩码
 * 参数：
 *      @mask：指向要检查的l3地址
 * 返回值：0 - 非法的掩码字符串; 非0 - 合法的掩码字符串
*/
extern int
is_l3_mask_valid(const struct l3_addr_st *mask);



/* 功能：将一个字符串形式的范围描述转化为一个地址范围结构体对象
 * 参数：
 *      @str：描述范围的字符串,格式如下:
 *              类型 1  (1) 192.168.0.0
 *                      (2) ::2
 *              类型 2  (3) 192.168.0.0/16
 *                      (4) ::/16
 *              类型 3  (5) 192.168.0.0/255.255.255.0
 *                      (6) ff::/ffff:ffff:ff00::
 *              类型 4  (7) 192.168.0.0-192.168.255.255
 *                      (8) ::2-::ff
 *      @l3_range：范围结构体对象的地址
 * 返回值：
 *        1: ipv4 类型1
 *        2: ipv6 类型1
 *        3: ipv4 类型2
 *        4: ipv6 类型2
 *        5: ipv4 类型3
 *        6: ipv6 类型3
 *        7: ipv4 类型4
 *        8: ipv6 类型4
 *        大于0：成功
 *        -1：字符串长度过长
 *        -2：无法解析的类型(不会发生)
 *        -3：将高低边界地址转化为Layer 3对象出错，地址格式不对。类型1.
 *        -4：低边界和高边界的地址类型不一致，[IPv4, IPv6]? 类型1.
 *        -5：低边界和高边界的大小有误. 类型1.
 *        -6：前缀长度有误(192.168.0.1/33) 类型2.
 *        -7：将IP转化为Layer 3对象出错(192.168./24) 类型2.
 *        -8：将IP/掩码转化为Layer 3对象出错. 类型3.
 *        -9：IP和掩码类型不匹配. 类型3.
 *　　　　-10:非法的掩码. 类型3.
*/
extern int
l3_string_to_range(const char *str, struct l3_range_st *l3_range);



/* 功能: 比较两个范围是否相同
 * 参数:
 *      @range1: 指向要比较的地址范围结构对象
 *      @range2: 指向要比较的地址范围结构对象
 * 返回值: 1 - 相等; 0 - 不相等
 */
extern int
is_l3_range_equal(const struct l3_range_st *range1,  const struct l3_range_st *range2);



/* 功能：将一个可读的文本字符串地址转化成一个Layer 3掩码地址对象
 * 参数：
 *      @src: 源字符串，类似于:"255.255.255.0", "FF:FF:FF::" ...
 *      @dst: 目的Layer 3掩码地址对象的地址
 * 返回值：0 - 成功; -1 - 失败，字符串格式错误
*/
extern int
string_to_l3_mask(const char *src, struct l3_addr_st *dst);



/* 功能: 获取一个符串形式的范围IP的掩码
 * 参数：
 *      @str：描述范围的字符串,格式如下:
 *              类型 1  (1) 192.168.0.0/16
 *                      (2) ::/16
 *              类型 2  (3) 192.168.0.0/255.255.255.0
 *                      (4) ff::/ffff:ffff:ff::
 *      @mask：掩码保存地址
 * 返回值：
 *         0: 成功
 *        -1: 不支持的字符串格式
 *        -2: 掩码表示字符串格式错误
 *        -3: IP字符串格式错误
 *        -4: 掩码范围长度有错误
 *        -5: 参数为无效指针错误
 */
extern int
get_l3_string_mask(const char *str, struct l3_addr_st *mask);



/*  功能: l3_addr_st类型IP地址和掩码的与操作
 *  参数:
 *          @l3_addr: IP地址
 *          @l3_mask: 掩码
 *          @result: 与操作之后的结果
 *  返回值:
 *          NULL - 失败；非NULL地址 - 成功与操作后的地址
 */
extern struct l3_addr_st *
l3_addr_and_mask(const struct l3_addr_st *l3_addr, const struct l3_addr_st *l3_mask,
                struct l3_addr_st *result);



/* 功能：将一个l3_range_st数组按起始ip的大小进行排序, 任何ipv4地址小于任何ipv6地址
 * 参数：
 *      @l3_range_ary: 需要排序的l3_range_st数组，存储返回结果
 *      @size: l3_range_ary中的元素个数
 *      @dir: 排序方向，LOW_TO_HIGH: 从小到大排列，HIGH_TO_LOW: 从大到小排列
 * 返回值： 0 - 成功;
 *          -1 - range中包含错误的地址类型
*/
enum
{
    LOW_TO_HIGH,
    HIGH_TO_LOW
};

extern int
l3_range_sort(struct l3_range_st l3_range_ary[], unsigned int size, int dir);



/* 功能：整理l3_range_st数组，先按起始ip的大小进行排序，从小到大排列，
 *       然后去除重复range，合并相邻range
 * 参数：
 *      @l3_range_ary: 需要排序的l3_range_st数组，存储返回结果
 *      @psize: l3_range_ary中的元素个数，存储整理后的size
 * 返回值： 0 - 成功;
 *          -1 - range中包含错误的地址类型
 *          -2 - range中start小于end
*/
extern int
l3_range_trim(struct l3_range_st l3_range_ary[], unsigned int* psize);



#ifdef __cplusplus
}
#endif

#endif  /* __LIBWNS_IP_COMPATIBLE_H__ */
