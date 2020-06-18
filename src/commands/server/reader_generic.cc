#include <algorithm>
#include <iostream>

#include "reader_generic.hh"

namespace pixel_terrain::commands::server {
    reader_generic::reader_generic() {}

    long int reader_generic::fill_buffer(char *buf, size_t len, size_t off) {
        if (cin.bad()) return -1;

        cin.read(buf + off, len - off);
        return cin.gcount();
    }

} // namespace pixel_terrain::commands::server
