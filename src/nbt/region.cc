/* Class to access a mca file.
   Implementation is based on matcool/anvil-parser. */

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>

#include <fcntl.h>

#include "file.hh"
#include "nbt.hh"
#include "region.hh"
#include "utils.hh"

namespace pixel_terrain::anvil {
    region::region(string file_name) {
        data =
            unique_ptr<file<unsigned char>>(new file<unsigned char>(file_name));
        len = data->size();
    }

    region::region(string filename, string journal_dir) {
        data =
            unique_ptr<file<unsigned char>>(new file<unsigned char>(filename));
        len = data->size();

        filesystem::path journal_path(journal_dir);
        journal_path /=
            filesystem::path(filename).filename().string() + ".journal"s;
        last_update = unique_ptr<file<uint64_t>>(
            new file<uint64_t>(journal_path, 1024, "r+"));
    }

    size_t region::header_offset(int chunk_x, int chunk_z) {
        return 4 * (chunk_x % 32 + chunk_z % 32 * 32);
    }

    size_t region::chunk_location_off(int chunk_x, int chunk_z) {
        size_t b_off = header_offset(chunk_x, chunk_z);

        if (b_off + 2 >= len) return 0;

        unsigned char buf[] = {0, (*data)[b_off], (*data)[b_off + 1],
                               (*data)[b_off + 2]};

        return nbt::utils::to_host_byte_order(
            *reinterpret_cast<int32_t *>(buf));
    }

    size_t region::chunk_location_sectors(int chunk_x, int chunk_z) {
        size_t b_off = header_offset(chunk_x, chunk_z);

        if (b_off + 3 >= len) return 0;

        return (*data)[b_off + 3];
    }

    nbt::nbt_file *region::chunk_data(int chunk_x, int chunk_z) {
        size_t location_off = chunk_location_off(chunk_x, chunk_z);
        size_t location_sec = chunk_location_sectors(chunk_x, chunk_z);
        if (location_off == 0 && location_sec == 0) {
            return nullptr;
        }

        location_off *= 4096;

        if (location_off + 4 >= len) return nullptr;

        int32_t length = nbt::utils::to_host_byte_order(
            *reinterpret_cast<int32_t *>(data->get_raw_data() + location_off));

        if (location_off + 5 + length - 1 > len) return nullptr;

        int compression = (*data)[location_off + 4];
        if (compression == 1) return nullptr;

        nbt::utils::decompressed_data *decompressed =
            nbt::utils::zlib_decompress(data->get_raw_data() + location_off + 5,
                                        length - 1);

        return new nbt::nbt_file(decompressed);
    }

    chunk *region::get_chunk(int chunk_x, int chunk_z) {
        nbt::nbt_file *nbt = chunk_data(chunk_x, chunk_z);

        if (nbt == nullptr) return nullptr;

        chunk *cur_chunk = new chunk(nbt);
        cur_chunk->parse_fields();
        return cur_chunk;
    }

    chunk *region::get_chunk_if_dirty(int chunk_x, int chunk_z) {
        nbt::nbt_file *nbt = chunk_data(chunk_x, chunk_z);
        if (nbt == nullptr) {
            return nullptr;
        }

        chunk *cur_chunk = new chunk(nbt);
        if (last_update.get() != nullptr) {
            if ((*last_update)[chunk_z * 32 + chunk_x] >=
                cur_chunk->last_update) {
                delete cur_chunk;
                return nullptr;
            }

            (*last_update)[chunk_z * 32 + chunk_x] = cur_chunk->last_update;
        }
        cur_chunk->parse_fields();

        return cur_chunk;
    }
} // namespace pixel_terrain::anvil
