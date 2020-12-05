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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Construct block-color mappng from embeded data.
   And decide that the block should be overridden by per-biome color. */

#include <cstdint>
#include <cstring>
#include <string>

#include "blocks.hh"

#include "block/block_colors_data.hh"

namespace pixel_terrain::image {
    std::unordered_map<std::string_view, std::uint32_t> colors;

    void init_block_list() {
        std::size_t off = 0;
        for (;;) {
            std::size_t len =
                std::strlen(reinterpret_cast<char const *>(block_colors_data) + off);
            if (len == 0) {
                break;
            }
            std::string_view block_name(
                reinterpret_cast<char const *>(block_colors_data) + off, len);
            off += len + 1;
            colors[block_name] =
                *reinterpret_cast<std::uint32_t const *>(block_colors_data + off);
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
