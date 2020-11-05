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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <algorithm>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include <zlib.h>

#include "utils.hh"

namespace pixel_terrain::nbt::utils {
    std::pair<std::shared_ptr<unsigned char[]>, std::size_t>
    zlib_decompress(unsigned char *data, std::size_t const len) {
        int z_ret;
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;
        unsigned char out[1024];

        z_ret = inflateInit(&strm);
        if (z_ret != Z_OK) return std::make_pair(nullptr, 0);

        std::vector<unsigned char> all_out;
        all_out.reserve(len * 2);

        strm.avail_in = len;
        strm.next_in = data;

        do {
            strm.avail_out = 1024;
            strm.next_out = out;

            z_ret = inflate(&strm, Z_NO_FLUSH);

            if (z_ret == Z_STREAM_ERROR || z_ret == Z_NEED_DICT ||
                z_ret == Z_DATA_ERROR || z_ret == Z_MEM_ERROR) {
                inflateEnd(&strm);

                return std::make_pair(nullptr, 0);
            }

            all_out.insert(end(all_out), out, out + 1024 - strm.avail_out);
        } while (strm.avail_out == 0);

        if (z_ret != Z_STREAM_END) {
            throw std::out_of_range(
                "buffer exausted before stream end of zlib");
        }

        inflateEnd(&strm);

        unsigned char *dd_out = new unsigned char[all_out.size()];
        copy(begin(all_out), end(all_out), dd_out);

        return std::make_pair(
            std::shared_ptr<unsigned char[]>(
                dd_out, [](unsigned char *data) { delete[] data; }),
            all_out.size());
    }

    std::tuple<int, int> parse_region_file_path(
        std::filesystem::path const &file_path) noexcept(false) {
        std::string filename = file_path.filename().string();
        if (filename.empty()) {
            throw std::invalid_argument("Invalid file path");
        }

        std::vector<std::string> elements;
        std::size_t prev = 0;
        for (std::size_t i = 0; i < filename.size(); ++i) {
            if (filename[i] == '.') {
                elements.push_back(filename.substr(prev, i - prev));
                prev = i + 1;
            }
        }
        if (prev <= filename.size()) {
            elements.push_back(filename.substr(prev));
        }

        if (elements.size() != 4 || elements[0] != "r" ||
            elements[3] != "mca") {
            throw std::invalid_argument("Invalid region filename format");
        }

        std::size_t idx;
        int x = std::stoi(elements[1], &idx);
        if (idx != elements[1].size()) {
            throw std::invalid_argument("Invalid region x axis");
        }
        int z = std::stoi(elements[2], &idx);
        if (idx != elements[2].size()) {
            throw std::invalid_argument("Invalid region z axis");
        }

        return {x, z};
    }
} // namespace pixel_terrain::nbt::utils
