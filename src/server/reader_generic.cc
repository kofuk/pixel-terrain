// SPDX-License-Identifier: MIT

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>

#include "server/reader_generic.hh"

namespace pixel_terrain::server {
    reader_generic::reader_generic() {}

    long int reader_generic::fill_buffer(std::uint8_t *buf, size_t len,
                                         size_t off) {
        if (std::cin.bad()) return -1;

        std::cin.read(reinterpret_cast<char *>(buf) + off, len - off);
        return std::cin.gcount();
    }

} // namespace pixel_terrain::server
