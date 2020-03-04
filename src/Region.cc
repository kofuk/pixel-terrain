#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#ifdef __unix__
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif /* __unix__ */

#include "NBT.hh"
#include "Region.hh"
#include "utils.hh"
#include "worker.hh"

namespace Anvil {
    Region::Region (string file_name) : journal_changed (false) {
        read_region_file (file_name);
    }

    Region::Region (string filename, string journal_dir)
        : journal_changed (false) {
        read_region_file (filename);

        filesystem::path journal_path (journal_dir);
        journal_path /=
            filesystem::path (filename).filename ().string () + ".journal";

        this->journal_file = journal_path.string ();

        ifstream journal (journal_path.string ());
        if (!journal) {
            fill (begin (last_update), end (last_update), 0);

            return;
        }

        journal.read (reinterpret_cast<char *> (last_update),
                      sizeof (uint64_t) * 1024);
    }

    Region::~Region () {
#ifdef __unix__
        if (munmap (data, len) == -1) {
            int err = errno;
            cerr << "warning: " << strerror (err) << endl;
        }
#else
        delete[] data;
#endif /* __unix__ */

        if (journal_changed && !journal_file.empty ()) {
            if (option_verbose) {
                cout << "writing journal" << endl;
            }

            ofstream out (journal_file);

            if (!out) {
                return;
            }

            out.write (reinterpret_cast<char *> (last_update),
                       sizeof (uint64_t) * 1024);
        }
    }

    void Region::read_region_file (string filename) {
#ifdef __unix__
        struct stat stat_buf;
        if (stat (filename.c_str (), &stat_buf) == -1) {
            throw runtime_error (strerror (errno));
        }

        if ((stat_buf.st_mode & S_IFMT) != S_IFREG) {
            throw logic_error ("region file is not regular file");
        }
        len = stat_buf.st_size;

        int fd = open (filename.c_str (), O_RDONLY);
        if (fd == -1) {
            throw runtime_error (strerror (errno));
        }

        data = reinterpret_cast<unsigned char *> (
            mmap (0, len, PROT_READ, MAP_PRIVATE, fd, 0));
        if (data == MAP_FAILED) {
            throw runtime_error (strerror (errno));
        }
        close (fd);
#else
        ifstream f (filename, ios::binary);

        if (!f) {
            throw invalid_argument (strerror (errno));
        }

        vector<unsigned char> dest;

        char buf[2048];
        do {
            f.read (buf, sizeof (buf));
            dest.insert (std::end (dest), buf, buf + f.gcount ());
        } while (!f.fail ());

        if (!f.eof ())
            throw logic_error ("an error occurred while reading file");

        len = dest.size ();
        data = new unsigned char[len];
        std::copy (std::begin (dest), std::end (dest), data);
#endif
    }

    size_t Region::header_offset (int chunk_x, int chunk_z) {
        return 4 * (chunk_x % 32 + chunk_z % 32 * 32);
    }

    size_t Region::chunk_location_off (int chunk_x, int chunk_z) {
        size_t b_off = header_offset (chunk_x, chunk_z);

        if (b_off + 2 >= len) return 0;

        unsigned char buf[] = {0, data[b_off], data[b_off + 1],
                               data[b_off + 2]};

        return NBT::Utils::to_host_byte_order (
            *reinterpret_cast<int32_t *> (buf));
    }

    size_t Region::chunk_location_sectors (int chunk_x, int chunk_z) {
        size_t b_off = header_offset (chunk_x, chunk_z);

        if (b_off + 3 >= len) return 0;

        return data[b_off + 3];
    }

    NBT::NBTFile *Region::chunk_data (int chunk_x, int chunk_z) {
        size_t location_off = chunk_location_off (chunk_x, chunk_z);
        size_t location_sec = chunk_location_sectors (chunk_x, chunk_z);
        if (location_off == 0 && location_sec == 0) {
            return nullptr;
        }

        location_off *= 4096;

        if (location_off + 4 >= len) return nullptr;

        int32_t length =
            NBT::Utils::to_host_byte_order (*(int32_t *)(data + location_off));

        if (location_off + 5 + length - 1 > len) return nullptr;

        int compression = data[location_off + 4];
        if (compression == 1) return nullptr;

        NBT::Utils::DecompressedData *decompressed =
            NBT::Utils::zlib_decompress (data + location_off + 5, length - 1);

        return new NBT::NBTFile (decompressed);
    }

    Chunk *Region::get_chunk (int chunk_x, int chunk_z) {
        NBT::NBTFile *nbt = chunk_data (chunk_x, chunk_z);

        if (nbt == nullptr) return nullptr;

        Chunk *chunk = new Chunk (nbt);
        chunk->parse_palette ();
        return chunk;
    }

    Chunk *Region::get_chunk_if_dirty (int chunk_x, int chunk_z) {
        NBT::NBTFile *nbt = chunk_data (chunk_x, chunk_z);
        if (nbt == nullptr) {
            return nullptr;
        }

        Chunk *chunk = new Chunk (nbt);
        if (last_update[chunk_z * 32 + chunk_x] >= chunk->last_update) {
            delete chunk;

            return nullptr;
        }

        last_update[chunk_z * 32 + chunk_x] = chunk->last_update;
        journal_changed = true;
        chunk->parse_palette ();

        return chunk;
    }
} // namespace Anvil
