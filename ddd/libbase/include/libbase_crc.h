/**
 * \file crc.h
 * crc计算
 * \author hyb
 * \defgroup base 基础函数库
 * \{
 */
#ifndef __LIBWNS_CRC_H__
#define __LIBWNS_CRC_H__

#include "libbase_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * crc32计算
 * \param str 要计算的数据指针
 * \param len 数据的长度
 * \param crc crc的初始值，也可以是上次没计算完的crc。这样就可以不用把数据一次性全部放在内存里面。
 *
 * \return crc32值，无符号32位整形
 */
uint32_t h_crc32(const uint8_t *buf, size_t len, uint32_t crc);

/**
 * crc计算
 * \param str 要计算的数据指针
 * \param len 数据的长度
 * \param crc crc的初始值，也可以是上次没计算完的crc。这样就可以不用把数据一次性全部放在内存里面。
 *
 * \return crc64值，无符号64位整形
 */
uint64_t h_crc64(const uint8_t *buf, size_t len, uint64_t crc);

#ifdef __cplusplus
}
#endif

/** \} */
#endif /* __LIBWNS_CRC_H__ */
