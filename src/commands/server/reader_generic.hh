#ifndef READER_GENERIC_HH
#define READER_GENERIC_HH

#include <cstddef>

#include "reader.hh"

using namespace std;

namespace pixel_terrain::commands::server {
    class reader_generic : public reader {
        int fd;

    public:
        reader_generic();

        long int fill_buffer(char *buf, size_t len, size_t off);
    };
} // namespace pixel_terrain::commands::server

#endif
