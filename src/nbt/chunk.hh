// SPDX-License-Identifier: MIT

#ifndef CHUNK_HH
#define CHUNK_HH

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "nbt/constants.hh"
#include "nbt/pull_parser/nbt_pull_parser.hh"

namespace pixel_terrain::anvil {
    class chunk {
        nbt::nbt_pull_parser parser;
        std::vector<std::uint8_t> *chunk_data_;

        std::array<std::vector<std::string> *, nbt::biomes::PALETTE_Y_MAX>
            palettes;
        std::array<std::vector<std::uint64_t> *,
                   nbt::biomes::BLOCK_STATES_COUNT>
            block_states;
        std::vector<std::int32_t> biomes;
        std::uint64_t last_update;
        std::int32_t data_version;
        unsigned char loaded_fields = 0;
        std::vector<std::string> tag_structure;

        static inline constexpr unsigned char FIELD_SECTIONS = 1;
        static inline constexpr unsigned char FIELD_LAST_UPDATE = 1 << 1;
        static inline constexpr unsigned char FIELD_BIOMES = 1 << 2;
        static inline constexpr unsigned char FIELD_DATA_VERSION = 1 << 3;

        void parse_fields();
        void parse_sections();
        auto current_field() -> unsigned char;
        void make_sure_field_parsed(unsigned char field) noexcept(false);

    public:
        chunk(std::vector<std::uint8_t> *data);
        ~chunk();

        [[nodiscard]] auto get_last_update() noexcept(false) -> std::uint64_t;
        [[nodiscard]] auto get_palette(unsigned char y)
            -> std::vector<std::string> *;
        [[nodiscard]] auto get_block(std::int32_t x, std::int32_t y,
                                     std::int32_t z) -> std::string;
        [[nodiscard]] auto get_biome(std::int32_t x, std::int32_t y,
                                     std::int32_t z) -> std::int32_t;
        [[nodiscard]] auto get_max_height() -> int;
    };
} // namespace pixel_terrain::anvil

#endif
