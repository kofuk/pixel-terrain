#ifndef CHUNK_HH
#define CHUNK_HH

#include <cstdint>
#include <vector>

#include "nbt.hh"

using namespace std;

namespace pixel_terrain::anvil {
    class chunk {
        nbt::nbt_file *nbt_file;
        nbt::tag_compound *data;
        array<vector<string> *, 16> palettes;
        vector<int32_t> biomes;

        nbt::tag_compound *get_section(unsigned char y);
        vector<string> *get_palette(nbt::tag_compound *section);

    public:
        uint64_t last_update;

        chunk(nbt::nbt_file *nbt_data);
        ~chunk();
        void parse_fields();
        /* ===== You MUST call parse_fields() beforehand. ===== */
        string get_block(int32_t x, int32_t y, int32_t z);
        int32_t get_biome(int32_t x, int32_t y, int32_t z);
        int get_max_height();
    };
} // namespace pixel_terrain::anvil

#endif
