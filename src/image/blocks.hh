// SPDX-License-Identifier: MIT

#ifndef BLOCKS_HH
#define BLOCKS_HH

#include <array>
#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace pixel_terrain::image {
    extern std::unordered_map<std::string_view, std::uint32_t> colors;

    auto is_biome_overridden(std::string const &block) -> bool;
} // namespace pixel_terrain::image

#endif
