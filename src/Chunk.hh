#ifndef CHUNK_HH
#define CHUNK_HH

#include <cstdint>
#include <vector>

#include "NBT.hh"

using namespace std;

namespace Anvil {
    class Chunk {
        int32_t version;
        NBT::NBTFile *nbt_file;
        NBT::TagCompound *data;
        int32_t x;
        int32_t z;
        array<vector<string> *, 16> palettes;

        void parse_palette ();
        NBT::TagCompound *get_section (unsigned char y);
        vector<string> *get_palette (NBT::TagCompound *section);

    public:
        uint64_t last_update;

        Chunk (NBT::NBTFile *nbt_data);
        ~Chunk ();
        string get_block (int32_t x, int32_t y, int32_t z);
    };
} // namespace Anvil

#endif
