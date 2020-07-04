#ifndef CHUNK_HH
#define CHUNK_HH

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "pull_parser/nbt_pull_parser.hh"

namespace pixel_terrain::anvil {
    class chunk {
        nbt::nbt_pull_parser parser;

        std::array<std::vector<std::string> *, 16> palettes;
        std::array<std::vector<std::uint64_t> *, 16> block_states;
        std::vector<std::int32_t> biomes;
        std::uint64_t last_update;
        unsigned char loaded_fields = 0;
        std::vector<std::string> tag_structure;

        static constexpr unsigned char FIELD_SECTIONS = 1;
        static constexpr unsigned char FIELD_LAST_UPDATE = 1 << 1;
        static constexpr unsigned char FIELD_BIOMES = 1 << 2;

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
