#ifndef UTILS_HH
#define UTILS_HH

#include <algorithm>
#include <cstdint>
#include <memory>

using namespace std;

namespace NBT {
    namespace Utils {
        static inline void swap_chars(unsigned char *a, unsigned char *b) {
            *a ^= *b;
            *b ^= *a;
            *a ^= *b;
        }

        static inline void reorder_8(unsigned char *src) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            swap_chars(src, src + 7);
            swap_chars(src + 1, src + 6);
            swap_chars(src + 2, src + 5);
            swap_chars(src + 3, src + 4);
#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#warning "PDP endian is not tested."
            swap_chars(src, src + 1);
            swap_chars(src + 2, src + 3);
            swap_chars(src + 4, src + 5);
            swap_chars(src + 6, src + 7);
#endif
        }

        static inline void reorder_4(unsigned char *src) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            swap_chars(src, src + 3);
            swap_chars(src + 1, src + 2);
#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#warning "PDP endian is not tested."
            swap_chars(src, src + 1);
            swap_chars(src + 2, src + 3);
#endif
        }

        static inline void reorder_2(unsigned char *src) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            swap_chars(src, src + 1);
#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#warning "PDP endian is not tested."
#endif
        }

        static inline int64_t to_host_byte_order(int64_t src) {
            reorder_8((unsigned char *)&src);

            return src;
        }

        static inline int32_t to_host_byte_order(int32_t src) {
            reorder_4((unsigned char *)&src);

            return src;
        }

        static inline int16_t to_host_byte_order(int16_t src) {
            reorder_2((unsigned char *)&src);

            return src;
        }

        static inline double to_host_byte_order(double src) {
            reorder_8((unsigned char *)&src);

            return src;
        }

        static inline float to_host_byte_order(float src) {
            reorder_4((unsigned char *)&src);

            return src;
        }

        struct DecompressedData {
            unsigned char const *data;
            size_t len;

            DecompressedData();
            ~DecompressedData();
        };

        DecompressedData *zlib_decompress(unsigned char *data,
                                          size_t const len);
    } // namespace Utils
} // namespace NBT

#endif
