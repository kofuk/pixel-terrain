/*
 * Copyright (c) 2020 Koki Fukuda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef UTILS_HH
#define UTILS_HH

#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

namespace pixel_terrain::nbt::utils {
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

    [[maybe_unused]] static inline std::uint64_t
    to_host_byte_order(std::uint64_t src) {
        reorder_8((unsigned char *)&src);

        return src;
    }

    [[maybe_unused]] static inline std::int32_t
    to_host_byte_order(std::int32_t src) {
        reorder_4((unsigned char *)&src);

        return src;
    }

    [[maybe_unused]] static inline std::int16_t
    to_host_byte_order(std::int16_t src) {
        reorder_2((unsigned char *)&src);

        return src;
    }

    [[maybe_unused]] static inline double to_host_byte_order(double src) {
        reorder_8((unsigned char *)&src);

        return src;
    }

    [[maybe_unused]] static inline float to_host_byte_order(float src) {
        reorder_4((unsigned char *)&src);

        return src;
    }

    std::pair<std::shared_ptr<unsigned char[]>, std::size_t>
    zlib_decompress(unsigned char *data, std::size_t const len);
} // namespace pixel_terrain::nbt::utils

#endif
