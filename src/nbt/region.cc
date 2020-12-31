// SPDX-License-Identifier: MIT

/* Class to access a mca file.
   Implementation is based on matcool/anvil-parser. */

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>

#include <fcntl.h>
#include <utility>

#include "nbt/constants.hh"
#include "nbt/file.hh"
#include "nbt/region.hh"
#include "nbt/utils.hh"

namespace pixel_terrain::anvil {
    region::region(std::filesystem::path const &filename) {
        data = new file<unsigned char>(filename);
        len = data->size();
    }

    region::region(std::filesystem::path const &filename,
                   std::filesystem::path const &journal_dir) {
        data = new file<unsigned char>(filename);
        len = data->size();

        std::filesystem::path journal_path(journal_dir);
        journal_path /=
            std::filesystem::path(filename).filename().concat(".ptcache");
        try {
            last_update =
                new file<std::uint64_t>(journal_path,
                                        nbt::biomes::CHUNK_PER_REGION_WIDTH *
                                            nbt::biomes::CHUNK_PER_REGION_WIDTH,
                                        "r+");
        } catch (...) {
            delete data;
            std::rethrow_exception(std::current_exception());
        }
    }

    region::~region() {
        delete last_update;
        delete data;
    }

    auto region::header_offset(int chunk_x, int chunk_z) -> std::size_t {
        return 4 * (chunk_x % 32 + chunk_z % 32 * 32); // NOLINT
    }

    auto region::chunk_location_off(int chunk_x, int chunk_z) -> std::size_t {
        std::size_t b_off = header_offset(chunk_x, chunk_z);

        if (b_off + 2 >= len) {
            return 0;
        }

        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        unsigned char buf[] = {0, (*data)[b_off], (*data)[b_off + 1],
                               (*data)[b_off + 2]};

        return nbt::utils::to_host_byte_order(
            *reinterpret_cast<std::int32_t *>(buf));
    }

    auto region::chunk_location_sectors(int chunk_x, int chunk_z)
        -> std::size_t {
        std::size_t b_off = header_offset(chunk_x, chunk_z);

        if (b_off + 3 >= len) {
            return 0;
        }

        return (*data)[b_off + 3];
    }

    auto region::chunk_data(int chunk_x, int chunk_z)
        -> std::pair<std::shared_ptr<unsigned char[]>, std::size_t> { // NOLINT
        std::size_t location_off = chunk_location_off(chunk_x, chunk_z);
        std::size_t location_sec = chunk_location_sectors(chunk_x, chunk_z);
        if (location_off == 0 && location_sec == 0) {
            return std::make_pair(nullptr, 0);
        }

        location_off *= 4096; // NOLINT

        if (location_off + 4 >= len) {
            return std::make_pair(nullptr, 0);
        }

        std::int32_t length =
            nbt::utils::to_host_byte_order(*reinterpret_cast<std::int32_t *>(
                data->get_raw_data() + location_off));

        if (location_off + 5 + length - 1 > len) { // NOLINT
            return std::make_pair(nullptr, 0);
        }

        int compression = (*data)[location_off + 4];
        if (compression == 1) {
            return std::make_pair(nullptr, 0);
        }

        return nbt::utils::zlib_decompress(
            data->get_raw_data() + location_off + 5, length - 1); // NOLINT
    }

    auto region::get_chunk(int chunk_x, int chunk_z) -> chunk * {
        auto [data, len] = chunk_data(chunk_x, chunk_z);

        if (data.get() == nullptr) {
            return nullptr;
        }

        auto *cur_chunk = new chunk(data, len);
        return cur_chunk;
    }

    auto region::exists_chunk_data(int chunk_x, int chunk_z) -> bool {
        return chunk_location_off(chunk_x, chunk_z) == 0 &&
               chunk_location_sectors(chunk_x, chunk_z) == 0;
    }

    auto region::get_chunk_if_dirty(int chunk_x, int chunk_z) -> chunk * {
        auto [data, len] = chunk_data(chunk_x, chunk_z);
        if (data.get() == nullptr) {
            return nullptr;
        }

        auto *cur_chunk = new chunk(data, len);
        if (last_update != nullptr) {
            // NOLINTNEXTLINE
            if ((*last_update)[chunk_z * 32 + chunk_x] >=
                cur_chunk->get_last_update()) {
                delete cur_chunk;
                return nullptr;
            }

            // NOLINTNEXTLINE
            (*last_update)[chunk_z * 32 + chunk_x] =
                cur_chunk->get_last_update();
        }

        return cur_chunk;
    }
} // namespace pixel_terrain::anvil
