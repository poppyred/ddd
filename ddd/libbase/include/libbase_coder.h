/*!
 * \file libbase_coder.h
 * \brief 加密与解密
 * 移植的代码
 *
 * \author Adam Xiao (iefcu), iefcuxy@gmail.com
 * \date 2012-11-14 11:47:08
 */

#ifndef _LIBWNS_CODER_
#define _LIBWNS_CODER_

#include "libbase_type.h"

__BEGIN_DECLS

/* Do base-64 encoding on a hunk of bytes.   Return the actual number of
 ** bytes generated.  Base-64 encoding takes up 4/3 the space of the original,
 ** plus a bit for end-padding.  3/2+5 gives a safe margin.
 */
/**
 * 函数的描述:base64编码
 * @param [IN]  inbuf   要解码的字符串
 * @param [IN]  inlen   输入的字符串的长度
 * @param [OUT] outbuf  解码后的字符串
 * @param [IN]  outlen输出的空间的长度
 * @return 返回编码后的字符串长度,失败返回－1
 */
int h_base64_encode(const uint8_t *inbuf, int inlen,
        uint8_t *outbuf, int outlen);

/* Do base-64 decoding on a string.  Ignore any non-base64 bytes.
 ** Return the actual number of bytes generated.  The decoded size will
 ** be at most 3/4 the size of the encoded, and may be smaller if there
 ** are padding characters (blanks, newlines).
 */
/**
 * 函数的描述:base64解码解压缩
 * @param [IN]  inbuf   要解码的字符串
 * @param [IN]  inlen   输入的字符串的长度
 * @param [OUT] outbuf  解码后的字符串
 * @param [IN]  outlen输出的空间的长度
 * @return 返回解码后的字符串长度,失败返回－1
 */
int h_base64_decode(const uint8_t *inbuf, int inlen,
        uint8_t *outbuf, int outlen);

__END_DECLS

#endif /* end of include guard: _LIBWNS_CODER_ */
