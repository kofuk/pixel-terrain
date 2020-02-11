#ifndef REGION_HH
#define REGION_HH

#include <string>

#include "Chunk.hh"
#include "NBT.hh"

namespace Anvil {
    class Region {
        unsigned char *data;
        size_t len;

        /* Construct new region object from given buffer of *.mca file content
         */
        size_t header_offset(int chunk_x, int chunk_z);
        size_t chunk_location_off(int chunk_x, int chunk_z);
        size_t chunk_location_sectors(int chunk_x, int chunk_z);
        NBT::NBTFile *chunk_data(int chunk_x, int chunk_z);

    public:
        Region(string file_name);
        ~Region();
        Chunk *get_chunk(int chunk_x, int chunk_z);
    };
} // namespace Anvil

#endif
