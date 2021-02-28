// SPDX-License-Identifier: MIT

#ifndef REGION_HH
#define REGION_HH

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "nbt/chunk.hh"
#include "nbt/file.hh"
#include "utils/path_hack.hh"

namespace pixel_terrain::anvil {
    class region {
        file<unsigned char> *data = nullptr;
        std::size_t len;
        file<std::uint64_t> *last_update = nullptr;

        static auto header_offset(int chunk_x, int chunk_z) -> std::size_t;
        auto chunk_location_off(int chunk_x, int chunk_z) -> std::size_t;
        auto chunk_location_sectors(int chunk_x, int chunk_z) -> std::size_t;

    public:
        /* Construct new region object from given buffer of *.mca file content
         */
        region(std::filesystem::path const &filename);
        region(std::filesystem::path const &filename,
               std::filesystem::path const &journal_dir);
        ~region();
        auto chunk_data(int chunk_x, int chunk_z)
            -> std::vector<std::uint8_t> *;
        auto get_chunk(int chunk_x, int chunk_z) -> chunk *;
        auto get_chunk_if_dirty(int chunk_x, int chunk_z) -> chunk *;
        auto is_chunk_missing(int chunk_x, int chunk_z) -> bool;
    };
} // namespace pixel_terrain::anvil

#endif
