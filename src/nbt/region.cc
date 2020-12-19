// SPDX-License-Identifier: MIT

/* Class to access a mca file.
   Implementation is based on matcool/anvil-parser. */

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>

#include <fcntl.h>
#include <utility>

#include "nbt/file.hh"
#include "nbt/region.hh"
#include "nbt/utils.hh"

namespace pixel_terrain::anvil {
    region::region(std::filesystem::path filename) {
        data = std::unique_ptr<file<unsigned char>>(
            new file<unsigned char>(filename));
        len = data->size();
    }

    region::region(std::filesystem::path filename,
                   std::filesystem::path journal_dir) {
        data = std::unique_ptr<file<unsigned char>>(
            new file<unsigned char>(filename));
        len = data->size();

        std::filesystem::path journal_path(journal_dir);
        journal_path /=
            std::filesystem::path(filename).filename().string() + ".journal";
        last_update = std::unique_ptr<file<uint64_t>>(
            new file<std::uint64_t>(journal_path, 1024, "r+"));
    }

    std::size_t region::header_offset(int chunk_x, int chunk_z) {
        return 4 * (chunk_x % 32 + chunk_z % 32 * 32);
    }

    std::size_t region::chunk_location_off(int chunk_x, int chunk_z) {
        std::size_t b_off = header_offset(chunk_x, chunk_z);

        if (b_off + 2 >= len) return 0;

        unsigned char buf[] = {0, (*data)[b_off], (*data)[b_off + 1],
                               (*data)[b_off + 2]};

        return nbt::utils::to_host_byte_order(
            *reinterpret_cast<std::int32_t *>(buf));
    }

    std::size_t region::chunk_location_sectors(int chunk_x, int chunk_z) {
        std::size_t b_off = header_offset(chunk_x, chunk_z);

        if (b_off + 3 >= len) return 0;

        return (*data)[b_off + 3];
    }

    std::pair<std::shared_ptr<unsigned char[]>, std::size_t>
    region::chunk_data(int chunk_x, int chunk_z) {
        std::size_t location_off = chunk_location_off(chunk_x, chunk_z);
        std::size_t location_sec = chunk_location_sectors(chunk_x, chunk_z);
        if (location_off == 0 && location_sec == 0) {
            return std::make_pair(nullptr, 0);
        }

        location_off *= 4096;

        if (location_off + 4 >= len) return std::make_pair(nullptr, 0);

        std::int32_t length =
            nbt::utils::to_host_byte_order(*reinterpret_cast<std::int32_t *>(
                data->get_raw_data() + location_off));

        if (location_off + 5 + length - 1 > len)
            return std::make_pair(nullptr, 0);

        int compression = (*data)[location_off + 4];
        if (compression == 1) return std::make_pair(nullptr, 0);

        return nbt::utils::zlib_decompress(
            data->get_raw_data() + location_off + 5, length - 1);
    }

    chunk *region::get_chunk(int chunk_x, int chunk_z) {
        auto [data, len] = chunk_data(chunk_x, chunk_z);

        if (data.get() == nullptr) return nullptr;

        chunk *cur_chunk = new chunk(data, len);
        return cur_chunk;
    }

    bool region::exists_chunk_data(int chunk_x, int chunk_z) {
        return !chunk_location_off(chunk_x, chunk_z) &&
               !chunk_location_sectors(chunk_x, chunk_z);
    }

    chunk *region::get_chunk_if_dirty(int chunk_x, int chunk_z) {
        auto [data, len] = chunk_data(chunk_x, chunk_z);
        if (data.get() == nullptr) {
            return nullptr;
        }

        chunk *cur_chunk = new chunk(data, len);
        if (last_update.get() != nullptr) {
            if ((*last_update)[chunk_z * 32 + chunk_x] >=
                cur_chunk->get_last_update()) {
                delete cur_chunk;
                return nullptr;
            }

            (*last_update)[chunk_z * 32 + chunk_x] =
                cur_chunk->get_last_update();
        }

        return cur_chunk;
    }
} // namespace pixel_terrain::anvil
