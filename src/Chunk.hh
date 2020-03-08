#ifndef CHUNK_HH
#define CHUNK_HH

#include <cstdint>
#include <vector>

#include "nbt.hh"

using namespace std;

namespace anvil {
    class Chunk {
        nbt::NBTFile *nbt_file;
        nbt::TagCompound *data;
        array<vector<string> *, 16> palettes;
        vector<int32_t> biomes;

        nbt::TagCompound *get_section (unsigned char y);
        vector<string> *get_palette (nbt::TagCompound *section);

    public:
        uint64_t last_update;

        Chunk (nbt::NBTFile *nbt_data);
        ~Chunk ();
        void parse_fields ();
        /* ===== You MUST call parse_fields() beforehand. ===== */
        string get_block (int32_t x, int32_t y, int32_t z);
        int32_t get_biome (int32_t x, int32_t y, int32_t z);
        int get_max_height ();
    };
} // namespace anvil

#endif
