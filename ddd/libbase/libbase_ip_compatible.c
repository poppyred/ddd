#include "libbase_ip_compatible.h"
#include "libbase_strutils.h"

#ifndef __KERNEL__

#define EXPORT_SYMBOL(sym)
#ifndef likely
#define likely
#endif
#ifndef unlikely
#define unlikely
#endif

#endif

/* Auxiliary Macros
 * These are only used in this module.
 */
#define BIN_IPV4_ADDR_SIZE      sizeof(struct in_addr)
#define BIN_IPV6_ADDR_SIZE      sizeof(struct in6_addr)
#define NS_INT16_TYPE_SIZE      sizeof(uint16_t)


/* RANGE 字符串类型 */
#define RANGE_TYPE_SINGLE   0
#define RANGE_TYPE_CIDR     1
#define RANGE_TYPE_NETMASK  2
#define RANGE_TYPE_RANGE    3
#define RANGE_TYPE_INVALID  4



/* RAGNGE 字符串解析错误码 */
#define RANGE_ERR_LENGTH    -1
#define RANGE_ERR_UNKNOWN   -2
#define RANGE_ERR_RTRANS    -3
#define RANGE_ERR_MISMATCH  -4
#define RANGE_ERR_BOUND     -5
#define RANGE_ERR_PREFIX    -6
#define RANGE_ERR_CTRANS    -7
#define RANGE_ERR_NTRANS    -8
#define RANGE_ERR_NMISMATCH -9
#define RANGE_ERR_NETMASK   -10


/* 获取掩码错误 */
#define ERR_MASK_STRING_GETMASK         (-2)    //掩码表示字符串格式错误
#define ERR_IP_STRING_GETMASK           (-3)    //IP字符串格式错误
#define ERR_PREFIX_LEN_GETMASK          (-4)    //CIDR形式掩码位数有错误
#define ERR_ARG_NULL_GETMASK            (-5)    //传入参数错误
#define ERR_STRING_NOTSURPORT_GETMASK   (-1)    //不支持的字符串格式


static __inline int
ipv4_pton(const char *src, uint8_t *dst)
{
    const char digits[] = "0123456789";
    const char *pch;
    int ch, saw_digit = 0, octets = 0;
    uint32_t new_dec;
    uint8_t tmp[BIN_IPV4_ADDR_SIZE];
    uint8_t *tp = tmp;

    *tp = 0;
    while ((ch = *src++) != 0)
    {
        pch = strchr(digits, ch);
        if (pch != NULL)
        {
            new_dec = *tp * 10 + (pch - digits);

            if (new_dec > 255)
                return 0;

            *tp = new_dec;

            if (!saw_digit)
            {
                if (++octets > 4)
                    return 0;

                saw_digit = 1;
            }
        }
        else if (ch == '.' && saw_digit)
        {
            if (octets == 4)
                return 0;

            *++tp = 0;
            saw_digit = 0;
        }
        else
            return 0;
    }
    if (octets < 4)
        return 0;

    memcpy_s(dst, tmp, BIN_IPV4_ADDR_SIZE);

    return 1;
}



static __inline int
ipv6_pton(const char *src, uint8_t *dst)
{
    const char xdigits_l[] = "0123456789abcdef";
    const char xdigits_u[] = "0123456789ABCDEF";
    const char *xdigits, *curtok, *pch;
    int ch, n, i, saw_xdigit = 0;
    uint32_t val = 0, cur_tok_len = 0;
#if 1 /* fix "dereferencing type-punned pointer will break strict-aliasing rules" warning */
    union {
        uint8_t u8a[BIN_IPV6_ADDR_SIZE];
        __be32 st;
    } tmp;
    uint8_t *tp = tmp.u8a;
#else /* old code with warning */
    uint8_t tmp[BIN_IPV6_ADDR_SIZE];
    uint8_t *tp = tmp;
    __be32 st;
#endif

    uint8_t *endp = tp + BIN_IPV6_ADDR_SIZE, *colonp = NULL;

    memset(tp, 0, BIN_IPV6_ADDR_SIZE);

    if (*src == ':')
    {
        if (*++src != ':')
            return 0;
    }

    curtok = src;
    while ((ch = *src++) != 0)
    {
        xdigits = xdigits_l;
        pch = strchr(xdigits, ch);
        if (pch == NULL)
        {
            xdigits = xdigits_u;
            pch = strchr(xdigits, ch);
        }

        if (pch != NULL)
        {
            val <<= 4;
            val |= (pch - xdigits);
            if (val > 0xffff)
                return 0;

            if (++cur_tok_len > 4)
                return 0;

            saw_xdigit = 1;
            continue;
        }

        if (ch == ':' && (*src) == 0 && saw_xdigit == 1)
            return 0;

        if (ch == ':')
        {
            curtok = src;
            if (!saw_xdigit)
            {
                if (colonp)
                    return 0;

                colonp = tp;
                continue;
            }

            if (tp + NS_INT16_TYPE_SIZE > endp)
                return 0;

            *tp++ = (u_char) (val >> 8) & 0xff;
            *tp++ = (u_char) val & 0xff;
            saw_xdigit = 0;
            val = 0;
            cur_tok_len = 0;
            continue;
        }

        if (ch == '.' && ((tp + BIN_IPV4_ADDR_SIZE) <= endp) &&
                ipv4_pton(curtok, tp) > 0)
        {
            tp += BIN_IPV4_ADDR_SIZE;
            saw_xdigit = 0;
            break;
        }

        return 0;
    }

    if (saw_xdigit)
    {
        if (tp + NS_INT16_TYPE_SIZE > endp)
            return 0;

        *tp++ = (u_char) (val >> 8) & 0xff;
        *tp++ = (u_char) val & 0xff;
    }

    if (colonp != NULL)
    {
        if (tp == endp)
            return 0;

        n = tp - colonp;

        for (i = 1; i <= n; i++)
        {
            endp[-i] = colonp[n - i];
            colonp[n - i] = 0;
        }

        tp = endp;
    }

    if (tp != endp)
        return 0;

    /* warn!!! need to do ipv6 sanity check */
#if 1 /* fix "dereferencing type-punned pointer will break strict-aliasing rules" warning */
    if ((tmp.st & htonl(L3_IPV4_MASK)) == htonl(L3_IPV4_MARK))
        return 0;

    memcpy_s(dst, tmp.u8a, BIN_IPV6_ADDR_SIZE);
#else /* old code with warning */
    st = *(__be32 *)tmp;
    if ((st & htonl(L3_IPV4_MASK)) == htonl(L3_IPV4_MARK))
        return 0;

    memcpy_s(dst, tmp, BIN_IPV6_ADDR_SIZE);
#endif

    return 1;
}



static __inline int
l3_pton(uint8_t type, const char *src, void *dst)
{
    if (type == L3_IPV4_TYPE)
        return ipv4_pton(src, (uint8_t*)dst);

    if (type == L3_IPV6_TYPE)
        return ipv6_pton(src, (uint8_t*)dst);

    return -1;
}



static __inline int
l3_ipv4_addr_cmp(const struct l3_addr_st *l3_a1,
        const struct l3_addr_st *l3_a2)
{
    uint32_t a1, a2;

    a1 = ntohl(l3_a1->addr_ip);
    a2 = ntohl(l3_a2->addr_ip);

    return a1 == a2 ? 0 : (a1 > a2 ? 1 : -1);
}



static __inline int
l3_ipv6_addr_cmp(const struct l3_addr_st *l3_a1,
        const struct l3_addr_st *l3_a2)
{
    uint32_t i, a1, a2;

    for (i = 0; i < __ARRAY_SIZE(l3_a1->addr_ip6); ++i)
    {
        a1 = ntohl(l3_a1->addr_ip6[i]);
        a2 = ntohl(l3_a2->addr_ip6[i]);

        if (a1 == a2)
            continue;

        return a1 > a2 ? 1 : -1;
    }

    return 0;
}



static __inline int
l3_addr_cmp_aux(uint8_t type, const struct l3_addr_st *l3_a1,
        const struct l3_addr_st *l3_a2)
{
    if (type == L3_IPV4_TYPE)
        return l3_ipv4_addr_cmp(l3_a1, l3_a2);

    if (type == L3_IPV6_TYPE)
        return l3_ipv6_addr_cmp(l3_a1, l3_a2);

    return -2;
}



int
l3_addr_cmp(const struct l3_addr_st *l3_addr1,
        const struct l3_addr_st *l3_addr2)
{
    uint8_t t1, t2;

    t1 = l3_addr_type(l3_addr1);
    t2 = l3_addr_type(l3_addr2);

    if (t1 == t2)
        return l3_addr_cmp_aux(t1, l3_addr1, l3_addr2);

    return t1 > t2 ? 3 : -3;
}
EXPORT_SYMBOL(l3_addr_cmp);



int
l3_addr_ntoh(const struct l3_addr_st *l3_net_addr,
    struct l3_addr_st *l3_host_addr)
{
    uint8_t type;

    if (unlikely(!l3_net_addr || !l3_host_addr))
        return -1;

    type = l3_addr_type(l3_net_addr);

    if (likely(type == L3_IPV4_TYPE))
    {
        memset(l3_host_addr, 0, sizeof(struct l3_addr_st));
        l3_host_addr->addr_ip = ntohl(l3_net_addr->addr_ip);
        l3_host_addr->addr_ip6[0] = htonl(L3_IPV4_MARK);

    }
    else if (type == L3_IPV6_TYPE)
    {
        int i, size;
        uint8_t *p_addr1, *p_addr2;
        uint32_t st;

        size = sizeof(l3_net_addr->addr_ip6);

        p_addr1 = (uint8_t *)(l3_net_addr->addr_ip6);
        p_addr2 = (uint8_t *)(l3_host_addr->addr_ip6);
        for (i = 0; i < size; ++i)
            p_addr2[i] = p_addr1[size - 1 - i];

        st = (uint32_t)(l3_host_addr->addr_ip6[0]);
        if ((st & htonl(L3_IPV4_MASK)) == htonl(L3_IPV4_MARK))
            return -3;
    }
    else
    {
        return -2;
    }

    return 0;
}
EXPORT_SYMBOL(l3_addr_ntoh);



int
l3_addr_ntoh2(struct l3_addr_st *l3_addr)
{
    uint8_t type;

    if (unlikely(!l3_addr))
        return -1;

    type = l3_addr_type(l3_addr);

    if (likely(type == L3_IPV4_TYPE))
    {
        l3_addr->addr_ip = ntohl(l3_addr->addr_ip);
    }
    else if (type == L3_IPV6_TYPE)
    {
        int i, j, size;
        uint8_t *p_addr;
        uint32_t st;

        size = sizeof(l3_addr->addr_ip6)/2;
        p_addr = (uint8_t *)(l3_addr->addr_ip6);
        for (i = 0; i < size; ++i)
        {
            j = size*2 - 1 - i;
            p_addr[i] = p_addr[i]^p_addr[j];
            p_addr[j] = p_addr[i]^p_addr[j];
            p_addr[i] = p_addr[i]^p_addr[j];
        }

        st = (uint32_t)(l3_addr->addr_ip6[0]);
        if ((st & htonl(L3_IPV4_MASK)) == htonl(L3_IPV4_MARK))
            return -3;
    }
    else
    {
        return -2;
    }

    return 0;
}
EXPORT_SYMBOL(l3_addr_ntoh2);



static __inline int
l3_ipv4_host_addr_cmp(const struct l3_addr_st *l3_a1,
        const struct l3_addr_st *l3_a2)
{
    uint32_t a1, a2;

    a1 = l3_a1->addr_ip;
    a2 = l3_a2->addr_ip;

    return a1 == a2 ? 0 : (a1 > a2 ? 1 : -1);
}



static __inline int
l3_ipv6_host_addr_cmp(const struct l3_addr_st *l3_a1,
        const struct l3_addr_st *l3_a2)
{
    uint32_t i, a1, a2, size;

    size = __ARRAY_SIZE(l3_a1->addr_ip6);
    for (i = size; i > 0; --i)
    {
        a1 = l3_a1->addr_ip6[i - 1];
        a2 = l3_a2->addr_ip6[i - 1];

        if (a1 == a2)
            continue;

        return a1 > a2 ? 1 : -1;
    }

    return 0;
}



static __inline int
l3_host_addr_cmp_aux(uint8_t type, const struct l3_addr_st *l3_host_a1,
        const struct l3_addr_st *l3_host_a2)
{
    if (likely(type == L3_IPV4_TYPE))
    {
        return l3_ipv4_host_addr_cmp(l3_host_a1, l3_host_a2);
    }
    else if (type == L3_IPV6_TYPE)
    {
        return l3_ipv6_host_addr_cmp(l3_host_a1, l3_host_a2);
    }
    else
    {
        return -2;
    }
}



static __inline int
l3_host_addr_cmp(const struct l3_addr_st *l3_host_addr1,
    const struct l3_addr_st *l3_host_addr2)
{
    uint8_t t1, t2;

    t1 = l3_addr_type(l3_host_addr1);
    t2 = l3_addr_type(l3_host_addr2);

    if (likely(t1 == t2))
    {
        return l3_host_addr_cmp_aux(t1, l3_host_addr1, l3_host_addr2);
    }
    else
    {
        return t1 > t2 ? 3 : -3;
    }
}
//EXPORT_SYMBOL(l3_host_addr_cmp);



int l3_addr_binary_search(const struct l3_addr_st *l3_addr,
    const struct l3_range_st l3_range_ary[],
    unsigned int size)
{
    unsigned int idx;
    unsigned int low = 0;
    unsigned int high = size;

    while (low < high)
    {
        idx = (low + high) / 2;
        if (l3_host_addr_cmp(l3_addr, &l3_range_ary[idx].start) < 0)
            high = idx;
        else if (l3_host_addr_cmp(l3_addr, &l3_range_ary[idx].end) > 0)
            low = idx + 1;
        else
            return 1;
    }

    return 0;
}
EXPORT_SYMBOL(l3_addr_binary_search);



int l3_net_addr_binary_search(const struct l3_addr_st *l3_addr,
    const struct l3_range_st l3_range_ary[],
    unsigned int size)
{
    unsigned int idx;
    unsigned int low = 0;
    unsigned int high = size;

    while (low < high)
    {
        idx = (low + high) / 2;
        if (l3_addr_cmp(l3_addr, &l3_range_ary[idx].start) < 0)
            high = idx;
        else if (l3_addr_cmp(l3_addr, &l3_range_ary[idx].end) > 0)
            low = idx + 1;
        else
            return 1;
    }

    return 0;
}
EXPORT_SYMBOL(l3_net_addr_binary_search);



int
ipv4_to_ipv6_mapping(struct l3_addr_st *l3_addr)
{
    if (!is_ipv4_addr(l3_addr))
        return -1;

    l3_addr->addr_ip6[0] = 0;
    l3_addr->addr_ip6[1] = 0;
    l3_addr->addr_ip6[2] = htonl(0xffff);

    return 0;
}
EXPORT_SYMBOL(ipv4_to_ipv6_mapping);


int
ipv4_to_ipv6_compatible(struct l3_addr_st *l3_addr)
{
    if (!is_ipv4_addr(l3_addr))
        return -1;

    l3_addr->addr_ip6[0] = 0;
    l3_addr->addr_ip6[1] = 0;
    l3_addr->addr_ip6[2] = 0;

    return 0;
}
EXPORT_SYMBOL(ipv4_to_ipv6_compatible);



int
is_ipv6_addr_mapping(struct l3_addr_st *l3_addr)
{
    if (!is_ipv6_addr(l3_addr))
        return 0;

    return l3_addr->addr_ip6[0] == 0 &&
        l3_addr->addr_ip6[1] == 0 &&
        l3_addr->addr_ip6[2] == htonl(0xffff);
}
EXPORT_SYMBOL(is_ipv6_addr_mapping);



int
is_ipv6_addr_compatible(struct l3_addr_st *l3_addr)
{
    if (!is_ipv6_addr(l3_addr))
        return 0;

    return l3_addr->addr_ip6[0] == 0 &&
        l3_addr->addr_ip6[1] == 0 &&
        l3_addr->addr_ip6[2] == 0 &&
        l3_addr->addr_ip6[3] != htonl(0x1);
}
EXPORT_SYMBOL(is_ipv6_addr_compatible);



int
ipv6_to_ipv4_special(struct l3_addr_st *l3_addr)
{
    if (is_ipv6_addr_mapping(l3_addr) ||
            is_ipv6_addr_compatible(l3_addr))
    {
        l3_addr->addr_ip6[0] = htonl(L3_IPV4_MARK);

        return 0;
    }

    return -1;
}
EXPORT_SYMBOL(ipv6_to_ipv4_special);



static __inline int
is_ipv4_zero(const struct l3_addr_st *l3_addr)
{
    return (0 == l3_addr->addr_ip);
}


static __inline int
is_ipv6_zero(const struct l3_addr_st *l3_addr)
{
    return (l3_addr->addr_ip6[0] == 0 &&
        l3_addr->addr_ip6[1] == 0 &&
        l3_addr->addr_ip6[2] == 0 &&
        l3_addr->addr_ip6[3] == 0);
}



int
is_l3_addr_zero(const struct l3_addr_st *l3_addr)
{
    if (!l3_addr)
        return 0;

    if (is_ipv4_addr(l3_addr))
        return is_ipv4_zero(l3_addr);

    if (is_ipv6_addr(l3_addr))
        return is_ipv6_zero(l3_addr);

    return 0;
}
EXPORT_SYMBOL(is_l3_addr_zero);



static __inline int
is_ipv4_all_set(const struct l3_addr_st *l3_addr)
{
    return (0xFFFFFFFF == l3_addr->addr_ip);
}



static __inline int
is_ipv6_all_set(const struct l3_addr_st *l3_addr)
{
    return (l3_addr->addr_ip6[0] == 0xFFFFFFFF &&
        l3_addr->addr_ip6[1] == 0xFFFFFFFF &&
        l3_addr->addr_ip6[2] == 0xFFFFFFFF &&
        l3_addr->addr_ip6[3] == 0xFFFFFFFF);
}



int
is_l3_addr_all_set(const struct l3_addr_st *l3_addr)
{
    if (!l3_addr)
        return 0;

    if (is_ipv4_addr(l3_addr))
        return is_ipv4_all_set(l3_addr);

    if (is_ipv6_addr(l3_addr))
        return is_ipv6_all_set(l3_addr);

    return 0;
}
EXPORT_SYMBOL(is_l3_addr_all_set);



static __inline int
ipv4_addr_inc(struct l3_addr_st *l3_addr)
{
    if (l3_addr->addr_ip == (__be32)-1)
        return -1;

    l3_addr->addr_ip = htonl(ntohl(l3_addr->addr_ip) + 1);
    return 0;
}



static __inline int
ipv6_addr_inc(struct l3_addr_st *l3_addr)
{
    int i;

    if (l3_addr->addr_ip6[0] == (__be32)-1 &&
        l3_addr->addr_ip6[1] == (__be32)-1 &&
        l3_addr->addr_ip6[2] == (__be32)-1 &&
        l3_addr->addr_ip6[3] == (__be32)-1)
        return -1;

    for (i = 3; i >= 0; --i)
    {
        if ((l3_addr->addr_ip6[i] =
                    htonl(ntohl(l3_addr->addr_ip6[i]) + 1)) != 0)
        {
            break;
        }
    }

    return 0;
}



int
l3_addr_inc(struct l3_addr_st *l3_addr)
{
    if (!l3_addr)
        return -3;

    if (is_ipv4_addr(l3_addr))
        return ipv4_addr_inc(l3_addr);

    if (is_ipv6_addr(l3_addr))
        return ipv6_addr_inc(l3_addr);

    return -2;
}
EXPORT_SYMBOL(l3_addr_inc);


static __inline int
ipv4_addr_dec(struct l3_addr_st *l3_addr)
{
    if (l3_addr->addr_ip == 0)
        return -1;

    l3_addr->addr_ip = htonl(ntohl(l3_addr->addr_ip) - 1);
    return 0;
}



static __inline int
ipv6_addr_dec(struct l3_addr_st *l3_addr)
{
    int i;

    if (l3_addr->addr_ip6[0] == 0 &&
        l3_addr->addr_ip6[1] == 0 &&
        l3_addr->addr_ip6[2] == 0 &&
        l3_addr->addr_ip6[3] == 0)
        return -1;

    for (i = 3; i >= 0; --i)
    {
        if ((l3_addr->addr_ip6[i] =
                    htonl(ntohl(l3_addr->addr_ip6[i]) - 1)) != (__be32)-1)
        {
            break;
        }
    }

    return 0;
}



int
l3_addr_dec(struct l3_addr_st *l3_addr)
{
    if (!l3_addr)
        return -3;

    if (is_ipv4_addr(l3_addr))
        return ipv4_addr_dec(l3_addr);

    if (is_ipv6_addr(l3_addr))
        return ipv6_addr_dec(l3_addr);

    return -2;
}
EXPORT_SYMBOL(l3_addr_dec);



static __inline int
is_ipv4_addr_close_to(const struct l3_addr_st *l3_addr1,
    const struct l3_addr_st *l3_addr2)
{
    if (l3_addr1->addr_ip == (__be32)-1)
        return 0;

    return (l3_addr2->addr_ip == htonl(ntohl(l3_addr1->addr_ip) + 1));
}



static __inline int
is_ipv6_addr_close_to(const struct l3_addr_st *l3_addr1,
    const struct l3_addr_st *l3_addr2)
{
    int i, carry = 1;

    if (l3_addr1->addr_ip6[0] == (__be32)-1 &&
        l3_addr1->addr_ip6[1] == (__be32)-1 &&
        l3_addr1->addr_ip6[2] == (__be32)-1 &&
        l3_addr1->addr_ip6[3] == (__be32)-1)
        return 0;

    for (i = 3; i >= 0; --i)
    {
        if (l3_addr2->addr_ip6[i] !=
            htonl(ntohl(l3_addr1->addr_ip6[i]) + carry))
            return 0;

        if (0 != l3_addr2->addr_ip6[i])
            carry = 0;
    }

    return 1;
}



int
is_l3_addr_close_to(const struct l3_addr_st *l3_addr1,
    const struct l3_addr_st *l3_addr2)
{
    uint8_t t1, t2;

    if (!l3_addr1 || !l3_addr2)
        return -1;

    t1 = l3_addr_type(l3_addr1);
    t2 = l3_addr_type(l3_addr2);

    if (t1 != t2)
        return -2;

    if (L3_IPV4_TYPE == t1)
        return is_ipv4_addr_close_to(l3_addr1, l3_addr2);
    else if (L3_IPV6_TYPE == t1)
        return is_ipv6_addr_close_to(l3_addr1, l3_addr2);

    return -1;
}
EXPORT_SYMBOL(is_l3_addr_close_to);



static __inline int
ipv6_prefix_equal2_aux(const __be32 *a1, const __be32 *a2,
        unsigned int prefixlen)
{
    unsigned pdw, pbi;

    /* check complete u32 in prefix */
    pdw = prefixlen >> 5;
    if (pdw && memcmp(a1, a2, pdw << 2))
        return 0;

    /* check incomplete u32 in prefix */
    pbi = prefixlen & 0x1f;
    if (pbi && ((a1[pdw] ^ a2[pdw]) & htonl((0xffffffff) << (32 - pbi))))
        return 0;

    return 1;
}



static __inline int
ipv6_prefix_equal2(const struct in6_addr *a1, const struct in6_addr *a2,
        unsigned int prefixlen)
{
    return ipv6_prefix_equal2_aux(a1->s6_addr32, a2->s6_addr32,
            prefixlen);
}



static __inline int
ipv4_prefix_equal_aux(const __be32 addr1, const __be32 addr2,
        unsigned int prefixlen)
{
    if (prefixlen > 32)
        return 0;

    if (prefixlen == 0)
        return 1;

    if ((addr1 ^ addr2) & htonl((0xffffffff) << (32 - prefixlen)))
        return 0;

    return 1;
}



static __inline int
ipv4_prefix_equal(const __be32 *a1, const __be32 *a2,
        unsigned int prefixlen)
{
    return ipv4_prefix_equal_aux(*a1, *a2, prefixlen);
}



int
l3_addr_prefix_equal(const struct l3_addr_st *l3_addr1,
        const struct l3_addr_st *l3_addr2, uint32_t prefix_len)
{
    uint8_t type = l3_addr_type(l3_addr1);
    if (type != l3_addr_type(l3_addr2))
        return 0;

    if (type == L3_IPV4_TYPE)
        return ipv4_prefix_equal(&l3_addr1->addr_in,
                &l3_addr2->addr_in, prefix_len);

    if (type == L3_IPV6_TYPE)
        return ipv6_prefix_equal2(&l3_addr1->addr_in6,
                &l3_addr2->addr_in6, prefix_len);

    return 0;
}
EXPORT_SYMBOL(l3_addr_prefix_equal);


int
ipv4_addr_mask_equal(const struct l3_addr_st *ipv4_addr1,
        const struct l3_addr_st *ipv4_addr2, const struct l3_addr_st *netmask)
{
    uint32_t u_mask_not;

    if (!is_ipv4_addr(ipv4_addr1) || !is_ipv4_addr(ipv4_addr2) ||
            !is_ipv4_addr(netmask))
        return 0;

    u_mask_not = ~ntohl(netmask->addr_ip);
    if (u_mask_not & (u_mask_not + 1))
        return 0;

    if ((ipv4_addr1->addr_ip ^ ipv4_addr2->addr_ip) & (netmask->addr_ip))
        return 0;

    return 1;
}
EXPORT_SYMBOL(ipv4_addr_mask_equal);


static __inline int
ipv4_addr_in_range_aux(const struct l3_addr_st *p_addr,
        const struct l3_addr_st *p_low,
        const struct l3_addr_st *p_hi)
{

    if (l3_ipv4_addr_cmp(p_addr, p_low) < 0)
        return -1;

    if (l3_ipv4_addr_cmp(p_addr, p_hi) > 0)
        return 1;

    return 0;
}



static __inline int
ipv4_addr_in_range(const struct l3_addr_st *l3_addr,
        const struct l3_addr_st *p_low, const struct l3_addr_st *p_hi)
{
    return ipv4_addr_in_range_aux(l3_addr, p_low, p_hi);
}



static __inline int
ipv6_addr_in_range_aux(const struct l3_addr_st *p_addr,
        const struct l3_addr_st *p_low, const struct l3_addr_st *p_hi)
{
    if (l3_ipv6_addr_cmp(p_addr, p_low) < 0)
        return -1;

    if (l3_ipv6_addr_cmp(p_addr, p_hi) > 0)
        return 1;
    return 0;
}



static __inline int
ipv6_addr_in_range(const struct l3_addr_st *l3_addr,
        const struct l3_addr_st *p_low, const struct l3_addr_st *p_hi)
{
    return ipv6_addr_in_range_aux(l3_addr, p_low, p_hi);
}



int
l3_addr_in_range2(const struct l3_addr_st *l3_addr,
        const struct l3_addr_st *p_low, const struct l3_addr_st *p_hi)
{
    if ((l3_addr_type(l3_addr) != l3_addr_type(p_low)) ||
            (l3_addr_type(l3_addr) != l3_addr_type(p_hi)))
        return -2;

    switch (l3_addr_type(l3_addr))
    {
    case L3_IPV4_TYPE:
        return ipv4_addr_in_range(l3_addr, p_low, p_hi);

    case L3_IPV6_TYPE:
        return ipv6_addr_in_range(l3_addr, p_low, p_hi);

    default:
        return -2;
    }
}
EXPORT_SYMBOL(l3_addr_in_range2);


static __inline int
format_ipv4_addr(const struct l3_addr_st *p_addr, char buf[], uint32_t size)
{
    char tmp[MAX_IPV4_TEXTBUF_LEN];
    const uint8_t *s = (const uint8_t*)&p_addr->addr_ip6[3];
    int n;

    n = sprintf(tmp, "%d.%d.%d.%d", s[0], s[1], s[2], s[3]);
    if (n > 0 && (uint32_t)n < size)
    {
        strcpy(buf, tmp);
        return n;
    }

    return -1;
}



static __inline int
format_ipv6_addr(const struct l3_addr_st *p_addr, char buf[], uint32_t size)
{
    char tmp[MAX_IPV6_TEXTBUF_LEN], *tp = tmp;
    int cur_pos = -1, cur_len = 0, best_pos = -1, best_len = 0, i;
    const uint8_t *s = (const uint8_t*)&p_addr->addr_in6.s6_addr[0];
    const uint8_t *p4 = s + 12;
    uint32_t segs[sizeof(struct in6_addr)/sizeof(uint16_t)];

    memset(segs, 0, sizeof(segs));
    for (i = 0; i < (int)__ARRAY_SIZE(p_addr->addr_in6.s6_addr); ++i)
    {
        /* devided to 8 parts, byte order converted */
        segs[i/2] |= (s[i] << ((1 - (i % 2)) << 3));
    }

    for (i = 0; i < (int)__ARRAY_SIZE(segs); ++i)
    {
        if (segs[i] == 0)
        {
            if (cur_pos == -1)
            {
                cur_pos = i;
                cur_len = 1;
            }
            else
                ++cur_len;
        }
        else
        {
            if (cur_pos != -1)
            {
                if (best_pos == -1 || cur_len > best_len)
                {
                    best_pos = cur_pos;
                    best_len = cur_len;
                }
                cur_pos = -1;
            }
        }
    }

    if (cur_pos != -1)
    {
        if (best_pos == -1 || cur_len > best_len)
        {
            best_pos = cur_pos;
            best_len = cur_len;
        }
    }

    if (best_pos != -1 && best_len < 2)
        best_pos = -1;

    for (i = 0; i < (int)__ARRAY_SIZE(segs); ++i)
    {
        if (best_pos != -1 && i >= best_pos && i < best_pos + best_len)
        {
            if (i == best_pos)
                *tp++ = ':';
            continue;
        }

        if (i != 0)
            *tp++ = ':';

        if (i == 6
                && best_pos == 0
                && (best_len == 6
                    || (best_len == 7 && segs[7] != 0x0001)
                    || (best_len == 5 && segs[5] == 0xffff)))
        {
            tp += sprintf(tp, "%d.%d.%d.%d", p4[0], p4[1], p4[2], p4[3]);
            break;
        }

        tp += sprintf(tp, "%x", segs[i]);
    }

    if (best_pos != -1 && best_pos + best_len == __ARRAY_SIZE(segs))
        *tp++ = ':';

    *tp++ = 0;

    if (tp - tmp > size)
        return -1;

    strcpy(buf, tmp);
    return tp - tmp -1;
}



int
l3_addr_to_string(const struct l3_addr_st *l3_addr, char buf[], uint32_t size)
{
    switch(l3_addr_type(l3_addr))
    {
    case L3_IPV4_TYPE:
        return format_ipv4_addr(l3_addr, buf, size);

    case L3_IPV6_TYPE:
        return format_ipv6_addr(l3_addr, buf, size);

    default:
        return -2;
    }
}
EXPORT_SYMBOL(l3_addr_to_string);


void
print_l3_addr(const struct l3_addr_st *l3_addr)
{
    char buf[MAX_IPV6_TEXTBUF_LEN];   /* WARN!! stack depth: > 40 */

    if (l3_addr_to_string(l3_addr, buf, (uint32_t)(MAX_IPV6_TEXTBUF_LEN)) < 0)
        snprintf(buf, MAX_IPV6_TEXTBUF_LEN, "Invalid L3 Address");
#ifdef __KERNEL__
    printk("%s", buf);
#else
    printf("%s", buf);
#endif
}
EXPORT_SYMBOL(print_l3_addr);

int
l3_mask_to_string(const struct l3_addr_st *l3_mask, char buf[], uint32_t size)
{
    switch(l3_addr_type(l3_mask))
    {
    case L3_IPV4_TYPE:
        return format_ipv4_addr(l3_mask, buf, size);

    case L3_IPV6_TYPE:
        return format_ipv6_addr(l3_mask, buf, size);

    default:
        return -2;
    }
}
EXPORT_SYMBOL(l3_mask_to_string);

void
print_l3_mask(const struct l3_addr_st *l3_mask)
{
    char buf[MAX_IPV6_TEXTBUF_LEN];   /* WARN!! stack depth: > 40 */

    if (l3_mask_to_string(l3_mask, buf, (uint32_t)(MAX_IPV6_TEXTBUF_LEN)) < 0)
        snprintf(buf, MAX_IPV6_TEXTBUF_LEN, "Invalid L3 Address");
#ifdef __KERNEL__
    printk("%s", buf);
#else
    printf("%s", buf);
#endif
}
EXPORT_SYMBOL(print_l3_mask);

int
string_to_l3_addr(const char *src, struct l3_addr_st *dst)
{
    struct l3_addr_st tmp;

    memset(&tmp, 0, sizeof(tmp));
    tmp.addr_ip6[0] = htonl(L3_IPV4_MARK);

    if (l3_pton(L3_IPV4_TYPE, src, &tmp.addr_in) != 1)
    {
        if (l3_pton(L3_IPV6_TYPE, src, &tmp.addr_in6) != 1)
            return -1;
    }

    memcpy_s(dst, &tmp, sizeof(tmp));
    return 0;
}
EXPORT_SYMBOL(string_to_l3_addr);


uint8_t
l3_string_type(const char *str)
{
    struct l3_addr_st trivial;

    if (string_to_l3_addr(str, &trivial) != 0)
        return L3_UNKNOWN_TYPE;

    return l3_addr_type(&trivial);
}
EXPORT_SYMBOL(l3_string_type);


int
set_l3_range(struct l3_range_st *l3_range, const struct l3_addr_st *l3_addr_low,
        const struct l3_addr_st *l3_addr_hi)
{
    uint8_t type = l3_addr_type(l3_addr_low);

    if (type != l3_addr_type(l3_addr_hi) || (type != L3_IPV4_TYPE &&
                type != L3_IPV6_TYPE))
    {
        return -1;
    }

    if (l3_addr_cmp(l3_addr_low, l3_addr_hi) > 0)
        return -2;

    l3_range->start = *l3_addr_low;
    l3_range->end = *l3_addr_hi;
    return 0;
}
EXPORT_SYMBOL(set_l3_range);


int
l3_addr_in_range(const struct l3_addr_st *l3_addr,
        const struct l3_range_st *l3_range)
{
    int range_type = l3_addr_type(&l3_range->start);

    if (l3_addr_type(l3_addr) != range_type)
        return -2;

    if (range_type == L3_IPV4_TYPE)
        return ipv4_addr_in_range_aux(l3_addr, &l3_range->start,
                &l3_range->end);

    if (range_type == L3_IPV6_TYPE)
        return ipv6_addr_in_range_aux(l3_addr, &l3_range->start,
                &l3_range->end);

    return -2;
}
EXPORT_SYMBOL(l3_addr_in_range);


static __inline int
l3_ipv4_range_to_string(const struct l3_range_st *l3_range, char buf[],
        uint32_t size)
{
    int len1, len2;

    len1 = format_ipv4_addr(&l3_range->start, buf, size);
    if (len1 < 0 || (uint32_t)len1 + 2 >= size)       /* '-', '\0' */
        return -1;

    buf += len1, size -= len1;
    *buf++ = '-', size -= 1;

    len2 = format_ipv4_addr(&l3_range->end, buf, size);
    if (len2 < 0)
        return -1;

    return len1 + len2 +1;
}



static __inline int
l3_ipv6_range_to_string(const struct l3_range_st *l3_range, char buf[],
        uint32_t size)
{
    int len1, len2;

    len1 = format_ipv6_addr(&l3_range->start, buf, size);
    if (len1 < 0 || (uint32_t)len1 + 2 >= size)       /* 2: '-', '\0' */
        return -1;

    buf += len1, size -= len1;
    *buf++ = '-', size -= 1;

    len2 = format_ipv6_addr(&l3_range->end, buf, size);
    if (len2 < 0)
        return -1;

    return len1 + len2 + 1;
}



int
l3_range_to_string(const struct l3_range_st *l3_range,
        char buf[], uint32_t size)
{
    if (l3_addr_type(&l3_range->start) == L3_IPV4_TYPE)
        return l3_ipv4_range_to_string(l3_range, buf, size);

    if (l3_addr_type(&l3_range->start) == L3_IPV6_TYPE)
        return l3_ipv6_range_to_string(l3_range, buf, size);

    return -2;
}
EXPORT_SYMBOL(l3_range_to_string);


static __inline int
string_to_range_range(struct l3_range_st *l3_range, char *p_t1, char *p_t2)
{
    uint8_t type;
    struct l3_addr_st start, end;

    if (string_to_l3_addr(p_t1, &start) == 0 &&
        string_to_l3_addr(p_t2, &end) == 0)
    {
        type = l3_addr_type(&start);
        if (type == l3_addr_type(&end))
        {
            if (l3_addr_cmp_aux(type, &start, &end) <= 0)
            {
                l3_range->start = start;
                l3_range->end = end;

                if (p_t1 == p_t2)
                    return (L3_IPV4_TYPE == type) ? 1 : 2;
                else
                    return (L3_IPV4_TYPE == type) ? 7 : 8;
            }
            return RANGE_ERR_BOUND;
        }
        return RANGE_ERR_MISMATCH;
    }
    return RANGE_ERR_RTRANS;
}



static __inline void
trim_space_right(char *str)
{
    char *p = str;

    while (*p != 0)
        ++p;
    --p;
    while (p >= str && *p == ' ')
        *p-- = 0;
}



static __inline int
string_to_range_cidr4_aux(struct l3_range_st *l3_range,
        struct l3_addr_st *l3_addr, long prefix_len)
{
    uint32_t mask, addr;

    if (prefix_len < 0 || prefix_len > 32)
        return RANGE_ERR_PREFIX;

    addr = ntohl(l3_addr->addr_ip);
    mask = ~((1ULL << (32 - prefix_len)) - 1);

    l3_range->start.addr_ip6[3] = htonl(addr&mask);
    l3_range->start.addr_ip6[0] = htonl(L3_IPV4_MARK);
    l3_range->end.addr_ip6[3] = htonl(addr|(~mask));
    l3_range->end.addr_ip6[0] = htonl(L3_IPV4_MARK);

    return 3;
}



static __inline int
string_to_range_cidr6_aux(struct l3_range_st *l3_range,
        struct l3_addr_st *l3_addr, long prefix_len)
{
    int bytes, bits, i;
    uint8_t mask, bs;

    if (prefix_len < 0 || prefix_len > 128)
        return RANGE_ERR_PREFIX;

    bytes = prefix_len / 8;
    bits = prefix_len % 8;
    memset(&l3_range->start, 0, sizeof(struct l3_addr_st));
    memset(&l3_range->end, -1, sizeof(struct l3_addr_st));

    for (i = 0; i < bytes; ++i)
    {
        l3_range->start.addr_in6.s6_addr[i] = l3_addr->addr_in6.s6_addr[i];
        l3_range->end.addr_in6.s6_addr[i] = l3_addr->addr_in6.s6_addr[i];
    }

    if (bits != 0)
    {
        mask = ~((1U << (8 - bits)) - 1);
        bs = l3_addr->addr_in6.s6_addr[bytes] & mask;
        l3_range->start.addr_in6.s6_addr[bytes] &= ~mask;
        l3_range->end.addr_in6.s6_addr[bytes] &= ~mask;
        l3_range->start.addr_in6.s6_addr[bytes] |= bs;
        l3_range->end.addr_in6.s6_addr[bytes] |= bs;
    }

    return 4;
}


static __inline int
string_to_range_cidr(struct l3_range_st *l3_range, char *p_t1, char *p_t2)
{
    char *p;
    long prefix_len;
    struct l3_addr_st net_addr;

#ifndef __KERNEL__
    prefix_len = strtol(p_t2, &p, 10);
#else
    prefix_len = simple_strtol(p_t2, &p, 10);
#endif

    if (p == p_t2 || *p != 0 || prefix_len < 0)
        return RANGE_ERR_PREFIX;

    if (string_to_l3_addr(p_t1, &net_addr) == 0)
    {
        if (is_ipv4_addr(&net_addr))
            return string_to_range_cidr4_aux(l3_range,
                    &net_addr, prefix_len);
        return string_to_range_cidr6_aux(l3_range, &net_addr, prefix_len);
    }

    return RANGE_ERR_CTRANS;
}


static __inline int
is_ipv4_mask_valid(const struct l3_addr_st *mask)
{
    uint32_t u_mask, u_mask_not;

    if (NULL == mask)
        return 0;

    if (is_ipv4_zero(mask))
        return 1;

    u_mask = ntohl(mask->addr_ip);

    u_mask_not = ~u_mask;
    if (u_mask_not & (u_mask_not + 1))
        return 0;

    return 1;
}


static __inline int
is_ipv6_mask_valid(const struct l3_addr_st *mask)
{
    int i;
    uint32_t u_mask, u_mask_not;

    if (NULL == mask)
        return 0;

    if (is_ipv6_zero(mask))
        return 1;

    for (i = 0; i < 4; ++i)
    {
        u_mask = ntohl(mask->addr_ip6[i]);

        if (0xFFFFFFFF == u_mask)
            continue;

        u_mask_not = ~u_mask;
        if (u_mask_not & (u_mask_not + 1))
            return 0;
        else
            break;
    }

    i++;
    for (; i < 4; ++i)
    {
        u_mask = ntohl(mask->addr_ip6[i]);
        if (u_mask)
            return 0;
    }

    return 1;
}


int
is_l3_mask_valid(const struct l3_addr_st *mask)
{
    uint8_t type;

    if (!mask)
        return 0;

    type = l3_addr_type(mask);

    if (L3_IPV4_TYPE == type)
        return is_ipv4_mask_valid(mask);
    else if (L3_IPV6_TYPE == type)
        return is_ipv6_mask_valid(mask);

    return 0;
}
EXPORT_SYMBOL(is_l3_mask_valid);

int
get_ipv4_mask_prefix(struct l3_addr_st *mask, unsigned long *prefix_len)
{
    uint32_t u_mask, u_mask_not;

    if (!is_ipv4_mask_valid(mask))
        return -1;

    if (is_ipv4_zero(mask))
    {
        *prefix_len = 0;
        return 0;
    }

    u_mask = ntohl(mask->addr_ip);
    u_mask_not = ~u_mask;
    *prefix_len = 32 - ffz(u_mask_not);

    return 0;
}

int
get_ipv6_mask_prefix(struct l3_addr_st *mask, unsigned long *prefix_len)
{
    int i;
    unsigned long tmp;
    uint32_t u_mask, u_mask_not;

    if (!is_ipv6_mask_valid(mask))
        return -1;

    if (is_ipv6_zero(mask))
    {
        *prefix_len = 0;
        return 0;
    }

    tmp = 0;
    for (i = 0; i < 4; ++i)
    {
        u_mask = ntohl(mask->addr_ip6[i]);

        if (0xFFFFFFFF == u_mask)
        {
            tmp += 32;
            continue;
        }

        if (u_mask)
        {
            u_mask_not = ~u_mask;
            tmp += 32 - ffz(u_mask_not);
        }

        break;
    }

    *prefix_len = tmp;

    return 0;
}

int
get_l3_mask_prefix(struct l3_addr_st *mask, unsigned long *prefix_len)
{
    uint8_t type;

    if (!mask || !prefix_len)
        return -1;

    type = l3_addr_type(mask);
    if (L3_IPV4_TYPE == type)
        return get_ipv4_mask_prefix(mask, prefix_len);
    else if (L3_IPV6_TYPE == type)
        return get_ipv6_mask_prefix(mask, prefix_len);

    return -1;
}
EXPORT_SYMBOL(get_l3_mask_prefix);


static __inline int
string_to_range_mask(struct l3_range_st *l3_range, char *p_t1, char *p_t2)
{
    uint8_t type;
    struct l3_addr_st start, mask;
    uint32_t u_mask, u_mask_not;

    if (string_to_l3_addr(p_t1, &start) == 0 &&
            string_to_l3_addr(p_t2, &mask) == 0)
    {
        if (!is_l3_mask_valid(&mask))
            return RANGE_ERR_NETMASK;

        type = l3_addr_type(&start);
        if (type == L3_IPV4_TYPE && type == l3_addr_type(&mask))
        {
            u_mask = mask.addr_ip;
            u_mask_not = ~u_mask;

            l3_range->start = start;
            l3_range->start.addr_ip &= u_mask;
            l3_range->start.addr_ip6[0] = htonl(L3_IPV4_MARK);
            l3_range->end = start;
            l3_range->end.addr_ip |= u_mask_not;
            l3_range->end.addr_ip6[0] = htonl(L3_IPV4_MARK);

            return 5;
        }
        else if (type == L3_IPV6_TYPE && type == l3_addr_type(&mask))
        {
            int idx;

            l3_range->start = start;
            l3_range->end = start;

            for (idx = 0; idx < 4; ++idx)
            {
                u_mask = mask.addr_ip6[idx];
                u_mask_not = ~u_mask;

                l3_range->start.addr_ip6[idx] &= u_mask;
                l3_range->end.addr_ip6[idx] |= u_mask_not;
            }

            return 6;
        }
        return RANGE_ERR_NMISMATCH;
    }

    return RANGE_ERR_NTRANS;
}


static __inline int
scan_range_string(char *buf, char **p_p1, char **p_p2)
{
    char *p_tok, *p1, *p2;

    if((p_tok = strchr(buf, '/')) != NULL)
    {
        *p_tok++ = 0;
        p1 = buf;
        p2 = p_tok;
        while (*p1 == ' ')
            ++p1;
        while (*p2 == ' ')
            ++p2;
        *p_p1 = p1;
        *p_p2 = p2;
        trim_space_right(p1);
        trim_space_right(p2);
        if (((p_tok = strchr(p2, '.')) != NULL) ||
                ((p_tok = strchr(p2, ':')) != NULL))
            return RANGE_TYPE_NETMASK;
        return RANGE_TYPE_CIDR;
    }
    else
    {
        if ((p_tok = strchr(buf, '-')) != NULL)
        {
            *p_tok++ = 0;
            p1 = buf;
            p2 = p_tok;
            while (*p1 == ' ')
                ++p1;
            while (*p2 == ' ')
                ++p2;
            trim_space_right(p1);
            trim_space_right(p2);
            *p_p1 = p1;
            *p_p2 = p2;
            return RANGE_TYPE_RANGE;
        }
        else
        {
            p1 = buf;
            while (*p1 == ' ')
                ++p1;
            trim_space_right(p1);
            *p_p1 = p1;
            *p_p2 = p1;
            return RANGE_TYPE_SINGLE;
        }
    }
    return RANGE_TYPE_INVALID;
}


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
int
l3_string_to_range(const char *str, struct l3_range_st *l3_range)
{
    char tmp[MAX_RANGE_STRING_BUFF];
    char *pt_1, *pt_2;
    int string_type;

    if (strlen(str) + 1 > MAX_RANGE_STRING_BUFF)
        return RANGE_ERR_LENGTH;

    strncpy(tmp, str, MAX_RANGE_STRING_BUFF);
    string_type = scan_range_string(tmp, &pt_1, &pt_2);

    switch(string_type)
    {
    case RANGE_TYPE_SINGLE:
        return string_to_range_range(l3_range, pt_1, pt_2);

    case RANGE_TYPE_CIDR:
        return string_to_range_cidr(l3_range, pt_1, pt_2);

    case RANGE_TYPE_NETMASK:
        return string_to_range_mask(l3_range, pt_1, pt_2);

    case RANGE_TYPE_RANGE:
        return string_to_range_range(l3_range, pt_1, pt_2);

    default:
        return RANGE_ERR_UNKNOWN;
    }
}
EXPORT_SYMBOL(l3_string_to_range);

int
string_to_l3_mask(const char *src, struct l3_addr_st *dst)
{
    struct l3_addr_st tmp;

    memset(&tmp, 0, sizeof(tmp));
    tmp.addr_ip6[0] = htonl(L3_IPV4_MARK);

    if (strchr(src, ':') == NULL)
    {
        if (l3_pton(L3_IPV4_TYPE, src, &tmp.addr_in) != 1)
            return -1;
    }
    else
    {
        if (l3_pton(L3_IPV6_TYPE, src, &tmp.addr_in6) != 1)
            return -1;
    }

    if (!is_l3_mask_valid(&tmp))
        return -1;

    memcpy_s(dst, &tmp, sizeof(struct l3_addr_st));
    return 0;
}
EXPORT_SYMBOL(string_to_l3_mask);

int
is_l3_range_equal(const struct l3_range_st *range1,
        const struct l3_range_st *range2)
{
    if (l3_addr_type(&range1->start) !=
            l3_addr_type(&range2->start))
        return 0;

    if ((l3_addr_cmp(&range1->start,
                    &range2->start) == 0) &&
            (l3_addr_cmp(&range1->end, &range2->end) == 0))
    {
        return 1;
    }
    return 0;
}
EXPORT_SYMBOL(is_l3_range_equal);

int
get_ipv4_mask(long prefix_len, struct l3_addr_st *mask)
{
    struct l3_addr_st tmpmask;
    memset(&tmpmask, 0, sizeof(struct l3_addr_st));

    if (prefix_len > 32)
        return ERR_PREFIX_LEN_GETMASK;

    string_to_l3_mask("255.255.255.255", &tmpmask);
    tmpmask.addr_ip = ntohl(tmpmask.addr_ip);
    tmpmask.addr_ip &= ~((1ULL << (32 - prefix_len)) - 1);
    tmpmask.addr_ip = htonl(tmpmask.addr_ip);
    *mask = tmpmask;

    return 0;
}
EXPORT_SYMBOL(get_ipv4_mask);

int
get_ipv6_mask(long prefix_len, struct l3_addr_st *mask)
{
    int bytes, bits, i;
    uint8_t umask, bs;
    struct l3_addr_st tmpmask;

    if (prefix_len < 0 || prefix_len > 128)
        return RANGE_ERR_PREFIX;

    if (string_to_l3_mask("FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF",
                &tmpmask) != 0)
        return ERR_PREFIX_LEN_GETMASK;

    memset(mask, 0, sizeof(struct l3_addr_st));
    bytes = prefix_len / 8;
    bits = prefix_len % 8;
    for (i = 0; i < bytes; ++i)
    {
        mask->addr_in6.s6_addr[i] = tmpmask.addr_in6.s6_addr[i];
    }

    if (bits != 0)
    {
        umask = ~((1U << (8 - bits)) - 1);
        bs = tmpmask.addr_in6.s6_addr[bytes] & umask;
        mask->addr_in6.s6_addr[bytes] &= ~umask;
        mask->addr_in6.s6_addr[bytes] |= bs;
    }

    return 0;
}
EXPORT_SYMBOL(get_ipv6_mask);

static __inline int
get_cidrstring_mask(const char *ipbegin, const char *maskbegin,
        struct l3_addr_st *mask)
{
    char *p;
    struct l3_addr_st tmpmask;
    long prefix_len = -1;

#ifndef __KERNEL__
    prefix_len = strtol(maskbegin, &p, 10);
#else
    prefix_len = simple_strtol(maskbegin, &p, 10);
#endif

    if (p == maskbegin || *p != 0 || prefix_len < 0)
        return ERR_PREFIX_LEN_GETMASK;

    if (string_to_l3_addr(ipbegin, &tmpmask) != 0)
        return ERR_IP_STRING_GETMASK;

    if (l3_addr_type(&tmpmask) == L3_IPV4_TYPE)
    {
        return get_ipv4_mask(prefix_len, mask);
    }

    if (l3_addr_type(&tmpmask) == L3_IPV6_TYPE)
    {
        return get_ipv6_mask(prefix_len, mask);
    }

    return ERR_IP_STRING_GETMASK;
}


static __inline int
get_netmask_mask(const char* maskbegin, struct l3_addr_st *mask)
{
    if (string_to_l3_mask(maskbegin, mask) != 0)
        return ERR_MASK_STRING_GETMASK;

    if (!is_l3_mask_valid(mask))
        return ERR_MASK_STRING_GETMASK;

    return 0;
}


int
get_l3_string_mask(const char *str, struct l3_addr_st *mask)
{
    char tmp[MAX_RANGE_STRING_BUFF];
    char *pt_1, *pt_2;
    int string_type;

    if ((str == NULL) || (mask == NULL))
        return ERR_ARG_NULL_GETMASK;

    if (strlen(str) + 1 > MAX_RANGE_STRING_BUFF)
        return ERR_MASK_STRING_GETMASK;

    memset(tmp, 0, sizeof(tmp));
    strncpy(tmp, str, sizeof(tmp));
    string_type = scan_range_string(tmp, &pt_1, &pt_2);

    switch(string_type)
    {
    case RANGE_TYPE_CIDR:
        return get_cidrstring_mask(pt_1, pt_2, mask);

    case RANGE_TYPE_NETMASK:
        return get_netmask_mask(pt_2, mask);

    default:
        return ERR_STRING_NOTSURPORT_GETMASK;
    }
}
EXPORT_SYMBOL(get_l3_string_mask);


struct l3_addr_st *
l3_addr_and_mask(const struct l3_addr_st *l3_addr,
        const struct l3_addr_st *l3_mask, struct l3_addr_st *result)
{
    int idx;

    if ((l3_addr == 0) || (l3_mask == 0) || (result == 0))
        return NULL;

    if (!is_l3_mask_valid(l3_mask))
        return NULL;

    if (l3_addr_type(l3_addr) != l3_addr_type(l3_mask))
        return NULL;

    *result = *l3_addr;

    if (l3_addr_type(l3_addr) == L3_IPV4_TYPE)
        result->addr_in &= l3_mask->addr_in;

    if (l3_addr_type(l3_addr) == L3_IPV6_TYPE)
    {
        for (idx=0; idx < 4; ++idx)
            result->addr_ip6[idx] &= l3_mask->addr_ip6[idx];
    }

    return result;
}
EXPORT_SYMBOL(l3_addr_and_mask);



static __inline int
l3_addr_swap(struct l3_addr_st* l3_addr1, struct l3_addr_st* l3_addr2)
{
    int i;

    if (!l3_addr1 || !l3_addr2)
        return -1;

    for (i = 0; i < 4; ++i)
    {
        l3_addr1->addr_ip6[i] = l3_addr1->addr_ip6[i]^l3_addr2->addr_ip6[i];
        l3_addr2->addr_ip6[i] = l3_addr1->addr_ip6[i]^l3_addr2->addr_ip6[i];
        l3_addr1->addr_ip6[i] = l3_addr1->addr_ip6[i]^l3_addr2->addr_ip6[i];
    }
    return 0;
}



int
l3_range_sort(struct l3_range_st l3_range_ary[], unsigned int size, int dir)
{
    int ret;
    unsigned int i, j;

    for (i = 0; i < size; ++i)
    {
        for (j = i + 1; j < size; ++j)
        {
            ret = l3_addr_cmp(&l3_range_ary[i].start,
                &l3_range_ary[j].start);

            if (ret == -2)
                return -1;

            if ((ret > 0 && dir == LOW_TO_HIGH) ||
                (ret < 0 && dir == HIGH_TO_LOW))
            {
                l3_addr_swap(&l3_range_ary[i].start, &l3_range_ary[j].start);
                l3_addr_swap(&l3_range_ary[i].end, &l3_range_ary[j].end);
            }
        }
    }
    return 0;
}
EXPORT_SYMBOL(l3_range_sort);



static __inline int
l3_range_trim_aux(struct l3_range_st l3_range_ary[], unsigned int* psize)
{
    int ret;
    unsigned int i, j;

    j = 0;
    for (i = 0; i < *psize - 1; ++i)
    {
        ret = l3_addr_cmp(&l3_range_ary[i].end, &l3_range_ary[j].end);
        if (ret == -2)
            return -1;
        else if (ret == 1)
            l3_range_ary[j].end = l3_range_ary[i].end;

        ret = l3_addr_cmp(&l3_range_ary[j].end, &l3_range_ary[i + 1].start);
        if (ret == -2)
            return -1;
        else if (ret == 1 || ret == 0)
            continue;
        if (1 != is_l3_addr_close_to(&l3_range_ary[j].end,
            &l3_range_ary[i + 1].start))
        {
            ++j;
            if ((ret = set_l3_range(&l3_range_ary[j],
                &l3_range_ary[i + 1].start, &l3_range_ary[i + 1].end)) != 0)
                return ret;
        }
    }

    // 处理最后一个IP范围
    ret = l3_addr_cmp(&l3_range_ary[*psize - 1].end, &l3_range_ary[j].end);
    if (ret == -2)
        return -1;
    else if (ret == 1)
        l3_range_ary[j].end = l3_range_ary[*psize - 1].end;

    *psize = j + 1;

    return 0;
}



int
l3_range_trim(struct l3_range_st l3_range_ary[], unsigned int* psize)
{
    int ret;

    if (0 == *psize)
        return 0;

    if ((ret = l3_range_sort(l3_range_ary, *psize, LOW_TO_HIGH)) != 0)
        return ret;

    return l3_range_trim_aux(l3_range_ary, psize);
}
EXPORT_SYMBOL(l3_range_trim);



// End: ip_compatible.c
// Model: C
