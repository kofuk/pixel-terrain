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

        nbt::TagCompound *get_section (unsigned char y);
        vector<string> *get_palette (nbt::TagCompound *section);

    public:
        uint64_t last_update;

        Chunk (nbt::NBTFile *nbt_data);
        ~Chunk ();
        void parse_palette ();
        /* You MUST call parse_palette() beforehand. */
        string get_block (int32_t x, int32_t y, int32_t z);
        /* You MUST call parse_palette() beforehand. */
        int get_max_height ();
    };
} // namespace anvil

#endif
