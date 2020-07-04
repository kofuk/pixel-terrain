#ifndef REGION_HH
#define REGION_HH

#include <memory>
#include <string>
#include <utility>

#include "chunk.hh"
#include "file.hh"

namespace pixel_terrain::anvil {
    class region {
        std::unique_ptr<file<unsigned char>> data;
        std::size_t len;
        std::unique_ptr<file<std::uint64_t>> last_update;

        std::size_t header_offset(int chunk_x, int chunk_z);
        std::size_t chunk_location_off(int chunk_x, int chunk_z);
        std::size_t chunk_location_sectors(int chunk_x, int chunk_z);
        std::pair<std::shared_ptr<unsigned char[]>, std::size_t>
        chunk_data(int chunk_x, int chunk_z);

    public:
        /* Construct new region object from given buffer of *.mca file content
         */
        region(std::string file_name);
        region(std::string file_name, std::string journal_dir);
        chunk *get_chunk(int chunk_x, int chunk_z);
        chunk *get_chunk_if_dirty(int chunk_x, int chunk_z);
    };
} // namespace pixel_terrain::anvil

#endif
