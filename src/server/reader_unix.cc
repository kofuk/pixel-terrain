// SPDX-License-Identifier: MIT

#include <algorithm>
#include <unistd.h>

#include "server/reader_unix.hh"

namespace pixel_terrain::server {
    reader_unix::reader_unix(int fd) : fd(fd) {}

    auto reader_unix::fill_buffer(char *buf, size_t len, size_t off)
        -> long int {
        return read(fd, buf + off, len - off);
    }

} // namespace pixel_terrain::server
