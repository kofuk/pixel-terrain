#ifndef REGION_HH
#define REGION_HH

#include <memory>
#include <string>

#include "Chunk.hh"
#include "File.hh"
#include "nbt.hh"

namespace anvil {
    class Region {
        unsigned char *data;
        size_t len;
        unique_ptr<File<uint64_t>> last_update;

        void read_region_file (string filename);

        size_t header_offset (int chunk_x, int chunk_z);
        size_t chunk_location_off (int chunk_x, int chunk_z);
        size_t chunk_location_sectors (int chunk_x, int chunk_z);
        nbt::NBTFile *chunk_data (int chunk_x, int chunk_z);

    public:
        /* Construct new region object from given buffer of *.mca file content
         */
        Region (string file_name);
        Region (string file_name, string journal_dir);
        ~Region ();
        Chunk *get_chunk (int chunk_x, int chunk_z);
        Chunk *get_chunk_if_dirty (int chunk_x, int chunk_z);
    };
} // namespace anvil

#endif
