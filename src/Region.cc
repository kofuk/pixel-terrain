#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "File.hh"
#include "Region.hh"
#include "nbt.hh"
#include "utils.hh"
#include "worker.hh"

namespace anvil {
    Region::Region (string file_name) {
        data = unique_ptr<File<unsigned char>> (
            new File<unsigned char> (file_name));
        len = data->size ();
    }

    Region::Region (string filename, string journal_dir) {
        data = unique_ptr<File<unsigned char>> (
            new File<unsigned char> (filename));
        len = data->size ();

        filesystem::path journal_path (journal_dir);
        journal_path /=
            filesystem::path (filename).filename ().string () + ".journal";
        last_update = unique_ptr<File<uint64_t>> (
            new File<uint64_t> (journal_path, 1024));
    }

    size_t Region::header_offset (int chunk_x, int chunk_z) {
        return 4 * (chunk_x % 32 + chunk_z % 32 * 32);
    }

    size_t Region::chunk_location_off (int chunk_x, int chunk_z) {
        size_t b_off = header_offset (chunk_x, chunk_z);

        if (b_off + 2 >= len) return 0;

        unsigned char buf[] = {0, (*data)[b_off], (*data)[b_off + 1],
                               (*data)[b_off + 2]};

        return nbt::utils::to_host_byte_order (
            *reinterpret_cast<int32_t *> (buf));
    }

    size_t Region::chunk_location_sectors (int chunk_x, int chunk_z) {
        size_t b_off = header_offset (chunk_x, chunk_z);

        if (b_off + 3 >= len) return 0;

        return (*data)[b_off + 3];
    }

    nbt::NBTFile *Region::chunk_data (int chunk_x, int chunk_z) {
        size_t location_off = chunk_location_off (chunk_x, chunk_z);
        size_t location_sec = chunk_location_sectors (chunk_x, chunk_z);
        if (location_off == 0 && location_sec == 0) {
            return nullptr;
        }

        location_off *= 4096;

        if (location_off + 4 >= len) return nullptr;

        int32_t length =
            nbt::utils::to_host_byte_order (*reinterpret_cast<int32_t *> (
                data->get_raw_data () + location_off));

        if (location_off + 5 + length - 1 > len) return nullptr;

        int compression = (*data)[location_off + 4];
        if (compression == 1) return nullptr;

        nbt::utils::DecompressedData *decompressed =
            nbt::utils::zlib_decompress (
                data->get_raw_data () + location_off + 5, length - 1);

        return new nbt::NBTFile (decompressed);
    }

    Chunk *Region::get_chunk (int chunk_x, int chunk_z) {
        nbt::NBTFile *nbt = chunk_data (chunk_x, chunk_z);

        if (nbt == nullptr) return nullptr;

        Chunk *chunk = new Chunk (nbt);
        chunk->parse_fields ();
        return chunk;
    }

    Chunk *Region::get_chunk_if_dirty (int chunk_x, int chunk_z) {
        nbt::NBTFile *nbt = chunk_data (chunk_x, chunk_z);
        if (nbt == nullptr) {
            return nullptr;
        }

        Chunk *chunk = new Chunk (nbt);
        if (last_update.get () != nullptr) {
            if ((*last_update)[chunk_z * 32 + chunk_x] >= chunk->last_update) {
                delete chunk;
                return nullptr;
            }

            (*last_update)[chunk_z * 32 + chunk_x] = chunk->last_update;
        }
        chunk->parse_fields ();

        return chunk;
    }
} // namespace anvil
