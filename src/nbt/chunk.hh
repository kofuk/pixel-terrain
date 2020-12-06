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

#ifndef CHUNK_HH
#define CHUNK_HH

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "nbt/pull_parser/nbt_pull_parser.hh"

namespace pixel_terrain::anvil {
    class chunk {
        nbt::nbt_pull_parser parser;

        std::array<std::vector<std::string> *, 16> palettes;
        std::array<std::vector<std::uint64_t> *, 16> block_states;
        std::vector<std::int32_t> biomes;
        std::uint64_t last_update;
        std::int32_t data_version;
        unsigned char loaded_fields = 0;
        std::vector<std::string> tag_structure;

        static constexpr unsigned char FIELD_SECTIONS = 1;
        static constexpr unsigned char FIELD_LAST_UPDATE = 1 << 1;
        static constexpr unsigned char FIELD_BIOMES = 1 << 2;
        static constexpr unsigned char FIELD_DATA_VERSION = 1 << 3;

        void parse_fields();
        void parse_sections();
        unsigned char current_field();
        void make_sure_field_parsed(unsigned char field) noexcept(false);

    public:
        chunk(std::shared_ptr<unsigned char[]> data, std::size_t length);
        ~chunk();

        std::uint64_t get_last_update() noexcept(false);
        std::vector<std::string> *get_palette(unsigned char y);
        std::string get_block(std::int32_t x, std::int32_t y, std::int32_t z);
        std::int32_t get_biome(std::int32_t x, std::int32_t y, std::int32_t z);
        int get_max_height();
    };
} // namespace pixel_terrain::anvil

#endif
