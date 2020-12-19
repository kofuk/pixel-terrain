// SPDX-License-Identifier: MIT

#ifndef READER_HH
#define READER_HH

#include <cstddef>

namespace pixel_terrain::server {
    class reader {
    public:
        virtual ~reader() = default;
        virtual long int fill_buffer(char *buf, std::size_t len,
                                     std::size_t off) = 0;
    };
} // namespace pixel_terrain::server

#endif
