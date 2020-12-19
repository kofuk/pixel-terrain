// SPDX-License-Identifier: MIT

#include <algorithm>
#include <iostream>

#include "server/reader_generic.hh"

namespace pixel_terrain::server {
    reader_generic::reader_generic() {}

    long int reader_generic::fill_buffer(char *buf, size_t len, size_t off) {
        if (std::cin.bad()) return -1;

        std::cin.read(buf + off, len - off);
        return std::cin.gcount();
    }

} // namespace pixel_terrain::server
