/*!
 * \file libbase_byteorder.h
 * \brief 字节序转换
 * c的标准库中没有提供64位字节序转换, 在此提供
 * \author Adam Xiao (iefcu), iefcuxy@gmail.com
 * \date 2012-10-22 17:34:54
 */

#ifndef _LIBWNS_BYTEORDER_
#define _LIBWNS_BYTEORDER_

#include <asm/byteorder.h>

/*!
 * 如下的宏是从linux-2.6.32.59源码中的 byteorder/generic.h挖出来的
 * 如有不明白之处, 还可去看linux内核源码
 */

/*
 * The "... p" macros, like le64_to_cpup, can be used with pointers
 * to unaligned data, but there will be a performance penalty on
 * some architectures.  Use get_unaligned for unaligned data.
 *
 * Conversion of XX-bit integers (16- 32- or 64-)
 * between native CPU format and little/big endian format
 * 64-bit stuff only defined for proper architectures
 *  cpu_to_[bl]eXX(__uXX x)
 *  [bl]eXX_to_cpu(__uXX x)
 *
 * The same, but takes a pointer to the value to convert
 *  cpu_to_[bl]eXXp(__uXX x)
 *  [bl]eXX_to_cpup(__uXX x)
 *
 * The same, but change in situ
 *  cpu_to_[bl]eXXs(__uXX x)
 *  [bl]eXX_to_cpus(__uXX x)
 *
 * See asm-foo/byteorder.h for examples of how to provide
 * architecture-optimized versions
*/
#define cpu_to_le64 __cpu_to_le64
#define le64_to_cpu __le64_to_cpu
#define cpu_to_le32 __cpu_to_le32
#define le32_to_cpu __le32_to_cpu
#define cpu_to_le16 __cpu_to_le16
#define le16_to_cpu __le16_to_cpu
#define cpu_to_be64 __cpu_to_be64
#define be64_to_cpu __be64_to_cpu
#define cpu_to_be32 __cpu_to_be32
#define be32_to_cpu __be32_to_cpu
#define cpu_to_be16 __cpu_to_be16
#define be16_to_cpu __be16_to_cpu
#define cpu_to_le64p __cpu_to_le64p
#define le64_to_cpup __le64_to_cpup
#define cpu_to_le32p __cpu_to_le32p
#define le32_to_cpup __le32_to_cpup
#define cpu_to_le16p __cpu_to_le16p
#define le16_to_cpup __le16_to_cpup
#define cpu_to_be64p __cpu_to_be64p
#define be64_to_cpup __be64_to_cpup
#define cpu_to_be32p __cpu_to_be32p
#define be32_to_cpup __be32_to_cpup
#define cpu_to_be16p __cpu_to_be16p
#define be16_to_cpup __be16_to_cpup
#define cpu_to_le64s __cpu_to_le64s
#define le64_to_cpus __le64_to_cpus
#define cpu_to_le32s __cpu_to_le32s
#define le32_to_cpus __le32_to_cpus
#define cpu_to_le16s __cpu_to_le16s
#define le16_to_cpus __le16_to_cpus
#define cpu_to_be64s __cpu_to_be64s
#define be64_to_cpus __be64_to_cpus
#define cpu_to_be32s __cpu_to_be32s
#define be32_to_cpus __be32_to_cpus
#define cpu_to_be16s __cpu_to_be16s
#define be16_to_cpus __be16_to_cpus

/*!
 * 64 bit host byte order convert to network byte order
 */
#define wns_htonll(x) cpu_to_be64(x)
/*!
 * 64 bit network byte order convert to host byte order
 */
#define wns_ntohll(x) be64_to_cpu(x)

#endif /* end of include guard: _LIBWNS_BYTEORDER_ */
