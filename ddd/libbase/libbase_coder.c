/**
 * @file coder.c
 * @brief 公共模块
 * @date 2007-07-12
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "libbase_coder.h"

/*********** base64 **************************************************************/

/* Base-64 encoding.  This encodes binary data as printable ASCII characters.
** Three 8-bit binary bytes are turned into four 6-bit values, like so:
**
**   [11111111]  [22222222]  [33333333]
**
**   [111111] [112222] [222233] [333333]
**
** Then the 6-bit values are represented using the characters "A-Za-z0-9+/".
*/

static char s_b64_encode_table[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',  /* 0-7 */
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',  /* 8-15 */
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',  /* 16-23 */
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',  /* 24-31 */
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',  /* 32-39 */
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',  /* 40-47 */
    'w', 'x', 'y', 'z', '0', '1', '2', '3',  /* 48-55 */
    '4', '5', '6', '7', '8', '9', '+', '/'   /* 56-63 */
};

static int b64_decode_table[256] =
{
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
};

int h_base64_encode(const uint8_t *inbuf, int inlen,
        uint8_t *outbuf, int outlen)
{
    int ptr_idx = 0;
    int space_idx = 0;
    int  phase = 0;
    char c;

    if (inlen<0 || outlen<0)
    {
        return -1;
    }
    else if (0==inlen)
    {
        return 0;
    }

    space_idx = 0;
    phase = 0;
    for ( ptr_idx = 0; ptr_idx < inlen; ++ptr_idx )
    {
        switch ( phase )
        {
        case 0:
            c = s_b64_encode_table[inbuf[ptr_idx] >> 2];
            if ( space_idx < outlen )
            {
                outbuf[space_idx++] = c;
            }
            else
            {
                return -1;
            }
            c = s_b64_encode_table[( inbuf[ptr_idx] & 0x3 ) << 4];
            if ( space_idx < outlen )
            {
                outbuf[space_idx++] = c;
            }
            else
            {
                return -1;
            }
            ++phase;
            break;
        case 1:
            outbuf[space_idx - 1] =
                s_b64_encode_table[
                    b64_decode_table[outbuf[space_idx - 1]] |
                    ( inbuf[ptr_idx] >> 4 ) ];
            c = s_b64_encode_table[( inbuf[ptr_idx] & 0xf ) << 2];
            if ( space_idx < outlen )
            {
                outbuf[space_idx++] = c;
            }
            else
            {
                return -1;
            }
            ++phase;
            break;
        case 2:
            outbuf[space_idx - 1] =
                s_b64_encode_table[
                    b64_decode_table[outbuf[space_idx - 1]] |
                    ( inbuf[ptr_idx] >> 6 ) ];
            c = s_b64_encode_table[inbuf[ptr_idx] & 0x3f];
            if ( space_idx < outlen )
            {
                outbuf[space_idx++] = c;
            }
            else
            {
                return -1;
            }
            phase = 0;
            break;
        default:
            assert(0); // should never reach here
            break;
        }
    }
    /* Pad with ='s. */
    while ( space_idx % 4 != 0)
    {
        if ( space_idx < outlen )
        {
            outbuf[space_idx++] = '=';
        }
        else
        {
            return -1;
        }
    }
    return space_idx;
}

/* Do base-64 decoding on a string.  Ignore any non-base64 bytes.
 ** Return the actual number of bytes generated.  The decoded size will
 ** be at most 3/4 the size of the encoded, and may be smaller if there
 ** are padding characters (blanks, newlines).
 */
/**
 * 函数的描述:base64解码解压缩
 * @param [IN]	inbuf	要解码的字符串
 * @param [IN]	inlen	输入的字符串的长度
 * @param [OUT] space 	解码后的字符串
 * @param [IN]	size输出的空间的长度
 * @return 返回解码后的字符串长度,失败返回－1
 */
int h_base64_decode(const uint8_t *inbuf, int inlen,
        uint8_t *outbuf, int outlen)
{
    const char* cp;
    int space_idx, phase;
    int d, prev_d = 0;
    uint8_t c;
    int i = 0;

    if (inlen<0 || outlen<0)
    {
        return -1;
    }
    else if (0==inlen)
    {
        return 0;
    }

    space_idx = 0;
    phase = 0;
    /* for ( cp = (char*)inbuf; *cp != '\0'; ++cp ) */
    for ( cp = (const char*)inbuf; i < inlen; ++cp, ++i )
    {
        d = b64_decode_table[(int) *cp];
        if ( d != -1 )
        {
            switch ( phase )
            {
            case 0:
                ++phase;
                break;
            case 1:
                c = (uint8_t)( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
                if ( space_idx < outlen )
                    outbuf[space_idx++] = c;
                else
                    return -1;
                ++phase;
                break;
            case 2:
                c = (uint8_t)((( prev_d & 0xf ) << 4) | ( ( d & 0x3c ) >> 2 ));
                if ( space_idx < outlen )
                    outbuf[space_idx++] = c;
                else
                    return -1;
                ++phase;
                break;
            case 3:
                c = (uint8_t)( ( ( prev_d & 0x03 ) << 6 ) | d );
                if ( space_idx < outlen )
                    outbuf[space_idx++] = c;
                else
                    return -1;
                phase = 0;
                break;
            default:
                assert(0); // should never reach here
                break;
            }
            prev_d = d;
        }
    }
    return space_idx;
}
