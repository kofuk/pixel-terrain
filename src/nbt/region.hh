#ifndef REGION_HH
#define REGION_HH

#include <memory>
#include <string>

#include "chunk.hh"
#include "file.hh"
#include "nbt.hh"

namespace pixel_terrain::anvil {
    class region {
        unique_ptr<file<unsigned char>> data;
        size_t len;
        unique_ptr<file<uint64_t>> last_update;

        size_t header_offset(int chunk_x, int chunk_z);
        size_t chunk_location_off(int chunk_x, int chunk_z);
        size_t chunk_location_sectors(int chunk_x, int chunk_z);
        nbt::nbt_file *chunk_data(int chunk_x, int chunk_z);

    public:
        /* Construct new region object from given buffer of *.mca file content
         */
        region(string file_name);
        region(string file_name, string journal_dir);
        chunk *get_chunk(int chunk_x, int chunk_z);
        chunk *get_chunk_if_dirty(int chunk_x, int chunk_z);
    };
} // namespace pixel_terrain::anvil

#endif
