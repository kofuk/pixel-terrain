// SPDX-License-Identifier: MIT

/* Class to access a mca file.
   Implementation is based on matcool/anvil-parser. */

#include <algorithm>
#include <array>
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
#include <vector>

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

        std::array<std::uint8_t, 4> buf;
        buf[0] = 0;
        std::copy(data->get_raw_data() + b_off,
                  data->get_raw_data() + b_off + 3, buf.begin() + 1);

        std::int32_t result;
        std::copy(buf.cbegin(), buf.cend(), &result);

        return result;
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
        -> std::vector<std::uint8_t> * {
        std::size_t location_off = chunk_location_off(chunk_x, chunk_z);
        std::size_t location_sec = chunk_location_sectors(chunk_x, chunk_z);
        if (location_off == 0 && location_sec == 0) {
            return nullptr;
        }

        location_off *= 4096; // NOLINT

        if (location_off >= len) {
            return nullptr;
        }

        std::int32_t length;
        std::memcpy(&length, data->get_raw_data() + location_off,
                    sizeof(std::int32_t));
        length = nbt::utils::to_host_byte_order(length);
        location_off += 4;

        int compression = (*data)[location_off];
        if (compression == 1) {
            return nullptr;
        }
        ++location_off;

        if (location_off + length - 1 > len) {
            return nullptr;
        }

        return nbt::utils::zlib_decompress(data->get_raw_data() + location_off,
                                           length - 1);
    }

    auto region::get_chunk(int chunk_x, int chunk_z) -> chunk * {
        auto data = chunk_data(chunk_x, chunk_z);

        if (data == nullptr) {
            return nullptr;
        }

        auto *cur_chunk = new chunk(data);
        return cur_chunk;
    }

    auto region::exists_chunk_data(int chunk_x, int chunk_z) -> bool {
        return chunk_location_off(chunk_x, chunk_z) == 0 &&
               chunk_location_sectors(chunk_x, chunk_z) == 0;
    }

    auto region::get_chunk_if_dirty(int chunk_x, int chunk_z) -> chunk * {
        std::vector<std::uint8_t> *data = chunk_data(chunk_x, chunk_z);
        if (data == nullptr) {
            return nullptr;
        }

        auto *cur_chunk = new chunk(data);
        if (last_update != nullptr) {
            if ((*last_update)[chunk_z * nbt::biomes::CHUNK_WIDTH + chunk_x] >=
                cur_chunk->get_last_update()) {
                delete cur_chunk;
                return nullptr;
            }

            (*last_update)[chunk_z * nbt::biomes::CHUNK_PER_REGION_WIDTH +
                           chunk_x] = cur_chunk->get_last_update();
        }

        return cur_chunk;
    }
} // namespace pixel_terrain::anvil
