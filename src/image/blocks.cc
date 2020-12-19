// SPDX-License-Identifier: MIT

/* Construct block-color mappng from embeded data.
   And decide that the block should be overridden by per-biome color. */

#include <cstdint>
#include <cstring>
#include <string>

#include "block/block_colors_data.hh"
#include "image/blocks.hh"

namespace pixel_terrain::image {
    std::unordered_map<std::string_view, std::uint32_t> colors;

    void init_block_list() {
        std::size_t off = 0;
        for (;;) {
            std::size_t len = std::strlen(
                reinterpret_cast<char const *>(block_colors_data) + off);
            if (len == 0) {
                break;
            }
            std::string_view block_name(
                reinterpret_cast<char const *>(block_colors_data) + off, len);
            off += len + 1;
            std::uint32_t color;
            std::memcpy(&color, block_colors_data + off, sizeof(std::uint32_t));
            colors[block_name] = color;
            off += 4;
        }
    }

    bool is_biome_overridden(std::string const &block) {
        /* first, check if block is water or grass_block because the most
         * blocks are one of them. */
        if (block == "water" || block == "grass_block") return true;

        /* next, capture grass, large_grass, seagrass and large_seagrass. */
        if (block.size() >= 5 && block.find("grass") == block.size() - 5)
            return true;

        /* next, check if block is leaf-family. */
        if (block.find("leaves") != std::string::npos) return true;

        /* capture remains */
        if (block == "fern" || block == "large_fern" || block == "vine" ||
            block == "bubble_column")
            return true;

        return false;
    }
} // namespace pixel_terrain::image
