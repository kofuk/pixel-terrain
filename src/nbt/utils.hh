// SPDX-License-Identifier: MIT

#ifndef UTILS_HH
#define UTILS_HH

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

namespace pixel_terrain::nbt::utils {
    static inline void swap_chars(std::uint8_t *a, std::uint8_t *b) {
        *a ^= *b;
        *b ^= *a;
        *a ^= *b;
    }

    static inline void reorder_8(std::uint8_t *src) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        swap_chars(src, src + 7);     // NOLINT(readability-magic-numbers)
        swap_chars(src + 1, src + 6); // NOLINT(readability-magic-numbers)
        swap_chars(src + 2, src + 5); // NOLINT(readability-magic-numbers)
        swap_chars(src + 3, src + 4); // NOLINT(readability-magic-numbers)
#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#warning "PDP endian is not tested."
        swap_chars(src, src + 1);
        swap_chars(src + 2, src + 3);
        swap_chars(src + 4, src + 5);
        swap_chars(src + 6, src + 7);
#endif
    }

    static inline void reorder_4(std::uint8_t *src) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        swap_chars(src, src + 3);
        swap_chars(src + 1, src + 2);
#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#warning "PDP endian is not tested."
        swap_chars(src, src + 1);
        swap_chars(src + 2, src + 3);
#endif
    }

    static inline void reorder_2(std::uint8_t *src) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        swap_chars(src, src + 1);
#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#warning "PDP endian is not tested."
#endif
    }

    [[maybe_unused]] static inline auto to_host_byte_order(std::uint64_t src)
        -> std::uint64_t {
        std::array<std::uint8_t, sizeof(std::uint64_t)> as_bytes;
        reorder_8((unsigned char *)std::memcpy(as_bytes.data(), &src,
                                               sizeof(std::uint64_t)));
        std::memcpy(&src, as_bytes.data(), sizeof(std::uint64_t));

        return src;
    }

    [[maybe_unused]] static inline auto to_host_byte_order(std::int32_t src)
        -> std::int32_t {
        std::array<std::uint8_t, sizeof(std::int32_t)> as_bytes;
        reorder_4(static_cast<std::uint8_t *>(
            std::memcpy(as_bytes.data(), &src, sizeof(std::int32_t))));
        std::memcpy(&src, as_bytes.data(), sizeof(std::int32_t));

        return src;
    }

    [[maybe_unused]] static inline auto to_host_byte_order(std::int16_t src)
        -> std::int16_t {
        std::array<std::uint8_t, sizeof(std::int16_t)> as_bytes;
        reorder_2(static_cast<std::uint8_t *>(
            std::memcpy(as_bytes.data(), &src, sizeof(std::int16_t))));
        std::memcpy(&src, as_bytes.data(), sizeof(std::int16_t));

        return src;
    }

    [[maybe_unused]] static inline auto to_host_byte_order(double src)
        -> double {
        std::array<std::uint8_t, sizeof(double)> as_bytes;
        reorder_8(static_cast<std::uint8_t *>(
            std::memcpy(as_bytes.data(), &src, sizeof(double))));
        std::memcpy(&src, as_bytes.data(), sizeof(double));

        return src;
    }

    [[maybe_unused]] static inline auto to_host_byte_order(float src) -> float {
        std::array<std::uint8_t, sizeof(float)> as_bytes;
        reorder_4(static_cast<std::uint8_t *>(
            std::memcpy(as_bytes.data(), &src, sizeof(float))));
        std::memcpy(&src, as_bytes.data(), sizeof(float));

        return src;
    }

    auto zlib_decompress(std::uint8_t *data, std::size_t len)
        -> std::vector<std::uint8_t> *;
    auto gzip_file_decompress(std::filesystem::path const &path)
        -> std::vector<std::uint8_t> *;
} // namespace pixel_terrain::nbt::utils

#endif
