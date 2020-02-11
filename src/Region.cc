#include <algorithm>
#include <cstdint>
#include <fstream>

#include "NBT.hh"
#include "Region.hh"
#include "Utils.hh"

namespace Anvil {
    Region::Region(string file_name) {
        ifstream f(file_name);

        if (!f) {
            throw 1;
        }

        vector<unsigned char> dest;

        char buf[2048];
        do {
            f.read(buf, sizeof(buf));
            dest.insert(std::end(dest), buf, buf + f.gcount());
        } while (!f.fail());

        if (!f.eof()) throw 1;

        len = dest.size();
        data = new unsigned char[len];
        std::copy(std::begin(dest), std::end(dest), data);
    }

    Region::~Region() { delete[] data; }

    size_t Region::header_offset(int chunk_x, int chunk_z) {
        return 4 * (chunk_x % 32 + chunk_z % 32 * 32);
    }

    size_t Region::chunk_location_off(int chunk_x, int chunk_z) {
        size_t b_off = header_offset(chunk_x, chunk_z);

        if (b_off + 2 >= len) return 0;

        unsigned char buf[] = {0, data[b_off], data[b_off + 1],
                               data[b_off + 2]};

        return NBT::Utils::to_host_byte_order(*(int32_t *)buf);
    }

    size_t Region::chunk_location_sectors(int chunk_x, int chunk_z) {
        size_t b_off = header_offset(chunk_x, chunk_z);

        if (b_off + 3 >= len) return 0;

        return data[b_off + 3];
    }

    NBT::NBTFile *Region::chunk_data(int chunk_x, int chunk_z) {
        size_t location_off = chunk_location_off(chunk_x, chunk_z);
        size_t location_sec = chunk_location_sectors(chunk_x, chunk_z);
        if (location_off == 0 && location_sec == 0) {
            return nullptr;
        }

        location_off *= 4096;

        if (location_off + 4 >= len) return nullptr;

        int32_t length =
            NBT::Utils::to_host_byte_order(*(int32_t *)(data + location_off));

        if (location_off + 5 + length - 1 > len) return nullptr;

        int compression = data[location_off + 4];
        if (compression == 1) return nullptr;

        NBT::Utils::DecompressedData *decompressed =
            NBT::Utils::zlib_decompress(data + location_off + 5, length - 1);

        return new NBT::NBTFile(decompressed);
    }

    Chunk *Region::get_chunk(int chunk_x, int chunk_z) {
        NBT::NBTFile *chunk = chunk_data(chunk_x, chunk_z);

        if (chunk == nullptr) return nullptr;

        return new Chunk(chunk);
    }
} // namespace Anvil
