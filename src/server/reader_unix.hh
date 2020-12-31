// SPDX-License-Identifier: MIT

#ifndef READER_UNIX_HH
#define READER_UNIX_HH

#include <cstddef>
#include <cstdint>

#include "server/reader.hh"

namespace pixel_terrain::server {
    class reader_unix : public reader {
        int fd;

    public:
        reader_unix(int fd);

        auto fill_buffer(std::uint8_t *buf, std::size_t len, std::size_t off)
            -> long int override;
    };
} // namespace pixel_terrain::server

#endif
