#ifndef READER_GENERIC_HH
#define READER_GENERIC_HH

#include <cstddef>

#include "reader.hh"

namespace pixel_terrain::server {
    class reader_generic : public reader {
        int fd;

    public:
        reader_generic();

        long int fill_buffer(char *buf, std::size_t len, std::size_t off);
    };
} // namespace pixel_terrain::server

#endif
