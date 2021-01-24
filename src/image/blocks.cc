// SPDX-License-Identifier: MIT

/* Construct block-color mappng from embeded data.
   And decide that the block should be overridden by per-biome color. */

#include <cstdint>
#include <cstring>
#include <string>

#include "block/block_colors_data.hh"
#include "image/blocks.hh"

namespace pixel_terrain::image {
    namespace {
        using block_color_map =
            std::unordered_map<std::string_view, std::uint32_t>;
    }

    auto init_block_list() -> block_color_map {
        block_color_map result;
        std::size_t off = 0;
        for (;;) {
            auto len = static_cast<std::size_t>(block_colors_data[off]);
            ++off;
            if (len == 0) {
                break;
            }

            std::string_view block_name(
                reinterpret_cast<char const *>(block_colors_data) + off, len);
            off += len;
            off += 1 + static_cast<std::size_t>(block_colors_data[off]);
            std::uint32_t color;
            std::memcpy(&color, block_colors_data + off, sizeof(std::uint32_t));
            result[block_name] = color;
            off += 4;
        }

        return result;
    }

    block_color_map colors = init_block_list();

    auto is_biome_overridden(std::string const &block) -> bool {
        using namespace std::string_literals;

        /* first, check if block is water or grass_block because the most
         * blocks are one of them. */
        if (block == "water" || block == "grass_block") {
            return true;
        }

        /* next, capture grass, large_grass, seagrass and large_seagrass. */
        constexpr std::size_t grass_suffix_len = 5;
        if (block.size() >= grass_suffix_len &&
            block.find("grass") == block.size() - grass_suffix_len) {
            return true;
        }

        /* next, check if block is leaf-family. */
        if (block.find("leaves") != std::string::npos) {
            return true;
        }

        /* capture remains */
        if (block == "fern" || block == "large_fern" || block == "vine" ||
            block == "bubble_column") {
            return true;
        }

        return false;
    }
} // namespace pixel_terrain::image
