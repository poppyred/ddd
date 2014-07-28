/* 内部接口，外部程序不要包含我 */
#ifndef _LIBWNS_CRC_TABLE_
#define _LIBWNS_CRC_TABLE_

#ifdef WORDS_BIGENDIAN
#include "libbase_crc_be.h"
#else
#include "libbase_crc_le.h"
#endif

#endif /* end of include guard: _LIBWNS_CRC_TABLE_ */
