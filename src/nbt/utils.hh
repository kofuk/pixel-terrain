// SPDX-License-Identifier: MIT

#ifndef UTILS_HH
#define UTILS_HH

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <tuple>
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
        unsigned char as_bytes[8];
        reorder_8((unsigned char *)std::memcpy(as_bytes, &src, 8));
        std::memcpy(&src, as_bytes, 8);

        return src;
    }

    [[maybe_unused]] static inline std::int32_t
    to_host_byte_order(std::int32_t src) {
        unsigned char as_bytes[4];
        reorder_4((unsigned char *)std::memcpy(as_bytes, &src, 4));
        std::memcpy(&src, as_bytes, 4);

        return src;
    }

    [[maybe_unused]] static inline std::int16_t
    to_host_byte_order(std::int16_t src) {
        unsigned char as_bytes[2];
        reorder_2((unsigned char *)std::memcpy(as_bytes, &src, 2));
        std::memcpy(&src, as_bytes, 2);

        return src;
    }

    [[maybe_unused]] static inline double to_host_byte_order(double src) {
        unsigned char as_bytes[8];
        reorder_8((unsigned char *)std::memcpy(as_bytes, &src, 8));
        std::memcpy(&src, as_bytes, 8);

        return src;
    }

    [[maybe_unused]] static inline float to_host_byte_order(float src) {
        unsigned char as_bytes[4];
        reorder_4((unsigned char *)std::memcpy(as_bytes, &src, 4));
        std::memcpy(&src, as_bytes, 4);

        return src;
    }

    std::pair<std::shared_ptr<unsigned char[]>, std::size_t>
    zlib_decompress(unsigned char *data, std::size_t const len);
    std::pair<std::shared_ptr<std::uint8_t[]>, std::size_t>
    gzip_file_decompress(std::filesystem::path const &path);

    std::tuple<int, int> parse_region_file_path(
        std::filesystem::path const &file_path) noexcept(false);
} // namespace pixel_terrain::nbt::utils

#endif
