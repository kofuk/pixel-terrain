// SPDX-License-Identifier: MIT

#include <algorithm>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include <zlib.h>

#include "nbt/utils.hh"

namespace pixel_terrain::nbt::utils {
    namespace {
        inline constexpr std::size_t ZLIB_IO_BUF_SIZE = 1024;
    }

    auto zlib_decompress(std::uint8_t *data, std::size_t const len)
        -> std::pair<std::shared_ptr<std::uint8_t[]>, std::size_t> { // NOLINT
        int z_ret;
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;
        std::uint8_t out[ZLIB_IO_BUF_SIZE]; // NOLINT

        z_ret = inflateInit(&strm);
        if (z_ret != Z_OK) {
            return std::make_pair(nullptr, 0);
        }

        std::vector<std::uint8_t> all_out;
        all_out.reserve(len * 2);

        strm.avail_in = len;
        strm.next_in = data;

        do {
            strm.avail_out = ZLIB_IO_BUF_SIZE;
            strm.next_out = out;

            z_ret = inflate(&strm, Z_NO_FLUSH);

            if (z_ret == Z_STREAM_ERROR || z_ret == Z_NEED_DICT ||
                z_ret == Z_DATA_ERROR || z_ret == Z_MEM_ERROR) {
                inflateEnd(&strm);

                return std::make_pair(nullptr, 0);
            }

            all_out.insert(end(all_out), out,
                           out + ZLIB_IO_BUF_SIZE - strm.avail_out);
        } while (strm.avail_out == 0);

        if (z_ret != Z_STREAM_END) {
            throw std::out_of_range(
                "buffer exausted before stream end of zlib");
        }

        inflateEnd(&strm);

        auto *dd_out = new std::uint8_t[all_out.size()];
        copy(begin(all_out), end(all_out), dd_out);

        return std::make_pair(
            std::shared_ptr<std::uint8_t[]>( // NOLINT
                dd_out, [](std::uint8_t const *data) { delete[] data; }),
            all_out.size());
    }

    auto gzip_file_decompress(std::filesystem::path const &path)
        -> std::pair<std::shared_ptr<std::uint8_t[]>, std::size_t> { // NOLINT
        gzFile in;
#ifdef OS_WIN
        in = ::gzopen_w(path.c_str(), "r");
#else
        in = ::gzopen(path.c_str(), "r");
#endif
        if (in == nullptr) {
            return std::make_pair(nullptr, 0);
        }

        std::vector<std::uint8_t> all_out;
        std::uint8_t buf[ZLIB_IO_BUF_SIZE]; // NOLINT
        std::size_t nread;

        while ((nread = ::gzread(in, buf, ZLIB_IO_BUF_SIZE)) != 0) {
            all_out.insert(all_out.cend(), buf, buf + nread);
        }

        ::gzclose(in);

        auto *data = new std::uint8_t[all_out.size()];
        std::copy(all_out.cbegin(), all_out.cend(), data);

        return std::make_pair(
            std::shared_ptr<std::uint8_t[]>( // NOLINT
                data, [](std::uint8_t const *data) { delete[] data; }),
            all_out.size());
    }
} // namespace pixel_terrain::nbt::utils
