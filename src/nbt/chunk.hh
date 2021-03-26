// SPDX-License-Identifier: MIT

#ifndef CHUNK_HH
#define CHUNK_HH

#include <array>
#include <cstdint>
#include <exception>
#include <memory>
#include <stdexcept>
#include <vector>

#include "nbt/constants.hh"
#if USE_V3_NBT_PARSER
#include "nbt/nbt.hh"
#else
#include "nbt/pull_parser/nbt_pull_parser.hh"
#endif

namespace pixel_terrain::anvil {
    class chunk_exception : public std::runtime_error {
    public:
        chunk_exception(std::string const &msg) : std::runtime_error(msg) {}
    };

    class chunk_parse_error : public chunk_exception {
    public:
        chunk_parse_error(std::string const &msg) : chunk_exception(msg) {}
    };

    class not_generated_chunk_error : public chunk_exception {
    public:
        not_generated_chunk_error(std::string const &msg)
            : chunk_exception(msg) {}
    };

    class broken_chunk_error : public chunk_exception {
    public:
        broken_chunk_error(std::string const &msg) : chunk_exception(msg) {}
    };

    class chunk {
#if !USE_V3_NBT_PARSER
        nbt::nbt_pull_parser parser;
        std::vector<std::uint8_t> *chunk_data_;
#endif

#if USE_BLOCK_LIGHT_DATA
        std::array<std::vector<std::uint8_t>, nbt::biomes::BLOCK_STATES_COUNT>
            block_lights_;
#endif

        std::array<std::vector<std::string> *, nbt::biomes::PALETTE_Y_MAX>
            palettes;
        std::array<std::vector<std::uint64_t> *,
                   nbt::biomes::BLOCK_STATES_COUNT>
            block_states;
        std::vector<std::int32_t> biomes;
        std::uint64_t last_update;
        std::int32_t data_version;
#if USE_V3_NBT_PARSER
        void init_fields(nbt::nbt const &nbt_file) noexcept(false);
#else
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
#endif

    public:
#if USE_V3_NBT_PARSER
        chunk(std::vector<std::uint8_t> const &data);
#else
        chunk(std::vector<std::uint8_t> *data);
#endif
        ~chunk();

        [[nodiscard]] auto get_last_update() noexcept(false) -> std::uint64_t;
        [[nodiscard]] auto get_palette(unsigned char y)
            -> std::vector<std::string> *;
        [[nodiscard]] auto get_block(std::int32_t x, std::int32_t y,
                                     std::int32_t z) -> std::string;
        [[nodiscard]] auto get_biome(std::int32_t x, std::int32_t y,
                                     std::int32_t z) -> std::int32_t;
        [[nodiscard]] auto get_max_height() -> int;

#if USE_BLOCK_LIGHT_DATA
        [[nodiscard]] auto get_block_light(std::int32_t x, std::int32_t y,
                                           std::int32_t z) -> std::uint8_t;
#endif
    };
} // namespace pixel_terrain::anvil

#endif
