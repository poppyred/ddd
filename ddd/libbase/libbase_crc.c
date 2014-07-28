
///////////////////////////////////////////////////////////////////////////////
//
/// \file       crc32.c
/// \brief      CRC32 calculation
//
//  This code has been put into the public domain.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
///////////////////////////////////////////////////////////////////////////////

#include "libbase_crc.h"
#include "libbase_type.h"
#include "libbase_config.h"

#include "libbase_crc_table.h"

#ifdef WORDS_BIGENDIAN
#   include "libbase_check_byteswap.h"

#   define A(x) ((x) >> 24)
#   define B(x) (((x) >> 16) & 0xFF)
#   define C(x) (((x) >> 8) & 0xFF)
#   define D(x) ((x) & 0xFF)

#   define S8(x) ((x) << 8)
#   define S32(x) ((x) << 32)

#else
#   define A(x) ((x) & 0xFF)
#   define B(x) (((x) >> 8) & 0xFF)
#   define C(x) (((x) >> 16) & 0xFF)
#   define D(x) ((x) >> 24)

#   define S8(x) ((x) >> 8)
#   define S32(x) ((x) >> 32)
#endif


#ifdef WORDS_BIGENDIAN
#   define A1(x) ((x) >> 56)
#else
#   define A1 A
#endif

uint32_t h_crc32(const uint8_t *buf, size_t size, uint32_t crc)
{
    crc = ~crc;

#ifdef WORDS_BIGENDIAN
    crc = bswap_32(crc);
#endif

    if (size > 8) {
        // Fix the alignment, if needed. The if statement above
        // ensures that this won't read past the end of buf[].
        while ((uintptr_t)(buf) & 7) {
            crc = lzma_crc32_table[0][*buf++ ^ A(crc)] ^ S8(crc);
            --size;
        }

        // Calculate the position where to stop.
        const uint8_t *const limit = buf + (size & ~(size_t)(7));

        // Calculate how many bytes must be calculated separately
        // before returning the result.
        size &= (size_t)(7);

        // Calculate the CRC32 using the slice-by-eight algorithm.
        // It is explained in this document:
        // http://www.intel.com/technology/comms/perfnet/download/CRC_generators.pdf
        //
        // The code below is different than the code in Intel's
        // paper, but the principle is identical. This should be
        // faster with GCC than Intel's code. This is tested only
        // with GCC 3.4.6 and 4.1.2 on x86, so your results may vary.
        //
        // Using -Os and -fomit-frame-pointer seem to give the best
        // results at least with GCC 4.1.2 on x86. It's sill far
        // from the speed of hand-optimized assembler.
        while (buf < limit) {
            crc ^= *(const uint32_t *)(buf);
            buf += 4;

            crc = lzma_crc32_table[7][A(crc)]
                ^ lzma_crc32_table[6][B(crc)]
                ^ lzma_crc32_table[5][C(crc)]
                ^ lzma_crc32_table[4][D(crc)];

            const uint32_t tmp = *(const uint32_t *)(buf);
            buf += 4;

            // It is critical for performance, that
            // the crc variable is XORed between the
            // two table-lookup pairs.
            crc = lzma_crc32_table[3][A(tmp)]
                ^ lzma_crc32_table[2][B(tmp)]
                ^ crc
                ^ lzma_crc32_table[1][C(tmp)]
                ^ lzma_crc32_table[0][D(tmp)];
        }
    }

    while (size-- != 0)
        crc = lzma_crc32_table[0][*buf++ ^ A(crc)] ^ S8(crc);

#ifdef WORDS_BIGENDIAN
    crc = bswap_32(crc);
#endif

    return ~crc;
}
uint64_t h_crc64(const uint8_t *buf, size_t size, uint64_t crc)
{
    crc = ~crc;

#ifdef WORDS_BIGENDIAN
    crc = bswap_64(crc);
#endif

    if (size > 4) {
        while ((uintptr_t)(buf) & 3) {
            crc = lzma_crc64_table[0][*buf++ ^ A1(crc)] ^ S8(crc);
            --size;
        }

        const uint8_t *const limit = buf + (size & ~(size_t)(3));
        size &= (size_t)(3);

        // Calculate the CRC64 using the slice-by-four algorithm.
        //
        // In contrast to CRC32 code, this one seems to be fastest
        // with -O3 -fomit-frame-pointer.
        while (buf < limit) {
#ifdef WORDS_BIGENDIAN
            const uint32_t tmp = (crc >> 32) ^ *(const uint32_t *)(buf);
#else
            const uint32_t tmp = crc ^ *(const uint32_t *)(buf);
#endif
            buf += 4;

            // It is critical for performance, that
            // the crc variable is XORed between the
            // two table-lookup pairs.
            crc = lzma_crc64_table[3][A(tmp)]
                ^ lzma_crc64_table[2][B(tmp)]
                ^ S32(crc)
                ^ lzma_crc64_table[1][C(tmp)]
                ^ lzma_crc64_table[0][D(tmp)];
        }
    }

    while (size-- != 0)
        crc = lzma_crc64_table[0][*buf++ ^ A1(crc)] ^ S8(crc);

#ifdef WORDS_BIGENDIAN
    crc = bswap_64(crc);
#endif

    return ~crc;
}

