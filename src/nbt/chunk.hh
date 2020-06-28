#ifndef CHUNK_HH
#define CHUNK_HH

#include <array>
#include <cstdint>
#include <vector>

#include "nbt.hh"

namespace pixel_terrain::anvil {
    class chunk {
        nbt::nbt_file *nbt_file;
        nbt::tag_compound *data;
        std::array<std::vector<std::string> *, 16> palettes;
        std::vector<std::int32_t> biomes;

        nbt::tag_compound *get_section(unsigned char y);
        std::vector<std::string> *get_palette(nbt::tag_compound *section);

    public:
        uint64_t last_update;

        chunk(nbt::nbt_file *nbt_data);
        ~chunk();
        void parse_fields();
        /* ===== You MUST call parse_fields() beforehand. ===== */
        std::string get_block(std::int32_t x, std::int32_t y, std::int32_t z);
        std::int32_t get_biome(std::int32_t x, std::int32_t y, std::int32_t z);
        int get_max_height();
    };
} // namespace pixel_terrain::anvil

#endif
