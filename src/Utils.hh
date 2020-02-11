#ifndef UTILS_HH
#define UTILS_HH

#include <algorithm>
#include <cstdint>
#include <memory>

using namespace std;

namespace NBT {
    namespace Utils {
        static inline void reorder_8(unsigned char *src) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            swap(src[0], src[7]);
            swap(src[1], src[6]);
            swap(src[2], src[5]);
            swap(src[3], src[4]);
#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#warning "PDP endian is not tested."
            swap(src[0], src[1]);
            swap(src[2], src[3]);
            swap(src[4], src[5]);
            swap(src[6], src[7]);
#endif
        }

        static inline void reorder_4(unsigned char *src) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            swap(src[0], src[3]);
            swap(src[1], src[2]);
#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#warning "PDP endian is not tested."
            swap(src[0], src[1]);
            swap(src[2], src[3]);
#endif
        }

        static inline void reorder_2(unsigned char *src) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            swap(src[0], src[1]);
#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#warning "PDP endian is not tested."
#endif
        }

        static inline int64_t to_host_byte_order(int64_t const src) {
            unsigned char buf[8];
            unsigned char *src_buf = (unsigned char *)&src;
            copy(src_buf, src_buf + 8, buf);
            reorder_8(buf);

            return *(int64_t *)buf;
        }

        static inline int32_t to_host_byte_order(int32_t const src) {
            unsigned char buf[4];
            unsigned char *src_buf = (unsigned char *)&src;
            copy(src_buf, src_buf + 4, buf);
            reorder_4(buf);

            return *(int32_t *)buf;
        }

        static inline int16_t to_host_byte_order(int16_t const src) {
            unsigned char buf[2];
            unsigned char *src_buf = (unsigned char *)&src;
            copy(src_buf, src_buf + 2, buf);
            reorder_2(buf);

            return *(int16_t *)buf;
        }

        static inline double to_host_byte_order(double const src) {
            unsigned char buf[8];
            unsigned char *src_buf = (unsigned char *)&src;
            copy(src_buf, src_buf + 8, buf);
            reorder_8(buf);

            return *(double *)buf;
        }

        static inline float to_host_byte_order(float const src) {
            unsigned char buf[4];
            unsigned char *src_buf = (unsigned char *)&src;
            copy(src_buf, src_buf + 4, buf);
            reorder_4(buf);

            return *(float *)buf;
        }

        struct DecompressedData {
            unsigned char const *data;
            size_t len;

            DecompressedData();
            ~DecompressedData();
        };

        DecompressedData *zlib_decompress(unsigned char *data, size_t const len);
    } // namespace Utils
} // namespace NBT

#endif
