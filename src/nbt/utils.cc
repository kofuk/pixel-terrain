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
        -> std::vector<std::uint8_t> * {
        int z_ret;
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;
        std::array<std::uint8_t, ZLIB_IO_BUF_SIZE> out;

        z_ret = inflateInit(&strm);
        if (z_ret != Z_OK) {
            return nullptr;
        }

        auto *all_out = new std::vector<std::uint8_t>;

        strm.avail_in = len;
        strm.next_in = data;

        do {
            strm.avail_out = ZLIB_IO_BUF_SIZE;
            strm.next_out = out.data();

            z_ret = inflate(&strm, Z_NO_FLUSH);

            if (z_ret == Z_STREAM_ERROR || z_ret == Z_NEED_DICT ||
                z_ret == Z_DATA_ERROR || z_ret == Z_MEM_ERROR) {
                inflateEnd(&strm);

                delete all_out;
                return nullptr;
            }

            all_out->insert(all_out->end(), out.begin(),
                            out.begin() + ZLIB_IO_BUF_SIZE - strm.avail_out);
        } while (strm.avail_out == 0);

        if (z_ret != Z_STREAM_END) {
            delete all_out;
            return nullptr;
        }

        inflateEnd(&strm);

        return all_out;
    }

    auto gzip_file_decompress(std::filesystem::path const &path)
        -> std::vector<std::uint8_t> * {
        ::gzFile in;
#ifdef OS_WIN
        in = ::gzopen_w(path.c_str(), "r");
#else
        in = ::gzopen(path.c_str(), "r");
#endif
        if (in == nullptr) {
            return nullptr;
        }

        auto *all_out = new std::vector<std::uint8_t>;
        std::array<std::uint8_t, ZLIB_IO_BUF_SIZE> buf;
        std::size_t nread;

        while ((nread = ::gzread(in, buf.data(), ZLIB_IO_BUF_SIZE)) != 0) {
            all_out->insert(all_out->cend(), buf.begin(), buf.begin() + nread);
        }

        ::gzclose(in);

        return all_out;
    }
} // namespace pixel_terrain::nbt::utils
