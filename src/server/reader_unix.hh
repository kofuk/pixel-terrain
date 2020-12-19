// SPDX-License-Identifier: MIT

#ifndef READER_UNIX_HH
#define READER_UNIX_HH

#include <cstddef>

#include "server/reader.hh"

namespace pixel_terrain::server {
    class reader_unix : public reader {
        int fd;

    public:
        reader_unix(int fd);

        long int fill_buffer(char *buf, std::size_t len, std::size_t off);
    };
} // namespace pixel_terrain::server

#endif
